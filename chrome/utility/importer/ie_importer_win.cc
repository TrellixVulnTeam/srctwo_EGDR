// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/utility/importer/ie_importer_win.h"

#include <objbase.h>
#include <ole2.h>
#include <intshcut.h>
#include <shlobj.h>
#include <stddef.h>
#include <urlhist.h>
#include <wininet.h>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "base/files/file_enumerator.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "base/win/registry.h"
#include "base/win/scoped_co_mem.h"
#include "base/win/scoped_comptr.h"
#include "base/win/scoped_handle.h"
#include "base/win/scoped_propvariant.h"
#include "chrome/common/importer/edge_importer_utils_win.h"
#include "chrome/common/importer/ie_importer_utils_win.h"
#include "chrome/common/importer/imported_bookmark_entry.h"
#include "chrome/common/importer/importer_bridge.h"
#include "chrome/common/importer/importer_data_types.h"
#include "chrome/common/importer/importer_url_row.h"
#include "chrome/common/importer/pstore_declarations.h"
#include "chrome/grit/generated_resources.h"
#include "chrome/utility/importer/favicon_reencode.h"
#include "components/autofill/core/common/password_form.h"
#include "ui/base/l10n/l10n_util.h"
#include "url/gurl.h"
#include "url/url_constants.h"

namespace {

// Registry key paths from which we import IE settings.
const base::char16 kSearchScopePath[] =
  L"Software\\Microsoft\\Internet Explorer\\SearchScopes";
const base::char16 kIEVersionKey[] =
  L"Software\\Microsoft\\Internet Explorer";

// NTFS stream name of favicon image data.
const base::char16 kFaviconStreamName[] = L":favicon:$DATA";

// A struct that hosts the information of AutoComplete data in PStore.
struct AutoCompleteInfo {
  base::string16 key;
  std::vector<base::string16> data;
  bool is_url;
};

// Gets the creation time of the given file or directory.
base::Time GetFileCreationTime(const base::string16& file) {
  base::Time creation_time;
  base::win::ScopedHandle file_handle(
      CreateFile(file.c_str(), GENERIC_READ,
                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                 NULL, OPEN_EXISTING,
                 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL));
  FILETIME creation_filetime;
  if (!file_handle.IsValid())
    return creation_time;
  if (GetFileTime(file_handle.Get(), &creation_filetime, NULL, NULL))
    creation_time = base::Time::FromFileTime(creation_filetime);
  return creation_time;
}

// Safely read an object of type T from a raw sequence of bytes.
template <typename T>
bool BinaryRead(T* data, size_t offset, const std::vector<uint8_t>& blob) {
  if (offset + sizeof(T) > blob.size())
    return false;
  memcpy(data, &blob[offset], sizeof(T));
  return true;
}

// Safely read an ITEMIDLIST from a raw sequence of bytes.
//
// An ITEMIDLIST is a list of SHITEMIDs, terminated by a SHITEMID with
// .cb = 0. Here, before simply casting &blob[offset] to LPITEMIDLIST,
// we verify that the list structure is not overrunning the boundary of
// the binary blob.
LPCITEMIDLIST BinaryReadItemIDList(size_t offset,
                                   size_t idlist_size,
                                   const std::vector<uint8_t>& blob) {
  size_t head = 0;
  while (true) {
    // Use a USHORT instead of SHITEMID to avoid buffer over read.
    USHORT id_cb;
    if (head >= idlist_size || !BinaryRead(&id_cb, offset + head, blob))
      return NULL;
    if (id_cb == 0)
      break;
    head += id_cb;
  }
  return reinterpret_cast<LPCITEMIDLIST>(&blob[offset]);
}

// Compares the two bookmarks in the order of IE's Favorites menu.
// Returns true if rhs should come later than lhs (lhs < rhs).
struct IEOrderBookmarkComparator {
  bool operator()(const ImportedBookmarkEntry& lhs,
                  const ImportedBookmarkEntry& rhs) const {
    static const uint32_t kNotSorted = 0xfffffffb;  // IE uses this magic value.
    base::FilePath lhs_prefix;
    base::FilePath rhs_prefix;
    for (size_t i = 0; i <= lhs.path.size() && i <= rhs.path.size(); ++i) {
      const base::FilePath::StringType lhs_i =
        (i < lhs.path.size() ? lhs.path[i] : lhs.title + L".url");
      const base::FilePath::StringType rhs_i =
        (i < rhs.path.size() ? rhs.path[i] : rhs.title + L".url");
      lhs_prefix = lhs_prefix.Append(lhs_i);
      rhs_prefix = rhs_prefix.Append(rhs_i);
      if (lhs_i == rhs_i)
        continue;
      // The first path element that differs between the two.
      std::map<base::FilePath, uint32_t>::const_iterator lhs_iter =
          sort_index_->find(lhs_prefix);
      std::map<base::FilePath, uint32_t>::const_iterator rhs_iter =
          sort_index_->find(rhs_prefix);
      uint32_t lhs_sort_index =
          (lhs_iter == sort_index_->end() ? kNotSorted : lhs_iter->second);
      uint32_t rhs_sort_index =
          (rhs_iter == sort_index_->end() ? kNotSorted : rhs_iter->second);
      if (lhs_sort_index != rhs_sort_index)
        return lhs_sort_index < rhs_sort_index;
      // If they have the same sort order, sort alphabetically.
      return lhs_i < rhs_i;
    }
    return lhs.path.size() < rhs.path.size();
  }
  const std::map<base::FilePath, uint32_t>* sort_index_;
};

// IE stores the order of the Favorites menu in registry under:
// HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\MenuOrder\Favorites.
// The folder hierarchy of Favorites menu is directly mapped to the key
// hierarchy in the registry.
//
// If the order of the items in a folder is customized by user, the order is
// recorded in the REG_BINARY value named "Order" of the corresponding key.
// The content of the "Order" value is a raw binary dump of an array of the
// following data structure
//   struct {
//     uint32_t size;  // Note that ITEMIDLIST is variably-sized.
//     uint32_t sort_index;  // 0 means this is the first item, 1 the second,
//     ...
//     ITEMIDLIST item_id;
//   };
// where each item_id should correspond to a favorites link file (*.url) in
// the current folder.
// gcc, in its infinite wisdom, only allows WARN_UNUSED_RESULT for prototypes.
// Clang, to be compatible with gcc, warns if WARN_UNUSED_RESULT is used in a
// non-gcc compatible manner (-Wgcc-compat). So even though gcc isn't used to
// build on Windows, declare some prototypes anyway to satisfy Clang's gcc
// compatibility warnings.
bool ParseFavoritesOrderBlob(const Importer* importer,
                             const std::vector<uint8_t>& blob,
                             const base::FilePath& path,
                             std::map<base::FilePath, uint32_t>* sort_index)
    WARN_UNUSED_RESULT;
bool ParseFavoritesOrderBlob(const Importer* importer,
                             const std::vector<uint8_t>& blob,
                             const base::FilePath& path,
                             std::map<base::FilePath, uint32_t>* sort_index) {
  static const int kItemCountOffset = 16;
  static const int kItemListStartOffset = 20;

  // Read the number of items.
  uint32_t item_count = 0;
  if (!BinaryRead(&item_count, kItemCountOffset, blob))
    return false;

  // Traverse over the items.
  size_t base_offset = kItemListStartOffset;
  for (uint32_t i = 0; i < item_count && !importer->cancelled(); ++i) {
    static const int kSizeOffset = 0;
    static const int kSortIndexOffset = 4;
    static const int kItemIDListOffset = 8;

    // Read the size (number of bytes) of the current item.
    uint32_t item_size = 0;
    if (!BinaryRead(&item_size, base_offset + kSizeOffset, blob) ||
        base_offset + item_size <= base_offset ||  // checking overflow
        base_offset + item_size > blob.size())
      return false;

    // Read the sort index of the current item.
    uint32_t item_sort_index = 0;
    if (!BinaryRead(&item_sort_index, base_offset + kSortIndexOffset, blob))
      return false;

    // Read the file name from the ITEMIDLIST structure.
    LPCITEMIDLIST idlist = BinaryReadItemIDList(
      base_offset + kItemIDListOffset, item_size - kItemIDListOffset, blob);
    TCHAR item_filename[MAX_PATH];
    if (!idlist || !SHGetPathFromIDList(idlist, item_filename))
      return false;
    base::FilePath item_relative_path =
      path.Append(base::FilePath(item_filename).BaseName());

    // Record the retrieved information and go to the next item.
    sort_index->insert(std::make_pair(item_relative_path, item_sort_index));
    base_offset += item_size;
  }
  return true;
}

bool ParseFavoritesOrderRegistryTree(
    const Importer* importer,
    const base::win::RegKey& key,
    const base::FilePath& path,
    std::map<base::FilePath, uint32_t>* sort_index) WARN_UNUSED_RESULT;
bool ParseFavoritesOrderRegistryTree(
    const Importer* importer,
    const base::win::RegKey& key,
    const base::FilePath& path,
    std::map<base::FilePath, uint32_t>* sort_index) {
  // Parse the order information of the current folder.
  DWORD blob_length = 0;
  if (key.ReadValue(L"Order", NULL, &blob_length, NULL) == ERROR_SUCCESS) {
    std::vector<uint8_t> blob(blob_length);
    if (blob_length > 0 &&
        key.ReadValue(L"Order", reinterpret_cast<DWORD*>(&blob[0]),
                      &blob_length, NULL) == ERROR_SUCCESS) {
      if (!ParseFavoritesOrderBlob(importer, blob, path, sort_index))
        return false;
    }
  }

  // Recursively parse subfolders.
  for (base::win::RegistryKeyIterator child(key.Handle(), L"");
       child.Valid() && !importer->cancelled();
       ++child) {
    base::win::RegKey subkey(key.Handle(), child.Name(), KEY_READ);
    if (subkey.Valid()) {
      base::FilePath subpath(path.Append(child.Name()));
      if (!ParseFavoritesOrderRegistryTree(importer, subkey, subpath,
                                           sort_index)) {
        return false;
      }
    }
  }
  return true;
}

bool ParseFavoritesOrderInfo(const Importer* importer,
                             std::map<base::FilePath, uint32_t>* sort_index)
    WARN_UNUSED_RESULT;
bool ParseFavoritesOrderInfo(const Importer* importer,
                             std::map<base::FilePath, uint32_t>* sort_index) {
  base::string16 key_path(importer::GetIEFavoritesOrderKey());
  base::win::RegKey key(HKEY_CURRENT_USER, key_path.c_str(), KEY_READ);
  if (!key.Valid())
    return false;
  return ParseFavoritesOrderRegistryTree(importer, key, base::FilePath(),
                                         sort_index);
}

// Reads the sort order from registry. If failed, we don't touch the list
// and use the default (alphabetical) order.
void SortBookmarksInIEOrder(
    const Importer* importer,
    std::vector<ImportedBookmarkEntry>* bookmarks) {
  std::map<base::FilePath, uint32_t> sort_index;
  if (!ParseFavoritesOrderInfo(importer, &sort_index))
    return;
  IEOrderBookmarkComparator compare = {&sort_index};
  std::sort(bookmarks->begin(), bookmarks->end(), compare);
}

// Reads an internet shortcut (*.url) |file| and returns a COM object
// representing it.
bool LoadInternetShortcut(
    const base::string16& file,
    base::win::ScopedComPtr<IUniformResourceLocator>* shortcut) {
  base::win::ScopedComPtr<IUniformResourceLocator> url_locator;
  if (FAILED(::CoCreateInstance(CLSID_InternetShortcut, NULL,
                                CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&url_locator))))
    return false;

  base::win::ScopedComPtr<IPersistFile> persist_file;
  if (FAILED(url_locator.CopyTo(persist_file.GetAddressOf())))
    return false;

  // Loads the Internet Shortcut from persistent storage.
  if (FAILED(persist_file->Load(file.c_str(), STGM_READ)))
    return false;

  std::swap(url_locator, *shortcut);
  return true;
}

// Reads the URL stored in the internet shortcut.
GURL ReadURLFromInternetShortcut(IUniformResourceLocator* url_locator) {
  base::win::ScopedCoMem<wchar_t> url;
  // GetURL can return S_FALSE (FAILED(S_FALSE) is false) when url == NULL.
  return (FAILED(url_locator->GetURL(&url)) || !url) ?
      GURL() : GURL(url.get());
}

// Reads the URL of the favicon of the internet shortcut.
GURL ReadFaviconURLFromInternetShortcut(IUniformResourceLocator* url_locator) {
  base::win::ScopedComPtr<IPropertySetStorage> property_set_storage;
  if (FAILED(url_locator->QueryInterface(IID_PPV_ARGS(&property_set_storage))))
    return GURL();

  base::win::ScopedComPtr<IPropertyStorage> property_storage;
  if (FAILED(property_set_storage->Open(FMTID_Intshcut, STGM_READ,
                                        property_storage.GetAddressOf()))) {
    return GURL();
  }

  PROPSPEC properties[] = {{PRSPEC_PROPID, {PID_IS_ICONFILE}}};
  // ReadMultiple takes a non-const array of PROPVARIANTs, but since this code
  // only needs an array of size 1: a non-const pointer to |output| is
  // equivalent.
  base::win::ScopedPropVariant output;
  // ReadMultiple can return S_FALSE (FAILED(S_FALSE) is false) when the
  // property is not found, in which case output[0].vt is set to VT_EMPTY.
  if (FAILED(property_storage->ReadMultiple(1, properties, output.Receive())) ||
      output.get().vt != VT_LPWSTR)
    return GURL();
  return GURL(output.get().pwszVal);
}

// Reads the favicon imaga data in an NTFS alternate data stream. This is where
// IE7 and above store the data.
bool ReadFaviconDataFromInternetShortcut(const base::string16& file,
                                         std::string* data) {
  return base::ReadFileToString(
      base::FilePath(file + kFaviconStreamName), data);
}

// Reads the favicon imaga data in the Internet cache. IE6 doesn't hold the data
// explicitly, but it might be found in the cache.
bool ReadFaviconDataFromCache(const GURL& favicon_url, std::string* data) {
  std::wstring url_wstring(base::UTF8ToWide(favicon_url.spec()));
  DWORD info_size = 0;
  GetUrlCacheEntryInfoEx(url_wstring.c_str(), NULL, &info_size, NULL, NULL,
                         NULL, 0);
  if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    return false;

  std::vector<char> buf(info_size);
  INTERNET_CACHE_ENTRY_INFO* cache =
      reinterpret_cast<INTERNET_CACHE_ENTRY_INFO*>(&buf[0]);
  if (!GetUrlCacheEntryInfoEx(url_wstring.c_str(), cache, &info_size, NULL,
                              NULL, NULL, 0)) {
    return false;
  }
  return base::ReadFileToString(base::FilePath(cache->lpszLocalFileName), data);
}

// Reads the binary image data of favicon of an internet shortcut file |file|.
// |favicon_url| read by ReadFaviconURLFromInternetShortcut is also needed to
// examine the IE cache.
bool ReadReencodedFaviconData(const base::string16& file,
                              const GURL& favicon_url,
                              std::vector<unsigned char>* data) {
  std::string image_data;
  if (!ReadFaviconDataFromInternetShortcut(file, &image_data) &&
      !ReadFaviconDataFromCache(favicon_url, &image_data)) {
    return false;
  }

  const unsigned char* ptr =
      reinterpret_cast<const unsigned char*>(image_data.c_str());
  return importer::ReencodeFavicon(ptr, image_data.size(), data);
}

// Loads favicon image data and registers to |favicon_map|.
void UpdateFaviconMap(
    const base::string16& url_file,
    const GURL& url,
    IUniformResourceLocator* url_locator,
    std::map<GURL, favicon_base::FaviconUsageData>* favicon_map) {
  GURL favicon_url = ReadFaviconURLFromInternetShortcut(url_locator);
  if (!favicon_url.is_valid())
    return;

  std::map<GURL, favicon_base::FaviconUsageData>::iterator it =
      favicon_map->find(favicon_url);
  if (it != favicon_map->end()) {
    // Known favicon URL.
    it->second.urls.insert(url);
  } else {
    // New favicon URL. Read the image data and store.
    favicon_base::FaviconUsageData usage;
    if (ReadReencodedFaviconData(url_file, favicon_url, &usage.png_data)) {
      usage.favicon_url = favicon_url;
      usage.urls.insert(url);
      favicon_map->insert(std::make_pair(favicon_url, usage));
    }
  }
}

}  // namespace

// static
// {E161255A-37C3-11D2-BCAA-00C04fD929DB}
const GUID IEImporter::kPStoreAutocompleteGUID = {
    0xe161255a, 0x37c3, 0x11d2,
    { 0xbc, 0xaa, 0x00, 0xc0, 0x4f, 0xd9, 0x29, 0xdb }
};
// {A79029D6-753E-4e27-B807-3D46AB1545DF}
const GUID IEImporter::kUnittestGUID = {
    0xa79029d6, 0x753e, 0x4e27,
    { 0xb8, 0x7, 0x3d, 0x46, 0xab, 0x15, 0x45, 0xdf }
};

IEImporter::IEImporter() : edge_import_mode_(false) {}

void IEImporter::StartImport(const importer::SourceProfile& source_profile,
                             uint16_t items,
                             ImporterBridge* bridge) {
  edge_import_mode_ = source_profile.importer_type == importer::TYPE_EDGE;
  bridge_ = bridge;

  if (edge_import_mode_) {
    // When using for Edge imports we only support Favorites.
    DCHECK_EQ(items, importer::FAVORITES);
    // As coming from untrusted source ensure items is correct.
    items = importer::FAVORITES;
  }
  source_path_ = source_profile.source_path;

  bridge_->NotifyStarted();

  if ((items & importer::HOME_PAGE) && !cancelled()) {
    bridge_->NotifyItemStarted(importer::HOME_PAGE);
    ImportHomepage();  // Doesn't have a UI item.
    bridge_->NotifyItemEnded(importer::HOME_PAGE);
  }
  // The order here is important!
  if ((items & importer::HISTORY) && !cancelled()) {
    bridge_->NotifyItemStarted(importer::HISTORY);
    ImportHistory();
    bridge_->NotifyItemEnded(importer::HISTORY);
  }
  if ((items & importer::FAVORITES) && !cancelled()) {
    bridge_->NotifyItemStarted(importer::FAVORITES);
    ImportFavorites();
    bridge_->NotifyItemEnded(importer::FAVORITES);
  }
  if ((items & importer::SEARCH_ENGINES) && !cancelled()) {
    bridge_->NotifyItemStarted(importer::SEARCH_ENGINES);
    ImportSearchEngines();
    bridge_->NotifyItemEnded(importer::SEARCH_ENGINES);
  }
  if ((items & importer::PASSWORDS) && !cancelled()) {
    bridge_->NotifyItemStarted(importer::PASSWORDS);
    // Always import IE6 passwords.
    ImportPasswordsIE6();

    if (CurrentIEVersion() >= 7)
      ImportPasswordsIE7();
    bridge_->NotifyItemEnded(importer::PASSWORDS);
  }
  bridge_->NotifyEnded();
}

IEImporter::~IEImporter() {
}

void IEImporter::ImportFavorites() {
  FavoritesInfo info;
  if (!GetFavoritesInfo(&info))
    return;

  BookmarkVector bookmarks;
  favicon_base::FaviconUsageDataList favicons;
  ParseFavoritesFolder(info, &bookmarks, &favicons);

  if (!bookmarks.empty() && !cancelled()) {
    const base::string16& first_folder_name =
        edge_import_mode_
            ? l10n_util::GetStringUTF16(IDS_BOOKMARK_GROUP_FROM_EDGE)
            : l10n_util::GetStringUTF16(IDS_BOOKMARK_GROUP_FROM_IE);

    bridge_->AddBookmarks(bookmarks, first_folder_name);
  }
  if (!favicons.empty() && !cancelled())
    bridge_->SetFavicons(favicons);
}

void IEImporter::ImportHistory() {
  const std::string kSchemes[] = {url::kHttpScheme,
                                  url::kHttpsScheme,
                                  url::kFtpScheme,
                                  url::kFileScheme};
  int total_schemes = arraysize(kSchemes);

  base::win::ScopedComPtr<IUrlHistoryStg2> url_history_stg2;
  if (FAILED(::CoCreateInstance(CLSID_CUrlHistory, NULL, CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&url_history_stg2)))) {
    return;
  }
  base::win::ScopedComPtr<IEnumSTATURL> enum_url;
  if (SUCCEEDED(url_history_stg2->EnumUrls(enum_url.GetAddressOf()))) {
    std::vector<ImporterURLRow> rows;
    STATURL stat_url;

    // IEnumSTATURL::Next() doesn't fill STATURL::dwFlags by default. Need to
    // call IEnumSTATURL::SetFilter() with STATURL_QUERYFLAG_TOPLEVEL flag to
    // specify how STATURL structure will be filled.
    // The first argument of IEnumSTATURL::SetFilter() specifies the URL prefix
    // that is used by IEnumSTATURL::Next() for filtering items by URL.
    // So need to pass an empty string here to get all history items.
    enum_url->SetFilter(L"", STATURL_QUERYFLAG_TOPLEVEL);
    while (!cancelled() &&
           enum_url->Next(1, &stat_url, NULL) == S_OK) {
      base::string16 url_string;
      if (stat_url.pwcsUrl) {
        url_string = stat_url.pwcsUrl;
        CoTaskMemFree(stat_url.pwcsUrl);
      }
      base::string16 title_string;
      if (stat_url.pwcsTitle) {
        title_string = stat_url.pwcsTitle;
        CoTaskMemFree(stat_url.pwcsTitle);
      }

      GURL url(url_string);
      // Skips the URLs that are invalid or have other schemes.
      if (!url.is_valid() ||
          (std::find(kSchemes, kSchemes + total_schemes, url.scheme()) ==
           kSchemes + total_schemes))
        continue;

      ImporterURLRow row(url);
      row.title = title_string;
      row.last_visit = base::Time::FromFileTime(stat_url.ftLastVisited);
      if (stat_url.dwFlags == STATURLFLAG_ISTOPLEVEL) {
        row.visit_count = 1;
        row.hidden = false;
      } else {
        // dwFlags should only contain the STATURLFLAG_ISTOPLEVEL bit per
        // the filter set above.
        DCHECK(!stat_url.dwFlags);
        row.hidden = true;
      }

      rows.push_back(row);
    }

    if (!cancelled()) {
      bridge_->SetHistoryItems(rows, importer::VISIT_SOURCE_IE_IMPORTED);
    }
  }
}

void IEImporter::ImportPasswordsIE6() {
  GUID AutocompleteGUID = kPStoreAutocompleteGUID;
  if (!source_path_.empty()) {
    // We supply a fake GUID for testting.
    AutocompleteGUID = kUnittestGUID;
  }

  // The PStoreCreateInstance function retrieves an interface pointer
  // to a storage provider. But this function has no associated import
  // library or header file, we must call it using the LoadLibrary()
  // and GetProcAddress() functions.
  typedef HRESULT (WINAPI *PStoreCreateFunc)(IPStore**, DWORD, DWORD, DWORD);
  HMODULE pstorec_dll = LoadLibrary(L"pstorec.dll");
  if (!pstorec_dll)
    return;
  PStoreCreateFunc PStoreCreateInstance =
      (PStoreCreateFunc)GetProcAddress(pstorec_dll, "PStoreCreateInstance");
  if (!PStoreCreateInstance) {
    FreeLibrary(pstorec_dll);
    return;
  }

  base::win::ScopedComPtr<IPStore> pstore;
  HRESULT result = PStoreCreateInstance(pstore.GetAddressOf(), 0, 0, 0);
  if (result != S_OK) {
    FreeLibrary(pstorec_dll);
    return;
  }

  std::vector<AutoCompleteInfo> ac_list;

  // Enumerates AutoComplete items in the protected database.
  base::win::ScopedComPtr<IEnumPStoreItems> item;
  result = pstore->EnumItems(0, &AutocompleteGUID, &AutocompleteGUID, 0,
                             item.GetAddressOf());
  if (result != PST_E_OK) {
    pstore.Reset();
    FreeLibrary(pstorec_dll);
    return;
  }

  wchar_t* item_name;
  while (!cancelled() && SUCCEEDED(item->Next(1, &item_name, 0))) {
    DWORD length = 0;
    unsigned char* buffer = NULL;
    result = pstore->ReadItem(0, &AutocompleteGUID, &AutocompleteGUID,
                              item_name, &length, &buffer, NULL, 0);
    if (SUCCEEDED(result)) {
      AutoCompleteInfo ac;
      ac.key = item_name;
      base::string16 data;
      data.insert(0, reinterpret_cast<wchar_t*>(buffer),
                  length / sizeof(wchar_t));

      // The key name is always ended with ":StringData".
      const wchar_t kDataSuffix[] = L":StringData";
      size_t i = ac.key.rfind(kDataSuffix);
      if (i != base::string16::npos && ac.key.substr(i) == kDataSuffix) {
        ac.key.erase(i);
        ac.is_url = (ac.key.find(L"://") != base::string16::npos);
        ac_list.push_back(ac);
        ac_list.back().data =
            base::SplitString(data, base::string16(1, '\0'),
                              base::TRIM_WHITESPACE, base::SPLIT_WANT_ALL);
      }
      CoTaskMemFree(buffer);
    }
    CoTaskMemFree(item_name);
  }
  // Releases them before unload the dll.
  item.Reset();
  pstore.Reset();
  FreeLibrary(pstorec_dll);

  size_t i;
  for (i = 0; i < ac_list.size(); i++) {
    if (!ac_list[i].is_url || ac_list[i].data.size() < 2)
      continue;

    GURL url(ac_list[i].key.c_str());
    if (!(base::LowerCaseEqualsASCII(url.scheme(), url::kHttpScheme) ||
          base::LowerCaseEqualsASCII(url.scheme(), url::kHttpsScheme))) {
      continue;
    }

    autofill::PasswordForm form;
    GURL::Replacements rp;
    rp.ClearUsername();
    rp.ClearPassword();
    rp.ClearQuery();
    rp.ClearRef();
    form.origin = url.ReplaceComponents(rp);
    form.username_value = ac_list[i].data[0];
    form.password_value = ac_list[i].data[1];
    form.signon_realm = url.GetOrigin().spec();

    // Goes through the list to find out the username field
    // of the web page.
    size_t list_it, item_it;
    for (list_it = 0; list_it < ac_list.size(); ++list_it) {
      if (ac_list[list_it].is_url)
        continue;

      for (item_it = 0; item_it < ac_list[list_it].data.size(); ++item_it)
        if (ac_list[list_it].data[item_it] == form.username_value) {
          form.username_element = ac_list[list_it].key;
          break;
        }
    }

    bridge_->SetPasswordForm(form);
  }
}

void IEImporter::ImportPasswordsIE7() {
  base::string16 key_path(importer::GetIE7PasswordsKey());
  base::win::RegKey key(HKEY_CURRENT_USER, key_path.c_str(), KEY_READ);
  base::win::RegistryValueIterator reg_iterator(HKEY_CURRENT_USER,
                                                key_path.c_str());
  importer::ImporterIE7PasswordInfo password_info;
  while (reg_iterator.Valid() && !cancelled()) {
    // Get the size of the encrypted data.
    DWORD value_len = 0;
    key.ReadValue(reg_iterator.Name(), NULL, &value_len, NULL);
    if (value_len) {
      // Query the encrypted data.
      password_info.encrypted_data.resize(value_len);
      if (key.ReadValue(reg_iterator.Name(),
                        &password_info.encrypted_data.front(),
                        &value_len, NULL) == ERROR_SUCCESS) {
        password_info.url_hash = reg_iterator.Name();
        password_info.date_created = base::Time::Now();

        bridge_->AddIE7PasswordInfo(password_info);
      }
    }

    ++reg_iterator;
  }
}

void IEImporter::ImportSearchEngines() {
  // On IE, search engines are stored in the registry, under:
  // Software\Microsoft\Internet Explorer\SearchScopes
  // Each key represents a search engine. The URL value contains the URL and
  // the DisplayName the name.
  typedef std::map<std::string, base::string16> SearchEnginesMap;
  SearchEnginesMap search_engines_map;
  for (base::win::RegistryKeyIterator key_iter(HKEY_CURRENT_USER,
       kSearchScopePath); key_iter.Valid(); ++key_iter) {
    base::string16 sub_key_name = kSearchScopePath;
    sub_key_name.append(L"\\").append(key_iter.Name());
    base::win::RegKey sub_key(HKEY_CURRENT_USER, sub_key_name.c_str(),
                              KEY_READ);
    base::string16 wide_url;
    if ((sub_key.ReadValue(L"URL", &wide_url) != ERROR_SUCCESS) ||
        wide_url.empty()) {
      VLOG(1) << "No URL for IE search engine at " << key_iter.Name();
      continue;
    }
    // For the name, we try the default value first (as Live Search uses a
    // non displayable name in DisplayName, and the readable name under the
    // default value).
    base::string16 name;
    if ((sub_key.ReadValue(NULL, &name) != ERROR_SUCCESS) || name.empty()) {
      // Try the displayable name.
      if ((sub_key.ReadValue(L"DisplayName", &name) != ERROR_SUCCESS) ||
          name.empty()) {
        VLOG(1) << "No name for IE search engine at " << key_iter.Name();
        continue;
      }
    }

    std::string url(base::WideToUTF8(wide_url));
    SearchEnginesMap::iterator t_iter = search_engines_map.find(url);
    if (t_iter == search_engines_map.end()) {
      // First time we see that URL.
      GURL gurl(url);
      if (gurl.is_valid()) {
        t_iter = search_engines_map.insert(std::make_pair(url, name)).first;
      }
    }
  }
  // ProfileWriter::AddKeywords() requires a vector and we have a map.
  std::vector<importer::SearchEngineInfo> search_engines;
  for (SearchEnginesMap::iterator i = search_engines_map.begin();
       i != search_engines_map.end(); ++i) {
    importer::SearchEngineInfo search_engine_info;
    search_engine_info.url = base::UTF8ToUTF16(i->first);
    search_engine_info.display_name = i->second;
    search_engines.push_back(search_engine_info);
  }
  bridge_->SetKeywords(search_engines, true);
}

void IEImporter::ImportHomepage() {
  static constexpr wchar_t kIEHomepage[] = L"Start Page";
  static constexpr wchar_t kIEDefaultHomepage[] = L"Default_Page_URL";

  base::string16 key_path(importer::GetIESettingsKey());

  base::win::RegKey key(HKEY_CURRENT_USER, key_path.c_str(), KEY_READ);
  base::string16 homepage_url;
  if (key.ReadValue(kIEHomepage, &homepage_url) != ERROR_SUCCESS ||
      homepage_url.empty())
    return;

  GURL homepage = GURL(homepage_url);
  if (!homepage.is_valid())
    return;

  // Check to see if this is the default website and skip import.
  base::win::RegKey keyDefault(HKEY_LOCAL_MACHINE, key_path.c_str(), KEY_READ);
  base::string16 default_homepage_url;
  LONG result = keyDefault.ReadValue(kIEDefaultHomepage, &default_homepage_url);
  if (result == ERROR_SUCCESS && !default_homepage_url.empty()) {
    if (homepage.spec() == GURL(default_homepage_url).spec())
      return;
  }
  bridge_->AddHomePage(homepage);
}

bool IEImporter::GetFavoritesInfo(IEImporter::FavoritesInfo* info) {
  if (!source_path_.empty()) {
    // Source path exists during testing as well as when importing from Edge.
    info->path = source_path_;
    info->path = info->path.AppendASCII("Favorites");
    info->links_folder = L"Links";
    return true;
  }

  // IE stores the favorites in the Favorites under user profile's folder.
  wchar_t buffer[MAX_PATH];
  if (FAILED(SHGetFolderPath(NULL, CSIDL_FAVORITES, NULL, SHGFP_TYPE_CURRENT,
                             buffer))) {
    return false;
  }

  // There is a Links folder under Favorites folder since Windows Vista, but it
  // is not recording in Vista's registry. So we assume the Links folder is
  // under Favorites folder since it looks like there is not name different in
  // every language version of Windows.
  info->path = base::FilePath(buffer);
  info->links_folder = L"Links";

  return true;
}

void IEImporter::ParseFavoritesFolder(
    const FavoritesInfo& info,
    BookmarkVector* bookmarks,
    favicon_base::FaviconUsageDataList* favicons) {
  base::FilePath file;
  std::vector<base::FilePath::StringType> file_list;
  base::FilePath favorites_path(info.path);
  // Favorites path length.  Make sure it doesn't include the trailing \.
  size_t favorites_path_len =
      favorites_path.StripTrailingSeparators().value().size();
  base::FileEnumerator file_enumerator(
      favorites_path, true, base::FileEnumerator::FILES);
  while (!(file = file_enumerator.Next()).value().empty() && !cancelled())
    file_list.push_back(file.value());

  // Keep the bookmarks in alphabetical order.
  std::sort(file_list.begin(), file_list.end());

  // Map from favicon URLs to the favicon data (the binary image data and the
  // set of bookmark URLs referring to the favicon).
  typedef std::map<GURL, favicon_base::FaviconUsageData> FaviconMap;
  FaviconMap favicon_map;

  for (std::vector<base::FilePath::StringType>::iterator it = file_list.begin();
       it != file_list.end(); ++it) {
    base::FilePath shortcut(*it);
    if (!base::LowerCaseEqualsASCII(shortcut.Extension(), ".url"))
      continue;

    // Skip the bookmark with invalid URL.
    base::win::ScopedComPtr<IUniformResourceLocator> url_locator;
    if (!LoadInternetShortcut(*it, &url_locator))
      continue;
    GURL url = ReadURLFromInternetShortcut(url_locator.Get());
    if (!url.is_valid())
      continue;
    // Skip default bookmarks. go.microsoft.com redirects to
    // search.microsoft.com, and http://go.microsoft.com/fwlink/?LinkId=XXX,
    // which URLs IE has as default, to some another sites.
    // We expect that users will never themselves create bookmarks having this
    // hostname.
    if (url.host() == "go.microsoft.com")
      continue;
    // Read favicon.
    UpdateFaviconMap(*it, url, url_locator.Get(), &favicon_map);

    // Make the relative path from the Favorites folder, without the basename.
    // ex. Suppose that the Favorites folder is C:\Users\Foo\Favorites.
    //   C:\Users\Foo\Favorites\Foo.url -> ""
    //   C:\Users\Foo\Favorites\Links\Bar\Baz.url -> "Links\Bar"
    base::FilePath::StringType relative_string =
        shortcut.DirName().value().substr(favorites_path_len);
    if (!relative_string.empty() &&
        base::FilePath::IsSeparator(relative_string[0]))
      relative_string = relative_string.substr(1);
    base::FilePath relative_path(relative_string);

    ImportedBookmarkEntry entry;
    // Remove the dot, the file extension, and the directory path.
    entry.title = shortcut.RemoveExtension().BaseName().value();
    entry.url = url;
    entry.creation_time = GetFileCreationTime(*it);
    if (!relative_path.empty())
      relative_path.GetComponents(&entry.path);

    // Add the bookmark.
    if (!entry.path.empty() && entry.path[0] == info.links_folder) {
      // Bookmarks in the Link folder should be imported to the toolbar.
      entry.in_toolbar = true;
    }
    bookmarks->push_back(entry);
  }

  if (!edge_import_mode_) {
    // Reflect the menu order in IE.
    SortBookmarksInIEOrder(this, bookmarks);
  }

  // Record favicon data.
  for (FaviconMap::iterator iter = favicon_map.begin();
       iter != favicon_map.end(); ++iter)
    favicons->push_back(iter->second);
}

int IEImporter::CurrentIEVersion() const {
  static int version = -1;
  if (version < 0) {
    wchar_t buffer[128];
    DWORD buffer_length = sizeof(buffer);
    base::win::RegKey reg_key(HKEY_LOCAL_MACHINE, kIEVersionKey, KEY_READ);
    LONG result = reg_key.ReadValue(L"Version", buffer, &buffer_length, NULL);
    version = ((result == ERROR_SUCCESS)? _wtoi(buffer) : 0);
  }
  return version;
}

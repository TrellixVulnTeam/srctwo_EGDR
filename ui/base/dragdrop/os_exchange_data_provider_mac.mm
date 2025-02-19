// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/dragdrop/os_exchange_data_provider_mac.h"

#import <Cocoa/Cocoa.h>

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/pickle.h"
#include "base/strings/sys_string_conversions.h"
#include "base/strings/utf_string_conversions.h"
#import "third_party/mozilla/NSPasteboard+Utils.h"
#import "ui/base/clipboard/clipboard_util_mac.h"
#include "ui/base/clipboard/custom_data_helper.h"
#import "ui/base/dragdrop/cocoa_dnd_util.h"
#include "url/gurl.h"

namespace ui {

OSExchangeDataProviderMac::OSExchangeDataProviderMac()
    : pasteboard_(new ui::UniquePasteboard) {}

OSExchangeDataProviderMac::OSExchangeDataProviderMac(
    scoped_refptr<ui::UniquePasteboard> pb)
    : pasteboard_(pb) {}

OSExchangeDataProviderMac::~OSExchangeDataProviderMac() {
}

std::unique_ptr<OSExchangeData::Provider>
OSExchangeDataProviderMac::Clone() const {
  return std::unique_ptr<OSExchangeData::Provider>(
      new OSExchangeDataProviderMac(pasteboard_));
}

void OSExchangeDataProviderMac::MarkOriginatedFromRenderer() {
  NOTIMPLEMENTED();
}

bool OSExchangeDataProviderMac::DidOriginateFromRenderer() const {
  NOTIMPLEMENTED();
  return false;
}

void OSExchangeDataProviderMac::SetString(const base::string16& string) {
  [pasteboard_->get() setString:base::SysUTF16ToNSString(string)
                        forType:NSPasteboardTypeString];
}

void OSExchangeDataProviderMac::SetURL(const GURL& url,
                                       const base::string16& title) {
  base::scoped_nsobject<NSPasteboardItem> item =
      ClipboardUtil::PasteboardItemFromUrl(base::SysUTF8ToNSString(url.spec()),
                                           base::SysUTF16ToNSString(title));
  ui::ClipboardUtil::AddDataToPasteboard(pasteboard_->get(), item);
}

void OSExchangeDataProviderMac::SetFilename(const base::FilePath& path) {
  [pasteboard_->get() setPropertyList:@[ base::SysUTF8ToNSString(path.value()) ]
                              forType:NSFilenamesPboardType];
}

void OSExchangeDataProviderMac::SetFilenames(
    const std::vector<FileInfo>& filenames) {
  NOTIMPLEMENTED();
}

void OSExchangeDataProviderMac::SetPickledData(
    const Clipboard::FormatType& format,
    const base::Pickle& data) {
  NSData* ns_data = [NSData dataWithBytes:data.data() length:data.size()];
  [pasteboard_->get() setData:ns_data forType:format.ToNSString()];
}

bool OSExchangeDataProviderMac::GetString(base::string16* data) const {
  DCHECK(data);
  NSString* item = [pasteboard_->get() stringForType:NSPasteboardTypeString];
  if (item) {
    *data = base::SysNSStringToUTF16(item);
    return true;
  }

  // There was no NSString, check for an NSURL.
  GURL url;
  base::string16 title;
  bool result =
      GetURLAndTitle(OSExchangeData::DO_NOT_CONVERT_FILENAMES, &url, &title);
  if (result)
    *data = base::UTF8ToUTF16(url.spec());

  return result;
}

bool OSExchangeDataProviderMac::GetURLAndTitle(
    OSExchangeData::FilenameToURLPolicy policy,
    GURL* url,
    base::string16* title) const {
  DCHECK(url);
  DCHECK(title);

  NSArray* urlArray = nil;
  NSArray* titleArray = nil;
  if (ClipboardUtil::URLsAndTitlesFromPasteboard(pasteboard_->get(), &urlArray,
                                                 &titleArray)) {
    *url = GURL(base::SysNSStringToUTF8([urlArray firstObject]));
    *title = base::SysNSStringToUTF16([titleArray firstObject]);
    return true;
  }

  // If there are no URLS, try to convert a filename to a URL if the policy
  // allows it. The title remains blank.
  base::FilePath path;
  if (policy != OSExchangeData::DO_NOT_CONVERT_FILENAMES &&
      GetFilename(&path)) {
    NSURL* fileUrl =
        [NSURL fileURLWithPath:base::SysUTF8ToNSString(path.value())];
    *url =
        GURL([[fileUrl absoluteString] stringByStandardizingPath].UTF8String);
    return true;
  }

  return false;
}

bool OSExchangeDataProviderMac::GetFilename(base::FilePath* path) const {
  NSArray* paths =
      [pasteboard_->get() propertyListForType:NSFilenamesPboardType];
  if ([paths count] == 0)
    return false;

  *path = base::FilePath([[paths objectAtIndex:0] UTF8String]);
  return true;
}

bool OSExchangeDataProviderMac::GetFilenames(
    std::vector<FileInfo>* filenames) const {
  NOTIMPLEMENTED();
  return false;
}

bool OSExchangeDataProviderMac::GetPickledData(
    const Clipboard::FormatType& format,
    base::Pickle* data) const {
  DCHECK(data);
  NSData* ns_data = [pasteboard_->get() dataForType:format.ToNSString()];
  if (!ns_data)
    return false;

  *data =
      base::Pickle(static_cast<const char*>([ns_data bytes]), [ns_data length]);
  return true;
}

bool OSExchangeDataProviderMac::HasString() const {
  base::string16 string;
  return GetString(&string);
}

bool OSExchangeDataProviderMac::HasURL(
    OSExchangeData::FilenameToURLPolicy policy) const {
  GURL url;
  base::string16 title;
  return GetURLAndTitle(policy, &url, &title);
}

bool OSExchangeDataProviderMac::HasFile() const {
  return [[pasteboard_->get() types] containsObject:NSFilenamesPboardType];
}

bool OSExchangeDataProviderMac::HasCustomFormat(
    const Clipboard::FormatType& format) const {
  return [[pasteboard_->get() types] containsObject:format.ToNSString()];
}

void OSExchangeDataProviderMac::SetDragImage(
    const gfx::ImageSkia& image,
    const gfx::Vector2d& cursor_offset) {
  drag_image_ = image;
  cursor_offset_ = cursor_offset;
}

gfx::ImageSkia OSExchangeDataProviderMac::GetDragImage() const {
  return drag_image_;
}

gfx::Vector2d OSExchangeDataProviderMac::GetDragImageOffset() const {
  return cursor_offset_;
}

NSData* OSExchangeDataProviderMac::GetNSDataForType(NSString* type) const {
  return [pasteboard_->get() dataForType:type];
}

// static
std::unique_ptr<OSExchangeData>
OSExchangeDataProviderMac::CreateDataFromPasteboard(NSPasteboard* pasteboard) {
  OSExchangeDataProviderMac* provider = new OSExchangeDataProviderMac();

  for (NSPasteboardItem* item in [pasteboard pasteboardItems])
    ClipboardUtil::AddDataToPasteboard(provider->pasteboard_->get(), item);

  return base::MakeUnique<OSExchangeData>(
      base::WrapUnique<OSExchangeData::Provider>(provider));
}

// static
NSArray* OSExchangeDataProviderMac::SupportedPasteboardTypes() {
  return @[
    kWebCustomDataPboardType, ui::ClipboardUtil::UTIForWebURLsAndTitles(),
    NSURLPboardType, NSFilenamesPboardType, ui::kChromeDragDummyPboardType,
    NSStringPboardType, NSHTMLPboardType, NSRTFPboardType,
    NSFilenamesPboardType, ui::kWebCustomDataPboardType, NSPasteboardTypeString
  ];
}

}  // namespace ui

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ppapi/proxy/ppapi_param_traits.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>  // For memcpy

#include "base/macros.h"
#include "build/build_config.h"
#include "ppapi/c/pp_resource.h"
#include "ppapi/proxy/ppapi_messages.h"
#include "ppapi/proxy/serialized_flash_menu.h"
#include "ppapi/proxy/serialized_var.h"
#include "ppapi/shared_impl/host_resource.h"
#include "ppapi/shared_impl/private/ppb_x509_certificate_private_shared.h"

namespace IPC {

namespace {

// Deserializes a vector from IPC. This special version must be used instead
// of the default IPC version when the vector contains a SerializedVar, either
// directly or indirectly (i.e. a vector of objects that have a SerializedVar
// inside them).
//
// The default vector deserializer does resize and then we deserialize into
// those allocated slots. However, the implementation of vector (at least in
// GCC's implementation), creates a new empty object using the default
// constructor, and then sets the rest of the items to that empty one using the
// copy constructor.
//
// Since we allocate the inner class when you call the default constructor and
// transfer the inner class when you do operator=, the entire vector will end
// up referring to the same inner class. Deserializing into this will just end
// up overwriting the same item over and over, since all the SerializedVars
// will refer to the same thing.
//
// The solution is to make a new object for each deserialized item, and then
// add it to the vector one at a time.
template <typename T>
bool ReadVectorWithoutCopy(const base::Pickle* m,
                           base::PickleIterator* iter,
                           std::vector<T>* output) {
  // This part is just a copy of the the default ParamTraits vector Read().
  int size;
  // ReadLength() checks for < 0 itself.
  if (!iter->ReadLength(&size))
    return false;
  // Resizing beforehand is not safe, see BUG 1006367 for details.
  if (INT_MAX / sizeof(T) <= static_cast<size_t>(size))
    return false;

  output->reserve(size);
  for (int i = 0; i < size; i++) {
    T cur;
    if (!ReadParam(m, iter, &cur))
      return false;
    output->push_back(cur);
  }
  return true;
}

// This serializes the vector of items to the IPC message in exactly the same
// way as the "regular" IPC vector serializer does. But having the code here
// saves us from having to copy this code into all ParamTraits that use the
// ReadVectorWithoutCopy function for deserializing.
template <typename T>
void WriteVectorWithoutCopy(base::Pickle* m, const std::vector<T>& p) {
  WriteParam(m, static_cast<int>(p.size()));
  for (size_t i = 0; i < p.size(); i++)
    WriteParam(m, p[i]);
}

}  // namespace

// PP_Bool ---------------------------------------------------------------------

// static
void ParamTraits<PP_Bool>::Write(base::Pickle* m, const param_type& p) {
  WriteParam(m, PP_ToBool(p));
}

// static
bool ParamTraits<PP_Bool>::Read(const base::Pickle* m,
                                base::PickleIterator* iter,
                                param_type* r) {
  // We specifically want to be strict here about what types of input we accept,
  // which ParamTraits<bool> does for us. We don't want to deserialize "2" into
  // a PP_Bool, for example.
  bool result = false;
  if (!ReadParam(m, iter, &result))
    return false;
  *r = PP_FromBool(result);
  return true;
}

// static
void ParamTraits<PP_Bool>::Log(const param_type& p, std::string* l) {
}

// PP_KeyInformation -------------------------------------------------------

// static
void ParamTraits<PP_KeyInformation>::Write(base::Pickle* m,
                                           const param_type& p) {
  WriteParam(m, p.key_id_size);
  m->WriteBytes(p.key_id, static_cast<int>(p.key_id_size));
  WriteParam(m, p.key_status);
  WriteParam(m, p.system_code);
}

// static
bool ParamTraits<PP_KeyInformation>::Read(const base::Pickle* m,
                                          base::PickleIterator* iter,
                                          param_type* p) {
  uint32_t size;
  if (!ReadParam(m, iter, &size))
    return false;
  if (size > sizeof(p->key_id))
    return false;
  p->key_id_size = size;

  const char* data;
  if (!iter->ReadBytes(&data, size))
    return false;
  memcpy(p->key_id, data, size);

  PP_CdmKeyStatus key_status;
  if (!ReadParam(m, iter, &key_status))
    return false;
  p->key_status = key_status;

  uint32_t system_code;
  if (!ReadParam(m, iter, &system_code))
    return false;
  p->system_code = system_code;

  return true;
}

// static
void ParamTraits<PP_KeyInformation>::Log(const param_type& p, std::string* l) {
  l->append("<PP_KeyInformation (");
  LogParam(p.key_id_size, l);
  l->append(" bytes)>");
}

// PP_NetAddress_Private -------------------------------------------------------

// static
void ParamTraits<PP_NetAddress_Private>::Write(base::Pickle* m,
                                               const param_type& p) {
  WriteParam(m, p.size);
  m->WriteBytes(p.data, static_cast<int>(p.size));
}

// static
bool ParamTraits<PP_NetAddress_Private>::Read(const base::Pickle* m,
                                              base::PickleIterator* iter,
                                              param_type* p) {
  uint16_t size;
  if (!ReadParam(m, iter, &size))
    return false;
  if (size > sizeof(p->data))
    return false;
  p->size = size;

  const char* data;
  if (!iter->ReadBytes(&data, size))
    return false;
  memcpy(p->data, data, size);
  return true;
}

// static
void ParamTraits<PP_NetAddress_Private>::Log(const param_type& p,
                                             std::string* l) {
  l->append("<PP_NetAddress_Private (");
  LogParam(p.size, l);
  l->append(" bytes)>");
}

// HostResource ----------------------------------------------------------------

// static
void ParamTraits<ppapi::HostResource>::Write(base::Pickle* m,
                                             const param_type& p) {
  WriteParam(m, p.instance());
  WriteParam(m, p.host_resource());
}

// static
bool ParamTraits<ppapi::HostResource>::Read(const base::Pickle* m,
                                            base::PickleIterator* iter,
                                            param_type* r) {
  PP_Instance instance;
  PP_Resource resource;
  if (!ReadParam(m, iter, &instance) || !ReadParam(m, iter, &resource))
    return false;
  r->SetHostResource(instance, resource);
  return true;
}

// static
void ParamTraits<ppapi::HostResource>::Log(const param_type& p,
                                           std::string* l) {
}

// SerializedVar ---------------------------------------------------------------

// static
void ParamTraits<ppapi::proxy::SerializedVar>::Write(base::Pickle* m,
                                                     const param_type& p) {
  p.WriteToMessage(m);
}

// static
bool ParamTraits<ppapi::proxy::SerializedVar>::Read(const base::Pickle* m,
                                                    base::PickleIterator* iter,
                                                    param_type* r) {
  return r->ReadFromMessage(m, iter);
}

// static
void ParamTraits<ppapi::proxy::SerializedVar>::Log(const param_type& p,
                                                   std::string* l) {
}

// std::vector<SerializedVar> --------------------------------------------------

void ParamTraits<std::vector<ppapi::proxy::SerializedVar>>::Write(
    base::Pickle* m,
    const param_type& p) {
  WriteVectorWithoutCopy(m, p);
}

// static
bool ParamTraits<std::vector<ppapi::proxy::SerializedVar>>::Read(
    const base::Pickle* m,
    base::PickleIterator* iter,
    param_type* r) {
  return ReadVectorWithoutCopy(m, iter, r);
}

// static
void ParamTraits< std::vector<ppapi::proxy::SerializedVar> >::Log(
    const param_type& p,
    std::string* l) {
}

// ppapi::PpapiPermissions -----------------------------------------------------

// static
void ParamTraits<ppapi::PpapiPermissions>::Write(base::Pickle* m,
                                                 const param_type& p) {
  WriteParam(m, p.GetBits());
}

// static
bool ParamTraits<ppapi::PpapiPermissions>::Read(const base::Pickle* m,
                                                base::PickleIterator* iter,
                                                param_type* r) {
  uint32_t bits;
  if (!ReadParam(m, iter, &bits))
    return false;
  *r = ppapi::PpapiPermissions(bits);
  return true;
}

// static
void ParamTraits<ppapi::PpapiPermissions>::Log(const param_type& p,
                                               std::string* l) {
}

// SerializedHandle ------------------------------------------------------------

// static
void ParamTraits<ppapi::proxy::SerializedHandle>::Write(base::Pickle* m,
                                                        const param_type& p) {
  ppapi::proxy::SerializedHandle::WriteHeader(p.header(), m);
  switch (p.type()) {
    case ppapi::proxy::SerializedHandle::SHARED_MEMORY:
      WriteParam(m, p.shmem());
      break;
    case ppapi::proxy::SerializedHandle::SOCKET:
    case ppapi::proxy::SerializedHandle::FILE:
      WriteParam(m, p.descriptor());
      break;
    case ppapi::proxy::SerializedHandle::INVALID:
      break;
    // No default so the compiler will warn on new types.
  }
}

// static
bool ParamTraits<ppapi::proxy::SerializedHandle>::Read(
    const base::Pickle* m,
    base::PickleIterator* iter,
    param_type* r) {
  ppapi::proxy::SerializedHandle::Header header;
  if (!ppapi::proxy::SerializedHandle::ReadHeader(iter, &header))
    return false;
  switch (header.type) {
    case ppapi::proxy::SerializedHandle::SHARED_MEMORY: {
      base::SharedMemoryHandle handle;
      if (ReadParam(m, iter, &handle)) {
        r->set_shmem(handle, header.size);
        return true;
      }
      break;
    }
    case ppapi::proxy::SerializedHandle::SOCKET: {
      IPC::PlatformFileForTransit socket;
      if (ReadParam(m, iter, &socket)) {
        r->set_socket(socket);
        return true;
      }
      break;
    }
    case ppapi::proxy::SerializedHandle::FILE: {
      IPC::PlatformFileForTransit desc;
      if (ReadParam(m, iter, &desc)) {
        r->set_file_handle(desc, header.open_flags, header.file_io);
        return true;
      }
      break;
    }
    case ppapi::proxy::SerializedHandle::INVALID:
      return true;
    // No default so the compiler will warn us if a new type is added.
  }
  return false;
}

// static
void ParamTraits<ppapi::proxy::SerializedHandle>::Log(const param_type& p,
                                                      std::string* l) {
}

// PPBURLLoader_UpdateProgress_Params ------------------------------------------

// static
void ParamTraits<ppapi::proxy::PPBURLLoader_UpdateProgress_Params>::Write(
    base::Pickle* m,
    const param_type& p) {
  WriteParam(m, p.instance);
  WriteParam(m, p.resource);
  WriteParam(m, p.bytes_sent);
  WriteParam(m, p.total_bytes_to_be_sent);
  WriteParam(m, p.bytes_received);
  WriteParam(m, p.total_bytes_to_be_received);
}

// static
bool ParamTraits<ppapi::proxy::PPBURLLoader_UpdateProgress_Params>::Read(
    const base::Pickle* m,
    base::PickleIterator* iter,
    param_type* r) {
  return ReadParam(m, iter, &r->instance) && ReadParam(m, iter, &r->resource) &&
         ReadParam(m, iter, &r->bytes_sent) &&
         ReadParam(m, iter, &r->total_bytes_to_be_sent) &&
         ReadParam(m, iter, &r->bytes_received) &&
         ReadParam(m, iter, &r->total_bytes_to_be_received);
}

// static
void ParamTraits<ppapi::proxy::PPBURLLoader_UpdateProgress_Params>::Log(
    const param_type& p,
    std::string* l) {
}

#if !defined(OS_NACL) && !defined(NACL_WIN64)
// PPBFlash_DrawGlyphs_Params --------------------------------------------------
// static
void ParamTraits<ppapi::proxy::PPBFlash_DrawGlyphs_Params>::Write(
    base::Pickle* m,
    const param_type& p) {
  WriteParam(m, p.instance);
  WriteParam(m, p.image_data);
  WriteParam(m, p.font_desc);
  WriteParam(m, p.color);
  WriteParam(m, p.position);
  WriteParam(m, p.clip);
  WriteParam(m, p.transformation[0][0]);
  WriteParam(m, p.transformation[0][1]);
  WriteParam(m, p.transformation[0][2]);
  WriteParam(m, p.transformation[1][0]);
  WriteParam(m, p.transformation[1][1]);
  WriteParam(m, p.transformation[1][2]);
  WriteParam(m, p.transformation[2][0]);
  WriteParam(m, p.transformation[2][1]);
  WriteParam(m, p.transformation[2][2]);
  WriteParam(m, p.allow_subpixel_aa);
  WriteParam(m, p.glyph_indices);
  WriteParam(m, p.glyph_advances);
}

// static
bool ParamTraits<ppapi::proxy::PPBFlash_DrawGlyphs_Params>::Read(
    const base::Pickle* m,
    base::PickleIterator* iter,
    param_type* r) {
  return ReadParam(m, iter, &r->instance) &&
         ReadParam(m, iter, &r->image_data) &&
         ReadParam(m, iter, &r->font_desc) && ReadParam(m, iter, &r->color) &&
         ReadParam(m, iter, &r->position) && ReadParam(m, iter, &r->clip) &&
         ReadParam(m, iter, &r->transformation[0][0]) &&
         ReadParam(m, iter, &r->transformation[0][1]) &&
         ReadParam(m, iter, &r->transformation[0][2]) &&
         ReadParam(m, iter, &r->transformation[1][0]) &&
         ReadParam(m, iter, &r->transformation[1][1]) &&
         ReadParam(m, iter, &r->transformation[1][2]) &&
         ReadParam(m, iter, &r->transformation[2][0]) &&
         ReadParam(m, iter, &r->transformation[2][1]) &&
         ReadParam(m, iter, &r->transformation[2][2]) &&
         ReadParam(m, iter, &r->allow_subpixel_aa) &&
         ReadParam(m, iter, &r->glyph_indices) &&
         ReadParam(m, iter, &r->glyph_advances) &&
         r->glyph_indices.size() == r->glyph_advances.size();
}

// static
void ParamTraits<ppapi::proxy::PPBFlash_DrawGlyphs_Params>::Log(
    const param_type& p,
    std::string* l) {
}

// SerializedDirEntry ----------------------------------------------------------

// static
void ParamTraits<ppapi::proxy::SerializedDirEntry>::Write(base::Pickle* m,
                                                          const param_type& p) {
  WriteParam(m, p.name);
  WriteParam(m, p.is_dir);
}

// static
bool ParamTraits<ppapi::proxy::SerializedDirEntry>::Read(
    const base::Pickle* m,
    base::PickleIterator* iter,
    param_type* r) {
  return ReadParam(m, iter, &r->name) && ReadParam(m, iter, &r->is_dir);
}

// static
void ParamTraits<ppapi::proxy::SerializedDirEntry>::Log(const param_type& p,
                                                        std::string* l) {
}

// ppapi::proxy::SerializedFontDescription -------------------------------------

// static
void ParamTraits<ppapi::proxy::SerializedFontDescription>::Write(
    base::Pickle* m,
    const param_type& p) {
  WriteParam(m, p.face);
  WriteParam(m, p.family);
  WriteParam(m, p.size);
  WriteParam(m, p.weight);
  WriteParam(m, p.italic);
  WriteParam(m, p.small_caps);
  WriteParam(m, p.letter_spacing);
  WriteParam(m, p.word_spacing);
}

// static
bool ParamTraits<ppapi::proxy::SerializedFontDescription>::Read(
    const base::Pickle* m,
    base::PickleIterator* iter,
    param_type* r) {
  return ReadParam(m, iter, &r->face) && ReadParam(m, iter, &r->family) &&
         ReadParam(m, iter, &r->size) && ReadParam(m, iter, &r->weight) &&
         ReadParam(m, iter, &r->italic) && ReadParam(m, iter, &r->small_caps) &&
         ReadParam(m, iter, &r->letter_spacing) &&
         ReadParam(m, iter, &r->word_spacing);
}

// static
void ParamTraits<ppapi::proxy::SerializedFontDescription>::Log(
    const param_type& p,
    std::string* l) {
}
#endif  // !defined(OS_NACL) && !defined(NACL_WIN64)

// ppapi::proxy::SerializedTrueTypeFontDesc ------------------------------------

// static
void ParamTraits<ppapi::proxy::SerializedTrueTypeFontDesc>::Write(
    base::Pickle* m,
    const param_type& p) {
  WriteParam(m, p.family);
  WriteParam(m, p.generic_family);
  WriteParam(m, p.style);
  WriteParam(m, p.weight);
  WriteParam(m, p.width);
  WriteParam(m, p.charset);
}

// static
bool ParamTraits<ppapi::proxy::SerializedTrueTypeFontDesc>::Read(
    const base::Pickle* m,
    base::PickleIterator* iter,
    param_type* r) {
  return ReadParam(m, iter, &r->family) &&
         ReadParam(m, iter, &r->generic_family) &&
         ReadParam(m, iter, &r->style) && ReadParam(m, iter, &r->weight) &&
         ReadParam(m, iter, &r->width) && ReadParam(m, iter, &r->charset);
}

// static
void ParamTraits<ppapi::proxy::SerializedTrueTypeFontDesc>::Log(
    const param_type& p,
    std::string* l) {
}

#if !defined(OS_NACL) && !defined(NACL_WIN64)
// ppapi::PepperFilePath -------------------------------------------------------

// static
void ParamTraits<ppapi::PepperFilePath>::Write(base::Pickle* m,
                                               const param_type& p) {
  WriteParam(m, static_cast<unsigned>(p.domain()));
  WriteParam(m, p.path());
}

// static
bool ParamTraits<ppapi::PepperFilePath>::Read(const base::Pickle* m,
                                              base::PickleIterator* iter,
                                              param_type* p) {
  unsigned domain;
  base::FilePath path;
  if (!ReadParam(m, iter, &domain) || !ReadParam(m, iter, &path))
    return false;
  if (domain > ppapi::PepperFilePath::DOMAIN_MAX_VALID)
    return false;

  *p = ppapi::PepperFilePath(
      static_cast<ppapi::PepperFilePath::Domain>(domain), path);
  return true;
}

// static
void ParamTraits<ppapi::PepperFilePath>::Log(const param_type& p,
                                             std::string* l) {
  l->append("(");
  LogParam(static_cast<unsigned>(p.domain()), l);
  l->append(", ");
  LogParam(p.path(), l);
  l->append(")");
}

// SerializedFlashMenu ---------------------------------------------------------

// static
void ParamTraits<ppapi::proxy::SerializedFlashMenu>::Write(
    base::Pickle* m,
    const param_type& p) {
  p.WriteToMessage(m);
}

// static
bool ParamTraits<ppapi::proxy::SerializedFlashMenu>::Read(
    const base::Pickle* m,
    base::PickleIterator* iter,
    param_type* r) {
  return r->ReadFromMessage(m, iter);
}

// static
void ParamTraits<ppapi::proxy::SerializedFlashMenu>::Log(const param_type& p,
                                                         std::string* l) {
}
#endif  // !defined(OS_NACL) && !defined(NACL_WIN64)

// PPB_X509Certificate_Fields --------------------------------------------------

// static
void ParamTraits<ppapi::PPB_X509Certificate_Fields>::Write(
    base::Pickle* m,
    const param_type& p) {
  WriteParam(m, p.values_);
}

// static
bool ParamTraits<ppapi::PPB_X509Certificate_Fields>::Read(
    const base::Pickle* m,
    base::PickleIterator* iter,
    param_type* r) {
  return ReadParam(m, iter, &(r->values_));
}

// static
void ParamTraits<ppapi::PPB_X509Certificate_Fields>::Log(const param_type& p,
                                                         std::string* l) {
}

// ppapi::SocketOptionData -----------------------------------------------------

// static
void ParamTraits<ppapi::SocketOptionData>::Write(base::Pickle* m,
                                                 const param_type& p) {
  ppapi::SocketOptionData::Type type = p.GetType();
  WriteParam(m, static_cast<int32_t>(type));
  switch (type) {
    case ppapi::SocketOptionData::TYPE_INVALID: {
      break;
    }
    case ppapi::SocketOptionData::TYPE_BOOL: {
      bool out_value = false;
      bool result = p.GetBool(&out_value);
      // Suppress unused variable warnings.
      static_cast<void>(result);
      DCHECK(result);

      WriteParam(m, out_value);
      break;
    }
    case ppapi::SocketOptionData::TYPE_INT32: {
      int32_t out_value = 0;
      bool result = p.GetInt32(&out_value);
      // Suppress unused variable warnings.
      static_cast<void>(result);
      DCHECK(result);

      WriteParam(m, out_value);
      break;
    }
    // No default so the compiler will warn on new types.
  }
}

// static
bool ParamTraits<ppapi::SocketOptionData>::Read(const base::Pickle* m,
                                                base::PickleIterator* iter,
                                                param_type* r) {
  *r = ppapi::SocketOptionData();
  int32_t type = 0;
  if (!ReadParam(m, iter, &type))
    return false;
  if (type != ppapi::SocketOptionData::TYPE_INVALID &&
      type != ppapi::SocketOptionData::TYPE_BOOL &&
      type != ppapi::SocketOptionData::TYPE_INT32) {
    return false;
  }
  switch (static_cast<ppapi::SocketOptionData::Type>(type)) {
    case ppapi::SocketOptionData::TYPE_INVALID: {
      return true;
    }
    case ppapi::SocketOptionData::TYPE_BOOL: {
      bool value = false;
      if (!ReadParam(m, iter, &value))
        return false;
      r->SetBool(value);
      return true;
    }
    case ppapi::SocketOptionData::TYPE_INT32: {
      int32_t value = 0;
      if (!ReadParam(m, iter, &value))
        return false;
      r->SetInt32(value);
      return true;
    }
    // No default so the compiler will warn on new types.
  }
  return false;
}

// static
void ParamTraits<ppapi::SocketOptionData>::Log(const param_type& p,
                                               std::string* l) {
}

}  // namespace IPC

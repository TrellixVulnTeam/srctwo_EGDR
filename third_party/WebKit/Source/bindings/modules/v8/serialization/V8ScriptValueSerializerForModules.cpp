// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bindings/modules/v8/serialization/V8ScriptValueSerializerForModules.h"

#include "bindings/modules/v8/V8CryptoKey.h"
#include "bindings/modules/v8/V8DOMFileSystem.h"
#include "bindings/modules/v8/V8RTCCertificate.h"
#include "bindings/modules/v8/serialization/WebCryptoSubTags.h"
#include "platform/FileSystemType.h"
#include "platform/bindings/ScriptWrappable.h"
#include "public/platform/Platform.h"
#include "public/platform/WebCrypto.h"
#include "public/platform/WebCryptoKey.h"
#include "public/platform/WebCryptoKeyAlgorithm.h"

namespace blink {

bool V8ScriptValueSerializerForModules::WriteDOMObject(
    ScriptWrappable* wrappable,
    ExceptionState& exception_state) {
  // Give the core/ implementation a chance to try first.
  // If it didn't recognize the kind of wrapper, try the modules types.
  if (V8ScriptValueSerializer::WriteDOMObject(wrappable, exception_state))
    return true;
  if (exception_state.HadException())
    return false;

  const WrapperTypeInfo* wrapper_type_info = wrappable->GetWrapperTypeInfo();
  if (wrapper_type_info == &V8CryptoKey::wrapperTypeInfo) {
    return WriteCryptoKey(wrappable->ToImpl<CryptoKey>()->Key(),
                          exception_state);
  }
  if (wrapper_type_info == &V8DOMFileSystem::wrapperTypeInfo) {
    DOMFileSystem* fs = wrappable->ToImpl<DOMFileSystem>();
    if (!fs->Clonable()) {
      exception_state.ThrowDOMException(
          kDataCloneError, "A FileSystem object could not be cloned.");
      return false;
    }
    WriteTag(kDOMFileSystemTag);
    // This locks in the values of the FileSystemType enumerators.
    WriteUint32(static_cast<uint32_t>(fs->GetType()));
    WriteUTF8String(fs->name());
    WriteUTF8String(fs->RootURL().GetString());
    return true;
  }
  if (wrapper_type_info == &V8RTCCertificate::wrapperTypeInfo) {
    RTCCertificate* certificate = wrappable->ToImpl<RTCCertificate>();
    WebRTCCertificatePEM pem = certificate->Certificate().ToPEM();
    WriteTag(kRTCCertificateTag);
    WriteUTF8String(pem.PrivateKey());
    WriteUTF8String(pem.Certificate());
    return true;
  }
  return false;
}

namespace {

uint32_t AlgorithmIdForWireFormat(WebCryptoAlgorithmId id) {
  switch (id) {
    case kWebCryptoAlgorithmIdAesCbc:
      return kAesCbcTag;
    case kWebCryptoAlgorithmIdHmac:
      return kHmacTag;
    case kWebCryptoAlgorithmIdRsaSsaPkcs1v1_5:
      return kRsaSsaPkcs1v1_5Tag;
    case kWebCryptoAlgorithmIdSha1:
      return kSha1Tag;
    case kWebCryptoAlgorithmIdSha256:
      return kSha256Tag;
    case kWebCryptoAlgorithmIdSha384:
      return kSha384Tag;
    case kWebCryptoAlgorithmIdSha512:
      return kSha512Tag;
    case kWebCryptoAlgorithmIdAesGcm:
      return kAesGcmTag;
    case kWebCryptoAlgorithmIdRsaOaep:
      return kRsaOaepTag;
    case kWebCryptoAlgorithmIdAesCtr:
      return kAesCtrTag;
    case kWebCryptoAlgorithmIdAesKw:
      return kAesKwTag;
    case kWebCryptoAlgorithmIdRsaPss:
      return kRsaPssTag;
    case kWebCryptoAlgorithmIdEcdsa:
      return kEcdsaTag;
    case kWebCryptoAlgorithmIdEcdh:
      return kEcdhTag;
    case kWebCryptoAlgorithmIdHkdf:
      return kHkdfTag;
    case kWebCryptoAlgorithmIdPbkdf2:
      return kPbkdf2Tag;
  }
  NOTREACHED() << "Unknown algorithm ID " << id;
  return 0;
}

uint32_t AsymmetricKeyTypeForWireFormat(WebCryptoKeyType key_type) {
  switch (key_type) {
    case kWebCryptoKeyTypePublic:
      return kPublicKeyType;
    case kWebCryptoKeyTypePrivate:
      return kPrivateKeyType;
    case kWebCryptoKeyTypeSecret:
      break;
  }
  NOTREACHED() << "Unknown asymmetric key type " << key_type;
  return 0;
}

uint32_t NamedCurveForWireFormat(WebCryptoNamedCurve named_curve) {
  switch (named_curve) {
    case kWebCryptoNamedCurveP256:
      return kP256Tag;
    case kWebCryptoNamedCurveP384:
      return kP384Tag;
    case kWebCryptoNamedCurveP521:
      return kP521Tag;
  }
  NOTREACHED() << "Unknown named curve " << named_curve;
  return 0;
}

uint32_t KeyUsagesForWireFormat(WebCryptoKeyUsageMask usages,
                                bool extractable) {
  // Reminder to update this when adding new key usages.
  static_assert(kEndOfWebCryptoKeyUsage == (1 << 7) + 1,
                "update required when adding new key usages");
  uint32_t value = 0;
  if (extractable)
    value |= kExtractableUsage;
  if (usages & kWebCryptoKeyUsageEncrypt)
    value |= kEncryptUsage;
  if (usages & kWebCryptoKeyUsageDecrypt)
    value |= kDecryptUsage;
  if (usages & kWebCryptoKeyUsageSign)
    value |= kSignUsage;
  if (usages & kWebCryptoKeyUsageVerify)
    value |= kVerifyUsage;
  if (usages & kWebCryptoKeyUsageDeriveKey)
    value |= kDeriveKeyUsage;
  if (usages & kWebCryptoKeyUsageWrapKey)
    value |= kWrapKeyUsage;
  if (usages & kWebCryptoKeyUsageUnwrapKey)
    value |= kUnwrapKeyUsage;
  if (usages & kWebCryptoKeyUsageDeriveBits)
    value |= kDeriveBitsUsage;
  return value;
}

}  // namespace

bool V8ScriptValueSerializerForModules::WriteCryptoKey(
    const WebCryptoKey& key,
    ExceptionState& exception_state) {
  WriteTag(kCryptoKeyTag);

  // Write params.
  const WebCryptoKeyAlgorithm& algorithm = key.Algorithm();
  switch (algorithm.ParamsType()) {
    case kWebCryptoKeyAlgorithmParamsTypeAes: {
      const auto& params = *algorithm.AesParams();
      WriteOneByte(kAesKeyTag);
      WriteUint32(AlgorithmIdForWireFormat(algorithm.Id()));
      DCHECK_EQ(0, params.LengthBits() % 8);
      WriteUint32(params.LengthBits() / 8);
      break;
    }
    case kWebCryptoKeyAlgorithmParamsTypeHmac: {
      const auto& params = *algorithm.HmacParams();
      WriteOneByte(kHmacKeyTag);
      DCHECK_EQ(0u, params.LengthBits() % 8);
      WriteUint32(params.LengthBits() / 8);
      WriteUint32(AlgorithmIdForWireFormat(params.GetHash().Id()));
      break;
    }
    case kWebCryptoKeyAlgorithmParamsTypeRsaHashed: {
      const auto& params = *algorithm.RsaHashedParams();
      WriteOneByte(kRsaHashedKeyTag);
      WriteUint32(AlgorithmIdForWireFormat(algorithm.Id()));
      WriteUint32(AsymmetricKeyTypeForWireFormat(key.GetType()));
      WriteUint32(params.ModulusLengthBits());
      WriteUint32(params.PublicExponent().size());
      WriteRawBytes(params.PublicExponent().Data(),
                    params.PublicExponent().size());
      WriteUint32(AlgorithmIdForWireFormat(params.GetHash().Id()));
      break;
    }
    case kWebCryptoKeyAlgorithmParamsTypeEc: {
      const auto& params = *algorithm.EcParams();
      WriteOneByte(kEcKeyTag);
      WriteUint32(AlgorithmIdForWireFormat(algorithm.Id()));
      WriteUint32(AsymmetricKeyTypeForWireFormat(key.GetType()));
      WriteUint32(NamedCurveForWireFormat(params.NamedCurve()));
      break;
    }
    case kWebCryptoKeyAlgorithmParamsTypeNone:
      DCHECK(WebCryptoAlgorithm::IsKdf(algorithm.Id()));
      WriteOneByte(kNoParamsKeyTag);
      WriteUint32(AlgorithmIdForWireFormat(algorithm.Id()));
      break;
  }

  // Write key usages.
  WriteUint32(KeyUsagesForWireFormat(key.Usages(), key.Extractable()));

  // Write key data.
  WebVector<uint8_t> key_data;
  if (!Platform::Current()->Crypto()->SerializeKeyForClone(key, key_data) ||
      key_data.size() > std::numeric_limits<uint32_t>::max()) {
    exception_state.ThrowDOMException(
        kDataCloneError, "A CryptoKey object could not be cloned.");
    return false;
  }
  WriteUint32(key_data.size());
  WriteRawBytes(key_data.Data(), key_data.size());

  return true;
}

}  // namespace blink

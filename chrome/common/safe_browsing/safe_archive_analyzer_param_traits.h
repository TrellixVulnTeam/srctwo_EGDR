// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Multiply-included param traits file, so no include guard.

#if !defined(FULL_SAFE_BROWSING)
#error FULL_SAFE_BROWSING should be defined.
#endif

#include "build/build_config.h"
#include "chrome/common/safe_browsing/archive_analyzer_results.h"
#include "chrome/common/safe_browsing/ipc_protobuf_message_macros.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_message_protobuf_utils.h"

IPC_ENUM_TRAITS_VALIDATE(
    safe_browsing::ClientDownloadRequest_DownloadType,
    safe_browsing::ClientDownloadRequest_DownloadType_IsValid(value))

IPC_PROTOBUF_MESSAGE_TRAITS_BEGIN(safe_browsing::ClientDownloadRequest_Digests)
  IPC_PROTOBUF_MESSAGE_TRAITS_OPTIONAL_COMPLEX_MEMBER(sha256)
  IPC_PROTOBUF_MESSAGE_TRAITS_OPTIONAL_COMPLEX_MEMBER(sha1)
  IPC_PROTOBUF_MESSAGE_TRAITS_OPTIONAL_COMPLEX_MEMBER(md5)
IPC_PROTOBUF_MESSAGE_TRAITS_END()

IPC_PROTOBUF_MESSAGE_TRAITS_BEGIN(
    safe_browsing::ClientDownloadRequest_CertificateChain_Element)
  IPC_PROTOBUF_MESSAGE_TRAITS_OPTIONAL_COMPLEX_MEMBER(certificate)
IPC_PROTOBUF_MESSAGE_TRAITS_END()

IPC_PROTOBUF_MESSAGE_TRAITS_BEGIN(
    safe_browsing::ClientDownloadRequest_CertificateChain)
  IPC_PROTOBUF_MESSAGE_TRAITS_REPEATED_COMPLEX_MEMBER(element)
IPC_PROTOBUF_MESSAGE_TRAITS_END()

IPC_PROTOBUF_MESSAGE_TRAITS_BEGIN(
    safe_browsing::ClientDownloadRequest_SignatureInfo)
  IPC_PROTOBUF_MESSAGE_TRAITS_REPEATED_COMPLEX_MEMBER(certificate_chain)
  IPC_PROTOBUF_MESSAGE_TRAITS_OPTIONAL_FUNDAMENTAL_MEMBER(trusted)
  IPC_PROTOBUF_MESSAGE_TRAITS_REPEATED_COMPLEX_MEMBER(signed_data)
IPC_PROTOBUF_MESSAGE_TRAITS_END()

IPC_PROTOBUF_MESSAGE_TRAITS_BEGIN(
    safe_browsing::ClientDownloadRequest_PEImageHeaders_DebugData)
  IPC_PROTOBUF_MESSAGE_TRAITS_OPTIONAL_COMPLEX_MEMBER(directory_entry)
  IPC_PROTOBUF_MESSAGE_TRAITS_OPTIONAL_COMPLEX_MEMBER(raw_data)
IPC_PROTOBUF_MESSAGE_TRAITS_END()

IPC_PROTOBUF_MESSAGE_TRAITS_BEGIN(
    safe_browsing::ClientDownloadRequest_PEImageHeaders)
  IPC_PROTOBUF_MESSAGE_TRAITS_OPTIONAL_COMPLEX_MEMBER(dos_header)
  IPC_PROTOBUF_MESSAGE_TRAITS_OPTIONAL_COMPLEX_MEMBER(file_header)
  IPC_PROTOBUF_MESSAGE_TRAITS_OPTIONAL_COMPLEX_MEMBER(optional_headers32)
  IPC_PROTOBUF_MESSAGE_TRAITS_OPTIONAL_COMPLEX_MEMBER(optional_headers64)
  IPC_PROTOBUF_MESSAGE_TRAITS_REPEATED_COMPLEX_MEMBER(section_header)
  IPC_PROTOBUF_MESSAGE_TRAITS_OPTIONAL_COMPLEX_MEMBER(export_section_data)
  IPC_PROTOBUF_MESSAGE_TRAITS_REPEATED_COMPLEX_MEMBER(debug_data)
IPC_PROTOBUF_MESSAGE_TRAITS_END()

IPC_PROTOBUF_MESSAGE_TRAITS_BEGIN(
    safe_browsing::ClientDownloadRequest_MachOHeaders_LoadCommand)
  IPC_PROTOBUF_MESSAGE_TRAITS_OPTIONAL_FUNDAMENTAL_MEMBER(command_id)
  IPC_PROTOBUF_MESSAGE_TRAITS_REPEATED_COMPLEX_MEMBER(command)
IPC_PROTOBUF_MESSAGE_TRAITS_END()

IPC_PROTOBUF_MESSAGE_TRAITS_BEGIN(
    safe_browsing::ClientDownloadRequest_MachOHeaders)
  IPC_PROTOBUF_MESSAGE_TRAITS_REPEATED_COMPLEX_MEMBER(mach_header)
  IPC_PROTOBUF_MESSAGE_TRAITS_REPEATED_COMPLEX_MEMBER(load_commands)
IPC_PROTOBUF_MESSAGE_TRAITS_END()

IPC_PROTOBUF_MESSAGE_TRAITS_BEGIN(
    safe_browsing::ClientDownloadRequest_ImageHeaders)
  IPC_PROTOBUF_MESSAGE_TRAITS_OPTIONAL_COMPLEX_MEMBER(pe_headers)
  IPC_PROTOBUF_MESSAGE_TRAITS_REPEATED_COMPLEX_MEMBER(mach_o_headers)
IPC_PROTOBUF_MESSAGE_TRAITS_END()

IPC_PROTOBUF_MESSAGE_TRAITS_BEGIN(
    safe_browsing::ClientDownloadRequest_ArchivedBinary)
  IPC_PROTOBUF_MESSAGE_TRAITS_OPTIONAL_COMPLEX_MEMBER(file_basename)
  IPC_PROTOBUF_MESSAGE_TRAITS_OPTIONAL_FUNDAMENTAL_MEMBER(download_type)
  IPC_PROTOBUF_MESSAGE_TRAITS_OPTIONAL_COMPLEX_MEMBER(digests)
  IPC_PROTOBUF_MESSAGE_TRAITS_OPTIONAL_FUNDAMENTAL_MEMBER(length)
  IPC_PROTOBUF_MESSAGE_TRAITS_OPTIONAL_COMPLEX_MEMBER(signature)
  IPC_PROTOBUF_MESSAGE_TRAITS_OPTIONAL_COMPLEX_MEMBER(image_headers)
IPC_PROTOBUF_MESSAGE_TRAITS_END()

IPC_STRUCT_TRAITS_BEGIN(safe_browsing::ArchiveAnalyzerResults)
  IPC_STRUCT_TRAITS_MEMBER(success)
  IPC_STRUCT_TRAITS_MEMBER(has_executable)
  IPC_STRUCT_TRAITS_MEMBER(has_archive)
  IPC_STRUCT_TRAITS_MEMBER(archived_binary)
  IPC_STRUCT_TRAITS_MEMBER(archived_archive_filenames)
#if defined(OS_MACOSX)
  IPC_STRUCT_TRAITS_MEMBER(signature_blob)
#endif  // OS_MACOSX
IPC_STRUCT_TRAITS_END()

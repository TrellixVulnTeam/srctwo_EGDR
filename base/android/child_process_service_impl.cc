// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "base/android/library_loader/library_loader_hooks.h"
#include "base/file_descriptor_store.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/optional.h"
#include "base/posix/global_descriptors.h"
#include "jni/ChildProcessServiceImpl_jni.h"

using base::android::JavaIntArrayToIntVector;
using base::android::JavaParamRef;

namespace base {
namespace android {

void RegisterFileDescriptors(JNIEnv* env,
                             const JavaParamRef<jclass>& clazz,
                             const JavaParamRef<jobjectArray>& j_keys,
                             const JavaParamRef<jintArray>& j_ids,
                             const JavaParamRef<jintArray>& j_fds,
                             const JavaParamRef<jlongArray>& j_offsets,
                             const JavaParamRef<jlongArray>& j_sizes) {
  std::vector<base::Optional<std::string>> keys;
  jsize keys_size = env->GetArrayLength(j_keys);
  keys.reserve(keys_size);
  for (jsize i = 0; i < keys_size; i++) {
    base::android::ScopedJavaLocalRef<jstring> str(
        env, static_cast<jstring>(env->GetObjectArrayElement(j_keys, i)));
    base::Optional<std::string> key;
    if (!str.is_null()) {
      key = base::android::ConvertJavaStringToUTF8(env, str);
    }
    keys.push_back(std::move(key));
  }

  std::vector<int> ids;
  base::android::JavaIntArrayToIntVector(env, j_ids, &ids);
  std::vector<int> fds;
  base::android::JavaIntArrayToIntVector(env, j_fds, &fds);
  std::vector<int64_t> offsets;
  base::android::JavaLongArrayToInt64Vector(env, j_offsets, &offsets);
  std::vector<int64_t> sizes;
  base::android::JavaLongArrayToInt64Vector(env, j_sizes, &sizes);

  DCHECK_EQ(keys.size(), ids.size());
  DCHECK_EQ(ids.size(), fds.size());
  DCHECK_EQ(fds.size(), offsets.size());
  DCHECK_EQ(offsets.size(), sizes.size());

  for (size_t i = 0; i < ids.size(); i++) {
    base::MemoryMappedFile::Region region = {offsets.at(i), sizes.at(i)};
    const base::Optional<std::string>& key = keys.at(i);
    int id = ids.at(i);
    int fd = fds.at(i);
    if (key) {
      base::FileDescriptorStore::GetInstance().Set(*key, base::ScopedFD(fd),
                                                   region);
    } else {
      base::GlobalDescriptors::GetInstance()->Set(id, fd, region);
    }
  }
}

void ExitChildProcess(JNIEnv* env, const JavaParamRef<jclass>& clazz) {
  VLOG(0) << "ChildProcessServiceImpl: Exiting child process.";
  base::android::LibraryLoaderExitHook();
  _exit(0);
}

}  // namespace android
}  // namespace base

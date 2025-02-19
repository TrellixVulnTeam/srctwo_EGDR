// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_CRONET_ANDROID_IO_BUFFER_WITH_BYTE_BUFFER_H_
#define COMPONENTS_CRONET_ANDROID_IO_BUFFER_WITH_BYTE_BUFFER_H_

#include <jni.h>

#include "base/android/scoped_java_ref.h"
#include "net/base/io_buffer.h"

namespace cronet {

// net::WrappedIOBuffer subclass for a buffer owned by a Java ByteBuffer. Keeps
// the ByteBuffer alive until destroyed. Uses WrappedIOBuffer because data() is
// owned by the embedder.
class IOBufferWithByteBuffer : public net::WrappedIOBuffer {
 public:
  // Creates a buffer wrapping the Java ByteBuffer |jbyte_buffer|.
  // |byte_buffer_data| points to the memory backed by the ByteBuffer, and
  // |position| is the index of the first byte of data inside of the buffer.
  // |limit| is the the index of the first element that should not be read or
  // written, preserved to verify that buffer is not changed externally during
  // networking operations.
  IOBufferWithByteBuffer(
      JNIEnv* env,
      const base::android::JavaParamRef<jobject>& jbyte_buffer,
      void* byte_buffer_data,
      jint position,
      jint limit);

  jint initial_position() const { return initial_position_; }
  jint initial_limit() const { return initial_limit_; }

  const base::android::JavaRef<jobject>& byte_buffer() const {
    return byte_buffer_;
  }

 private:
  ~IOBufferWithByteBuffer() override;

  base::android::ScopedJavaGlobalRef<jobject> byte_buffer_;

  const jint initial_position_;
  const jint initial_limit_;

  DISALLOW_COPY_AND_ASSIGN(IOBufferWithByteBuffer);
};

}  // namespace cronet

#endif  // COMPONENTS_CRONET_ANDROID_IO_BUFFER_WITH_BYTE_BUFFER_H_

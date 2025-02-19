// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// CompoundBuffer implements a data buffer that is composed of several pieces,
// each stored in a refcounted IOBuffer. It is needed for encoding/decoding
// video pipeline to represent data packet and minimize data copying.
// It is particularly useful for splitting data between multiple RTP packets
// and assembling them into one buffer on the receiving side.
//
// CompoundBufferInputStream implements ZeroCopyInputStream interface
// to be used by protobuf to decode data stored in CompoundBuffer into
// a protocol buffer message.
//
// Mutations to the buffer are not thread-safe. Immutability can be ensured
// with the Lock() method.

#ifndef REMOTING_BASE_COMPOUND_BUFFER_H_
#define REMOTING_BASE_COMPOUND_BUFFER_H_

#include <stddef.h>
#include <stdint.h>

#include <deque>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "google/protobuf/io/zero_copy_stream.h"

namespace net {
class IOBuffer;
class IOBufferWithSize;
}  // namespace net

namespace remoting {

class CompoundBuffer {
 public:
  CompoundBuffer();
  ~CompoundBuffer();

  void Clear();

  // Adds new chunk to the buffer. |start| defines position of the chunk
  // within the |buffer|. |size| is the size of the chunk that is being
  // added, not size of the |buffer|.
  void Append(net::IOBuffer* buffer, int size);
  void Append(net::IOBuffer* buffer, const char* start, int size);
  void Append(const CompoundBuffer& buffer);
  void Prepend(net::IOBuffer* buffer, int size);
  void Prepend(net::IOBuffer* buffer, const char* start, int size);
  void Prepend(const CompoundBuffer& buffer);

  // Same as above, but creates new IOBuffer and copies the data.
  void AppendCopyOf(const char* data, int data_size);
  void PrependCopyOf(const char* data, int data_size);

  // Drop |bytes| bytes from the beginning or the end of the buffer.
  void CropFront(int bytes);
  void CropBack(int bytes);

  // Current size of the buffer.
  int total_bytes() const { return total_bytes_; }

  // Locks the buffer. After the buffer is locked, no data can be
  // added or removed (content can still be changed if some other
  // object holds reference to the IOBuffer objects).
  void Lock();

  // Returns true if content is locked.
  bool locked() const { return locked_; }

  // Creates new IOBufferWithSize object and copies all data into it.
  // Ownership of the result is given to the caller.
  net::IOBufferWithSize* ToIOBufferWithSize() const;

  // Copies all data into given location.
  void CopyTo(char* data, int data_size) const;

  // Clears the buffer, and initializes it with the interval from |buffer|
  // starting at |start| and ending at |end|. The data itself isn't copied.
  void CopyFrom(const CompoundBuffer& source, int start, int end);

 private:
  friend class CompoundBufferInputStream;

  struct DataChunk {
    DataChunk(net::IOBuffer* buffer, const char* start, int size);
    DataChunk(const DataChunk& other);
    ~DataChunk();

    scoped_refptr<net::IOBuffer> buffer;
    const char* start;
    int size;
  };
  typedef std::deque<DataChunk> DataChunkList;

  DataChunkList chunks_;
  int total_bytes_;
  bool locked_;

  DISALLOW_COPY_AND_ASSIGN(CompoundBuffer);
};

class CompoundBufferInputStream
    : public google::protobuf::io::ZeroCopyInputStream  {
 public:
  // Caller keeps ownership of |buffer|. |buffer| must be locked.
  explicit CompoundBufferInputStream(const CompoundBuffer* buffer);
  ~CompoundBufferInputStream() override;

  int position() const { return position_; }

  // google::protobuf::io::ZeroCopyInputStream interface.
  bool Next(const void** data, int* size) override;
  void BackUp(int count) override;
  bool Skip(int count) override;
  int64_t ByteCount() const override;

 private:
  const CompoundBuffer* buffer_;

  size_t current_chunk_;
  int current_chunk_position_;
  int position_;
  int last_returned_size_;
};

}  // namespace remoting

#endif  // REMOTING_BASE_COMPOUND_BUFFER_H_

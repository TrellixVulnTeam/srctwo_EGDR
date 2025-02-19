// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_LOADER_STREAM_WRITER_H_
#define CONTENT_BROWSER_LOADER_STREAM_WRITER_H_

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "content/browser/streams/stream_write_observer.h"

class GURL;

namespace net {
class IOBuffer;
}

namespace content {

class Stream;
class StreamRegistry;

// StreamWriter is a helper class for ResourceHandlers which route their output
// into a Stream. It manages an internal buffer and handles back-pressure from
// the Stream's reader.
class StreamWriter : public StreamWriteObserver {
 public:
  // Creates a new StreamWriter without an initialized Stream or controller. The
  // controller must be set before the writer is used.
  StreamWriter();
  ~StreamWriter() override;

  Stream* stream() { return stream_.get(); }

  // When immediate mode is enabled, the |stream_| is flushed every time new
  // data is made available by calls to OnReadCompleted.
  void set_immediate_mode(bool enabled) { immediate_mode_ = enabled; }

  // Initializes the writer with a new Stream in |registry|. |origin| will be
  // used to construct the URL for the Stream. See WebCore::BlobURL and and
  // WebCore::SecurityOrigin in Blink to understand how origin check is done on
  // resource loading. |cancel_callback| must be called if the StreamWriter
  // closes the stream.
  void InitializeStream(StreamRegistry* registry,
                        const GURL& origin,
                        const base::Closure& cancel_callback);

  // Prepares a buffer to read data from the request. This call will be followed
  // by either OnReadCompleted (on successful read or EOF) or destruction.  The
  // buffer may not be recycled until OnReadCompleted is called. If |min_size|
  // is not -1, it is the minimum size of the returned buffer.
  //
  // OnWillRead may be called before the stream is initialized. This is to
  // support MimeTypeResourceHandler which reads the initial chunk of data
  // early.
  void OnWillRead(scoped_refptr<net::IOBuffer>* buf,
                  int* buf_size,
                  int min_size);

  // A read was completed, forward the data to the Stream.
  // |need_more_data_callback| must be called (synchronously or asynchronously)
  // once the writer is ready for more data.  Invoking the callback may result
  // in more data being received recursively.
  //
  // InitializeStream must have been called before calling OnReadCompleted.
  void OnReadCompleted(int bytes_read,
                       const base::Closure& need_more_data_callback);

  // Called when there is no more data to read to the stream.
  void Finalize(int status);

 private:
  // StreamWriteObserver implementation.
  void OnSpaceAvailable(Stream* stream) override;
  void OnClose(Stream* stream) override;

  scoped_refptr<Stream> stream_;
  scoped_refptr<net::IOBuffer> read_buffer_;
  bool immediate_mode_;

  base::Closure cancel_callback_;
  base::Closure need_more_data_callback_;

  DISALLOW_COPY_AND_ASSIGN(StreamWriter);
};

}  // namespace content

#endif  // CONTENT_BROWSER_LOADER_STREAM_WRITER_H_

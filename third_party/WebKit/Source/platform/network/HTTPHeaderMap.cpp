/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "platform/network/HTTPHeaderMap.h"

#include <memory>
#include "platform/wtf/PtrUtil.h"

namespace blink {

HTTPHeaderMap::HTTPHeaderMap() {}

HTTPHeaderMap::~HTTPHeaderMap() {}

std::unique_ptr<CrossThreadHTTPHeaderMapData> HTTPHeaderMap::CopyData() const {
  std::unique_ptr<CrossThreadHTTPHeaderMapData> data =
      WTF::MakeUnique<CrossThreadHTTPHeaderMapData>();
  data->ReserveInitialCapacity(size());

  HTTPHeaderMap::const_iterator end_it = end();
  for (HTTPHeaderMap::const_iterator it = begin(); it != end_it; ++it)
    data->UncheckedAppend(std::make_pair(it->key.GetString().IsolatedCopy(),
                                         it->value.GetString().IsolatedCopy()));

  return data;
}

void HTTPHeaderMap::Adopt(std::unique_ptr<CrossThreadHTTPHeaderMapData> data) {
  Clear();
  size_t data_size = data->size();
  for (size_t index = 0; index < data_size; ++index) {
    std::pair<String, String>& header = (*data)[index];
    Set(AtomicString(header.first), AtomicString(header.second));
  }
}

}  // namespace blink

// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_MACHINE_INTELLIGENCE_RANKER_MODEL_H_
#define COMPONENTS_MACHINE_INTELLIGENCE_RANKER_MODEL_H_

#include <memory>
#include <string>

#include "base/macros.h"

namespace machine_intelligence {

class RankerModelProto;

class RankerModel {
 public:
  RankerModel();
  ~RankerModel();

  // Returns a new ranker model constructed from |data|.
  static std::unique_ptr<RankerModel> FromString(const std::string& data);

  const RankerModelProto& proto() const { return *proto_; }
  RankerModelProto* mutable_proto() const { return proto_.get(); }

  // Returns true if this ranker model has expired.
  bool IsExpired() const;

  const std::string& GetSourceURL() const;

  // Returns a serialization of this ranker model.
  std::string SerializeAsString() const;

 private:
  // The underlying ranker model proto.
  std::unique_ptr<RankerModelProto> proto_;

  DISALLOW_COPY_AND_ASSIGN(RankerModel);
};

}  // namespace machine_intelligence

#endif  // COMPONENTS_MACHINE_INTELLIGENCE_RANKER_MODEL_H_

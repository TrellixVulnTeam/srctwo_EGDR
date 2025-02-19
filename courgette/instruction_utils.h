// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COURGETTE_INSTRUCTION_UTILS_H_
#define COURGETTE_INSTRUCTION_UTILS_H_

#include <stdint.h>

#include "base/callback.h"
#include "base/macros.h"
#include "courgette/image_utils.h"
#include "courgette/memory_allocator.h"

namespace courgette {

// An interface to receive emitted instructions parsed from an executable.
class InstructionReceptor {
 public:
  InstructionReceptor() = default;
  virtual ~InstructionReceptor() = default;

  // Generates an entire base relocation table.
  virtual CheckBool EmitPeRelocs() = 0;

  // Generates an ELF style relocation table for X86.
  virtual CheckBool EmitElfRelocation() = 0;

  // Generates an ELF style relocation table for ARM.
  virtual CheckBool EmitElfARMRelocation() = 0;

  // Following instruction will be assembled at address 'rva'.
  virtual CheckBool EmitOrigin(RVA rva) = 0;

  // Generates a single byte of data or machine instruction.
  virtual CheckBool EmitSingleByte(uint8_t byte) = 0;

  // Generates multiple bytes of data or machine instructions.
  virtual CheckBool EmitMultipleBytes(const uint8_t* bytes, size_t len) = 0;

  // Generates a 4-byte relative reference to address of 'label'.
  virtual CheckBool EmitRel32(Label* label) = 0;

  // Generates a 4-byte relative reference to address of 'label' for ARM.
  virtual CheckBool EmitRel32ARM(uint16_t op,
                                 Label* label,
                                 const uint8_t* arm_op,
                                 uint16_t op_size) = 0;

  // Generates a 4-byte absolute reference to address of 'label'.
  virtual CheckBool EmitAbs32(Label* label) = 0;

  // Generates an 8-byte absolute reference to address of 'label'.
  virtual CheckBool EmitAbs64(Label* label) = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(InstructionReceptor);
};

// A rerunable callback that emit instructions to a provided receptor. Returns
// true on success, and false otherwise.
using InstructionGenerator = base::Callback<CheckBool(InstructionReceptor*)>;

// A counter that increments via .push_back(), so it can be passed via template
// to substitute std::vector<T>, to count elements instead of storing them.
template <typename T>
class CountingVector {
 public:
  CountingVector() {}

  void push_back(const T& /* unused */) { ++size_; }
  size_t size() const { return size_; }

 private:
  size_t size_ = 0;
};

}  // namespace courgette

#endif  // COURGETTE_INSTRUCTION_UTILS_H_

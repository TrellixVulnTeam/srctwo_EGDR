// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>

#include "courgette/assembly_program.h"

#include "base/logging.h"
#include "courgette/encoded_program.h"
#include "courgette/instruction_utils.h"

namespace courgette {

namespace {

// An instruction receptor that adds each received abs32/rel32 Label* to the
// matching VECTOR member variable. Template VECTOR allows code reuse for
// counting (CountingVector) and storage (std::vector).
template <template <typename T, typename... Args> class CONTAINER>
class LabelReceptor : public InstructionReceptor {
 public:
  using VECTOR = CONTAINER<Label*>;

  LabelReceptor() = default;
  ~LabelReceptor() override = default;

  VECTOR* mutable_abs32_vector() { return &abs32_vector_; }
  VECTOR* mutable_rel32_vector() { return &rel32_vector_; }

  // InstructionReceptor:
  CheckBool EmitPeRelocs() override { return true; }
  CheckBool EmitElfRelocation() override { return true; }
  CheckBool EmitElfARMRelocation() override { return true; }
  CheckBool EmitOrigin(RVA rva) override { return true; }
  CheckBool EmitSingleByte(uint8_t byte) override { return true; }
  CheckBool EmitMultipleBytes(const uint8_t* bytes, size_t len) override {
    return true;
  }
  CheckBool EmitRel32(Label* label) override {
    rel32_vector_.push_back(label);
    return true;
  }
  CheckBool EmitRel32ARM(uint16_t op,
                         Label* label,
                         const uint8_t* arm_op,
                         uint16_t op_size) override {
    rel32_vector_.push_back(label);
    return true;
  }
  CheckBool EmitAbs32(Label* label) override {
    abs32_vector_.push_back(label);
    return true;
  }
  CheckBool EmitAbs64(Label* label) override {
    abs32_vector_.push_back(label);
    return true;
  }

 private:
  VECTOR abs32_vector_;
  VECTOR rel32_vector_;

  DISALLOW_COPY_AND_ASSIGN(LabelReceptor);
};

}  // namespace

AssemblyProgram::AssemblyProgram(ExecutableType kind, uint64_t image_base)
    : kind_(kind), image_base_(image_base) {}

AssemblyProgram::~AssemblyProgram() = default;

void AssemblyProgram::PrecomputeLabels(RvaVisitor* abs32_visitor,
                                       RvaVisitor* rel32_visitor) {
  abs32_label_manager_.Read(abs32_visitor);
  rel32_label_manager_.Read(rel32_visitor);
  TrimLabels();
}

// Chosen empirically to give the best reduction in payload size for
// an update from daisy_3701.98.0 to daisy_4206.0.0.
const int AssemblyProgram::kLabelLowerLimit = 5;

void AssemblyProgram::TrimLabels() {
  // For now only trim for ARM binaries.
  if (kind() != EXE_ELF_32_ARM)
    return;

  int lower_limit = kLabelLowerLimit;

  VLOG(1) << "TrimLabels: threshold " << lower_limit;

  rel32_label_manager_.RemoveUnderusedLabels(lower_limit);
}

void AssemblyProgram::UnassignIndexes() {
  abs32_label_manager_.UnassignIndexes();
  rel32_label_manager_.UnassignIndexes();
}

void AssemblyProgram::DefaultAssignIndexes() {
  abs32_label_manager_.DefaultAssignIndexes();
  rel32_label_manager_.DefaultAssignIndexes();
}

void AssemblyProgram::AssignRemainingIndexes() {
  abs32_label_manager_.AssignRemainingIndexes();
  rel32_label_manager_.AssignRemainingIndexes();
}

Label* AssemblyProgram::FindAbs32Label(RVA rva) {
  return abs32_label_manager_.Find(rva);
}

Label* AssemblyProgram::FindRel32Label(RVA rva) {
  return rel32_label_manager_.Find(rva);
}

CheckBool AssemblyProgram::AnnotateLabels(const InstructionGenerator& gen) {
  // Pass 1: Compute required space.
  LabelReceptor<CountingVector> count_receptor;
  if (!gen.Run(&count_receptor))
    return false;

  // Pass 2: Reserve and store annotations.
  LabelReceptor<std::vector> annotate_receptor;
  annotate_receptor.mutable_abs32_vector()->reserve(
      count_receptor.mutable_abs32_vector()->size());
  annotate_receptor.mutable_rel32_vector()->reserve(
      count_receptor.mutable_rel32_vector()->size());
  if (!gen.Run(&annotate_receptor))
    return false;

  // Move results to |abs32_label_annotations_| and |re32_label_annotations_|.
  abs32_label_annotations_.swap(*annotate_receptor.mutable_abs32_vector());
  rel32_label_annotations_.swap(*annotate_receptor.mutable_rel32_vector());
  return true;
}

bool AssemblyProgram::PrepareEncodedProgram(EncodedProgram* encoded) const {
  encoded->set_image_base(image_base_);
  return encoded->ImportLabels(abs32_label_manager_, rel32_label_manager_);
}

}  // namespace courgette

/* Copyright 2023 Tencent Inc.  All rights reserved.

==============================================================================*/
#pragma once

#include "numerous_llm/layers/base_layer.h"

namespace numerous_llm {

class AttentionLayer : public BaseLayer {
 public:
  virtual Status Init(const std::vector<std::any>& parameters, std::shared_ptr<Context> context, int rank) override;

  virtual Status Forward(const std::vector<Tensor>& input_tensors, std::vector<Tensor>& output_tensors) = 0;

 protected:
  int layer_index_;
  int max_position_embeddings_;
  int block_size_;
  int num_heads_;
  int head_size_;
  bool is_causal_{true};
};

}  // namespace numerous_llm

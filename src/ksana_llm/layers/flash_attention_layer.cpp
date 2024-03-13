/* Copyright 2023 Tencent Inc.  All rights reserved.

==============================================================================*/

#include "ksana_llm/layers/flash_attention_layer.h"
#include "ksana_llm/kernels/nvidia/kernel_wrapper.h"

namespace ksana_llm {

Status FlashAttentionLayer::Forward(const std::vector<Tensor>& input_tensors, std::vector<Tensor>& output_tensors) {
  // input_tensors:
  //     0: qkv_tensor shape [max_token_num, hidden_units, 3], type same as weight
  //     1: input offset tensor shape [max_batch_size + 1], type uint64
  //     2: kv_list shape [num_layer, max_block_num, 2], type pointer
  //     3: kv_cache_offset_tensor shape [max_batch_size + 1], type int32
  //     4: rotary embedding pos tensor shape [max_token_num], type int64
  //     5: forward shape: [batch_size, max_tokens, kv_cache_offset_list.back()]
  // output_tensors:
  //     0: flash_attention_output shape: [std::max(max_batch_size * vocab_size, max_token_num * hidden_units * 3)]

  int max_tokens = input_tensors[5].shape[1];
  int batch_size = input_tensors[5].shape[0];
  int layer_block_num = input_tensors[5].shape[2];
  int total_tokens = input_tensors[0].shape[0];

  void** k_list = (input_tensors[2].GetPtr<void*>()) + layer_index_ * layer_block_num * 2;
  void** v_list = k_list + layer_block_num;
  AttenVarlen(input_tensors[0].GetPtr<void>(), input_tensors[4].GetPtr<void>(), output_tensors[0].GetPtr<void>(),
              input_tensors[1].GetPtr<void>(), rotary_embedding_cuda_, total_tokens, max_tokens, batch_size, num_heads_,
              head_size_, stride_size_, is_causal_, rank_, block_token_num_, k_list, v_list,
              input_tensors[3].GetPtr<void>(), context_->GetComputeStreams()[rank_]);
  output_tensors[0].shape[0] = input_tensors[0].shape[0];
  output_tensors[0].shape[1] = input_tensors[0].shape[1] / 3;
  output_tensors[0].dtype = input_tensors[0].dtype;
  return Status();
}

}  // namespace ksana_llm

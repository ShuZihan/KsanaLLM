/* Copyright 2024 Tencent Inc.  All rights reserved.

==============================================================================*/
#pragma once

#include <memory>
#include "ksana_llm/models/common/common_weight.h"

namespace ksana_llm {

template <typename T>
class GPTWeight : public CommonWeight<T> {
 public:
  GPTWeight() {}
  explicit GPTWeight(const ModelConfig& model_config, int rank, std::shared_ptr<Context> context);

  void ProcessWeights() override;

 protected:
  using CommonWeight<T>::context_;
  using CommonWeight<T>::rank_;

  using CommonWeight<T>::weights_map_;
  using CommonWeight<T>::weights_data_type_map_;

  using CommonWeight<T>::model_config_;

  using CommonWeight<T>::tensor_manager_;
};

}  // namespace ksana_llm

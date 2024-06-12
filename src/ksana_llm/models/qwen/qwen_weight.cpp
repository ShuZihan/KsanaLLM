/* Copyright 2024 Tencent Inc.  All rights reserved.

==============================================================================*/
#include "ksana_llm/models/qwen/qwen_weight.h"

namespace ksana_llm {

template <typename T>
QwenWeight<T>::QwenWeight(const ModelConfig& model_config, int rank, std::shared_ptr<Context> context) {
  common_weight_ = std::make_shared<CommonWeight<T>>(model_config, rank, context);
}

template <typename T>
Tensor QwenWeight<T>::GetModelWeights(const std::string& weight_name) {
  return common_weight_->GetModelWeights(weight_name);
}

template <typename T>
Status QwenWeight<T>::LoadWeightsFromFile(std::shared_ptr<BaseFileTensorLoader>& weights_loader) {
  if (!common_weight_->LoadWeightsFromFile(weights_loader).OK()) {
    NLLM_LOG_ERROR << fmt::format("Load weight file error.");
    exit(-1);
  }
  return Status();
}

template <typename T>
void QwenWeight<T>::ProcessWeights() {
  common_weight_->ProcessWeights();
}

template class QwenWeight<float>;
template class QwenWeight<float16>;
#ifdef ENABLE_BFLOAT16
template class QwenWeight<bfloat16>;
#endif

}  // namespace ksana_llm

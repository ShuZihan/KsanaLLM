/*
 * Copyright 2024 Tencent Inc.  All rights reserved.
 */

#include <gtest/gtest.h>

#include "csrc/utils/nvidia/cuda_utils.h"
#include "tests/kernels/nvidia/utils/testsuit_base.h"

#include "csrc/kernels/nvidia/layernorm/layernorm.h"

namespace llm_kernels {
namespace nvidia {
namespace test {

class LlamaNvidiaLayernormTestSuit : public NvidiaTestSuitBase {
 public:
  void SetUp() override { NvidiaTestSuitBase::SetUp(); }

  void TearDown() override { NvidiaTestSuitBase::TearDown(); }

 protected:
  using NvidiaTestSuitBase::stream;
  const std::vector<std::pair<int, int>> m_n_pairs = {{2, 4096}};

  template <typename T>
  void RunLayerNormRef(const float variance_epsilon = 1e-6f) {
    std::string type_str = "float";
    if (std::is_same<T, half>::value) {
      type_str = "half";
    } else if (std::is_same<T, __nv_bfloat16>::value) {
      type_str = "bfloat16";
    }

    std::stringstream ss;
    ss << "python layernorm_test.py --type=" << type_str << " --variance_epsilon=" << std::to_string(variance_epsilon);
    system(ss.str().c_str());
  }

  template <typename T>
  void TestLayerNorm(const size_t m, const size_t n) {
    std::string type_str = "float";
    if (std::is_same<T, half>::value) {
      type_str = "half";
    } else if (std::is_same<T, __nv_bfloat16>::value) {
      type_str = "bfloat16";
    }

    BufferMeta input_meta = CreateBuffer<T>(MemoryType::MEMORY_GPU, {m, n},
                                            /*is_random_init*/ true);
    BufferMeta weight_meta = CreateBuffer<T>(MemoryType::MEMORY_GPU, {n},
                                             /*is_random_init*/ true);
    BufferMeta bias_meta = CreateBuffer<T>(MemoryType::MEMORY_GPU, {n},
                                           /*is_random_init*/ true);
    BufferMeta layernorm_output_meta = CreateBuffer<T>(MemoryType::MEMORY_GPU, {m, n},
                                                       /*is_random_init*/ false);
    BufferMeta rmsnorm_output_meta = CreateBuffer<T>(MemoryType::MEMORY_GPU, {m, n},
                                                     /*is_random_init*/ false);
    BufferMeta layernorm_output_ref_meta = CreateBuffer<T>(MemoryType::MEMORY_GPU, {m, n},
                                                           /*is_random_init*/ false);
    BufferMeta rmsnorm_output_ref_meta = CreateBuffer<T>(MemoryType::MEMORY_GPU, {m, n},
                                                         /*is_random_init*/ false);
    float layernorm_eps = 1e-6;

    input_meta.SaveNpy<T>("layernorm_test_input.npy");
    weight_meta.SaveNpy<T>("layernorm_test_weight.npy");
    bias_meta.SaveNpy<T>("layernorm_test_bias.npy");
    RunLayerNormRef<T>(layernorm_eps);
    LoadNpy<T>("layernorm_test_output.npy", MemoryType::MEMORY_GPU, layernorm_output_ref_meta);
    LoadNpy<T>("rmsnorm_test_output.npy", MemoryType::MEMORY_GPU, rmsnorm_output_ref_meta);
    CHECK_NVIDIA_CUDA_ERROR(cudaStreamSynchronize(stream));
    CHECK_NVIDIA_CUDA_ERROR(cudaDeviceSynchronize());

    InvokeLayerNorm<T>(reinterpret_cast<T*>(layernorm_output_meta.data_ptr),
                       reinterpret_cast<const T*>(input_meta.data_ptr),
                       reinterpret_cast<const T*>(weight_meta.data_ptr), reinterpret_cast<const T*>(bias_meta.data_ptr),
                       layernorm_eps, m, n, stream);
    // compute rmsnorm if bias is none
    InvokeLayerNorm<T>(reinterpret_cast<T*>(rmsnorm_output_meta.data_ptr),
                       reinterpret_cast<const T*>(input_meta.data_ptr),
                       reinterpret_cast<const T*>(weight_meta.data_ptr),
                       /* bias */ nullptr, layernorm_eps, m, n, stream);
    CHECK_NVIDIA_CUDA_ERROR(cudaStreamSynchronize(stream));
    CHECK_NVIDIA_CUDA_ERROR(cudaDeviceSynchronize());

    EXPECT_TRUE(CheckResult<T>("layernorm_" + type_str + "_m_" + std::to_string(m) + "_n_" + std::to_string(n),
                               layernorm_output_ref_meta, layernorm_output_meta, 1e-3f, 1e-3f));
    EXPECT_TRUE(CheckResult<T>("rmsnorm_" + type_str + "_m_" + std::to_string(m) + "_n_" + std::to_string(n),
                               rmsnorm_output_ref_meta, rmsnorm_output_meta, 1e-3f, 1e-3f));

    DeleteBuffer(rmsnorm_output_ref_meta);
    DeleteBuffer(layernorm_output_ref_meta);
    DeleteBuffer(rmsnorm_output_meta);
    DeleteBuffer(layernorm_output_meta);
    DeleteBuffer(bias_meta);
    DeleteBuffer(weight_meta);
    DeleteBuffer(input_meta);
  }
};

TEST_F(LlamaNvidiaLayernormTestSuit, HalfLayernormCommonTest) {
  for (const auto& m_n_pair : m_n_pairs) {
    TestLayerNorm<half>(static_cast<size_t>(m_n_pair.first), static_cast<size_t>(m_n_pair.second));
  }
}

TEST_F(LlamaNvidiaLayernormTestSuit, FloatLayernormCommonTest) {
  for (const auto& m_n_pair : m_n_pairs) {
    TestLayerNorm<float>(static_cast<size_t>(m_n_pair.first), static_cast<size_t>(m_n_pair.second));
  }
}

}  // namespace test
}  // namespace nvidia
}  // namespace llm_kernels

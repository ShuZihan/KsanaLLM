/* Copyright 2023 Tencent Inc.  All rights reserved.

==============================================================================*/
#pragma once

namespace numerous_llm {

// The memory device.
enum MemoryDevice {
  // CPU
  MEMORY_CPU,

  // CPU with pinned memory.
  MEMORY_CPU_PINNED,

  // NVIDIA GPU
  MEMORY_GPU,

  // HUAWEI Ascend
  MEMORY_ASCEND
};

// The memory block information.
struct MemoryBlock {
  // block id, unique in global.
  int block_id;

  // block size, in bytes.
  int block_size;

  // The reference count of current block.
  int ref_count = 0;

  // /The device of this block, CPU or GPU or NPU.
  MemoryDevice device;

  // The physical address of this block.
  void *address = nullptr;
};

}  // namespace numerous_llm

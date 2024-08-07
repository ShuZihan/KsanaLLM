/* Copyright 2024 Tencent Inc.  All rights reserved.

==============================================================================*/
#pragma once

#include "ksana_llm/cache_manager/base_cache_manager.h"
#include "ksana_llm/cache_manager/cache_manager_interface.h"

#include "ksana_llm/runtime/request_state.h"
#include "ksana_llm/runtime/threadpool.h"
#include "ksana_llm/utils/environment.h"

namespace ksana_llm {

struct DirectCachedRequest;

// Describe a cached data, on either device or host.
struct DirectCachedBlock {
  // The unique id of this block, independent with content, not changed after created.
  size_t block_id = 0;

  // Whether the block is on device or host.
  bool is_device_location = true;

  // The block id of every device, the index is device_id。
  // If this cached block is swaped out, the value is host block id of every device.
  std::vector<int> memory_block_ids;
};

// Describe the cache information for a infer request.
struct DirectCachedRequest {
  // The id of this request, as same as the id of InferRequest.
  int64_t req_id;

  // include input tokens, used to get next step block number.
  size_t output_token_num = 0;

  // The cached blocks associated with this request, not include root node.
  // Contain all blocks even when request is waiting, or blocks that haved been swapped.
  std::vector<DirectCachedBlock*> cached_blocks;
};

// Used to support prefix caching.
class DirectCacheManager : public CacheManagerInterface,
                           public BaseCacheManager<DirectCachedBlock, DirectCachedRequest> {
 public:
  explicit DirectCacheManager(const CacheManagerConfig& cache_manager_config);
  ~DirectCacheManager();

  // Initialize all the memory blocks.
  void InitializeCachedBlocks();

  // Get block number that not usable now, but will be usable in future.
  // That is, the blocks used by swapout, but not merged yet.
  size_t GetFutureBlockNumber();

  // Get all usable block number, including free and reusable ones.
  size_t GetUsableBlockNumber();

  // The value is from block mamanger.
  size_t GetHostFreeBlockNumber();

  // Get the needed block num for specific request.
  size_t GetRequestStepBlockNumber(int64_t req_id);

  // Get the usable block num for specific request.
  size_t GetRequestUsableBlockNumber(int64_t req_id);

  // Check the block num of specific request, the token number must be enough for next generation.
  // The shared block num always 0.
  Status GetRequestPrefixBlockNumber(int64_t req_id, const std::vector<int>& input_token_ids, size_t& shared_block_num,
                                     size_t& unique_block_num, size_t& shared_token_num);

  // Allocate new blocks for request, called only when req is running.
  Status AllocateRequestBlocks(int64_t req_id, size_t block_num, std::vector<std::vector<int>>& req_block_ids);

  // Update the token ids of this request.
  Status UpdateRequestTokens(int64_t req_id, const std::vector<int>& token_ids,
                             std::vector<std::vector<int>>& req_block_ids);

  // Get the freeable/needed block num if swap out/in a request.
  Status GetRequestFreeableBlockNum(int64_t req_id, size_t& block_num);
  Status GetRequestNeededBlockNum(int64_t req_id, size_t& block_num);

  // Swap out/in specific request async.
  Status SwapoutRequestAsync(int64_t req_id, size_t& swapped_block_num, size_t& free_block_num);
  Status SwapinRequestAsync(int64_t req_id, size_t& block_num, std::vector<std::vector<int>>& req_block_ids);

  // Waiting until at lease on swap out/in task done, return the pending task number.
  Status WaitSwapoutRequests(std::vector<int64_t>& req_ids, size_t& left_req_num, bool blocking = true);
  Status WaitSwapinRequests(std::vector<int64_t>& req_ids, size_t& left_req_num, bool blocking = true);

  // Merge the swapped out blocks to free list, no need to get host block ids.
  // The swapout of the request's block must be done before call this.
  Status MergeSwapoutRequest(int64_t req_id);

  // Merge the swapped in block to the tree, update block ids for infer request.
  // The swapin of the request's block must be done before call this.
  Status MergeSwapinRequest(int64_t req_id, std::vector<std::vector<int>>& req_block_ids);

  // Drop a swaped cached request.
  void DestroySwapedRequest(int64_t req_id);

  // Update internal state after request finished.
  void DestroyFinishedRequest(int64_t req_id);

 private:
  // Create a new cached block.
  DirectCachedBlock* CreateCachedBlock(size_t block_id);

  // Create a temporarily cached block, no block id generated.
  DirectCachedBlock* CreateEmptyCachedBlock();
};

}  // namespace ksana_llm
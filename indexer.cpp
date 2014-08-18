#include "indexer.h"
#include <algorithm>

namespace utility {

const unsigned long kMaxIndexNumber = 0xFFFFFFFF;

Indexer::Indexer() {
  index_count_ = 0;
}

Indexer::~Indexer(){
}

Index Indexer::CreateIndex() {
  std::lock_guard<std::mutex> lock(index_pool_lock);
  if (index_pool_.size() == kMaxIndexNumber) {
    return kInvalidIndex;
  }
  unsigned long ret_index = 0;
  if (index_count_ != kMaxIndexNumber) {
    ++index_count_;
    ret_index = index_count_;
  } else {
    unsigned long min_index = 1;
    auto find_index = index_pool_.find(min_index);
    while (find_index != index_pool_.end()) {
      ++min_index;
      find_index = index_pool_.find(min_index);
    }
    ret_index = min_index;
  }
  index_pool_.insert(ret_index);
  return ret_index;
}

void Indexer::DestroyIndex(Index index) {
  std::lock_guard<std::mutex> lock(index_pool_lock);
  index_pool_.erase(index);
}

void Indexer::DestroyIndex(const std::vector<Index>& multi_index) {
  std::lock_guard<std::mutex> lock(index_pool_lock);
  std::for_each(multi_index.begin(), multi_index.end(), [this](Index index) { index_pool_.erase(index); });
}

void Indexer::Clear() {
  std::lock_guard<std::mutex> lock(index_pool_lock);
  index_pool_.clear();
  index_count_ = 0;
}

} // namespace utility
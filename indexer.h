/************************************************************************/
/*  Indexer                                                             */
/*  THREAD: safe                                                        */
/*  AUTHOR: chen_lu@outlook.com                                         */
/************************************************************************/

#ifndef UTILITY_INDEXER_H_
#define UTILITY_INDEXER_H_

#include "uncopyable.h"
#include <mutex>
#include <unordered_set>
#include <vector>

namespace utility {

typedef unsigned long Index;
const Index kInvalidIndex = 0;

class Indexer : public Uncopyable {
 public:
  Indexer();
  ~Indexer();

  Index CreateIndex();
  void DestroyIndex(Index index);
  void DestroyIndex(const std::vector<Index>& multi_index);
  void Clear();

 private:
  unsigned long index_count_;
  std::unordered_set<Index> index_pool_;
  std::mutex index_pool_lock;
};

} // namespace utility

#endif // UTILITY_INDEXER_H_
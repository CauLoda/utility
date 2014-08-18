/************************************************************************/
/*  Message Queue                                                       */
/*  THREAD: safe                                                        */
/*  AUTHOR: chen_lu@outlook.com                                         */
/************************************************************************/

#ifndef UTILITY_MESSAGE_QUEUE_H_
#define UTILITY_MESSAGE_QUEUE_H_

#include "timer.h"
#include "indexer.h"
#include <functional>
#include <memory>
#include <thread>
#include <unordered_map>
#include <time.h>

namespace utility {

class MessageQueue : public Uncopyable {
 public:
  MessageQueue();
  ~MessageQueue();

  bool Init(int timeout);
  bool Push(std::function<bool (Index, bool)>&& sender);
  void Pop(Index index);
  void Pop(const std::vector<Index>& batch_index);
  void Uninit();

 private:
  bool CheckTimeout();

 private:
  class MessageResender {
   public:
    MessageResender(std::function<bool (Index)>&& resender) : resender_(std::move(resender)) {}
    const time_t& resend_time() const { return resend_time_; }
    void set_resend_time(const time_t& resend_time) { resend_time_ = resend_time; }
    const std::function<bool (Index)>& resender() const { return resender_; }

   private:
    time_t resend_time_;
    std::function<bool (Index)> resender_;
  };

 private:
  Timer timer_;
  int timeout_;
  Indexer queue_indexer_;
  std::unordered_map<Index, std::unique_ptr<MessageResender>> task_queue_;
  std::mutex task_queue_lock_;
  std::unique_ptr<std::thread> check_thread_;
};

} // namespace utility

#endif // UTILITY_MESSAGE_QUEUE_H_
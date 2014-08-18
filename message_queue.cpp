#include "message_queue.h"
#include "log.h"
#include <algorithm>

namespace utility {

MessageQueue::MessageQueue() {
  timeout_ = 0;
}

MessageQueue::~MessageQueue() {
  Uninit();
}

bool MessageQueue::Init(int timeout) {
  if (timeout <= 0) {
    return false;
  }
  if (!timer_.Init(timeout)) {
    return false;
  }
  timeout_ = timeout;
  auto thread_proc = std::bind(&MessageQueue::CheckTimeout, this);
  check_thread_.reset(new std::thread(thread_proc));
  return true;
}

bool MessageQueue::Push(std::function<bool (Index, bool)>&& sender) {
  auto index = queue_indexer_.CreateIndex();
  if (index == kInvalidIndex) {
    LOG(kError, "no useful message queue index.");
    return false;
  }
  auto resender = std::bind(sender, std::placeholders::_1, true);
  std::unique_ptr<MessageResender> new_msg_resender(new MessageResender(std::move(resender)));
  new_msg_resender->set_resend_time(time(nullptr) + timeout_);
  task_queue_lock_.lock();
  task_queue_.insert(std::make_pair(index, std::move(new_msg_resender)));
  task_queue_lock_.unlock();
  if (!sender(index, false)) {
    Pop(index);
    return false;
  }
  return true;
}

void MessageQueue::Pop(Index index) {
  task_queue_lock_.lock();
  auto find_resender = task_queue_.find(index);
  if (find_resender != task_queue_.end()) {
    task_queue_.erase(find_resender);
    task_queue_lock_.unlock();
    queue_indexer_.DestroyIndex(index);
  } else {
    task_queue_lock_.unlock();
  }
}

void MessageQueue::Pop(const std::vector<Index>& batch_index) {
  task_queue_lock_.lock();
  std::for_each(batch_index.begin(), batch_index.end(), [this](Index index) { task_queue_.erase(index); });
  task_queue_lock_.unlock();
  queue_indexer_.DestroyIndex(batch_index);
}

void MessageQueue::Uninit() {
  timer_.Uninit();
  if (check_thread_ != nullptr) {
    check_thread_->join();
    check_thread_ = nullptr;
  }
  task_queue_lock_.lock();
  task_queue_.clear();
  task_queue_lock_.unlock();
  queue_indexer_.Clear();
  timeout_ = 0;
}

bool MessageQueue::CheckTimeout() {
  while (true) {
    if (!timer_.Wait()) {
      LOG(kError, "fail to wait message queue timer.");
      return false;
    }
    auto next_timeout = timeout_;
    std::vector<Index> to_destroy_index;
    task_queue_lock_.lock();
    auto now_time = time(nullptr);
    auto current_task = task_queue_.begin();
    while (current_task != task_queue_.end()) {
      const auto& index = current_task->first;
      auto& resender = current_task->second;
      auto current_task_timeout = resender->resend_time() - now_time;
      if (current_task_timeout > 0) {
        if (current_task_timeout < next_timeout) {
          next_timeout = static_cast<int>(current_task_timeout);
        }
        ++current_task;
        continue;
      }
      if (resender->resender()(index)) {
        resender->set_resend_time(now_time + timeout_);
        ++current_task;
      } else {
        current_task = task_queue_.erase(current_task);
        to_destroy_index.push_back(index);
      }
    }
    task_queue_lock_.unlock();
    timer_.ResetTimer(next_timeout);
    queue_indexer_.DestroyIndex(to_destroy_index);
  }
  return true;
}

} // namespace utility
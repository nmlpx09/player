#include "context.h"

bool TContext::IsQueueEmpty() noexcept {
    return Queue.empty();
}

void TContext::ReadWait() {
    std::unique_lock<std::recursive_mutex> ulock{Mutex};
    ReadCv.wait(ulock, [this] { return IsQueueEmpty() || IsStop(); });
}

void TContext::WriteWait() {
    std::unique_lock<std::recursive_mutex> ulock{Mutex};
    WriteCv.wait(ulock, [this] { return !IsQueueEmpty() || IsStop(); });
}

void TContext::ReadNotify() noexcept {
    ReadCv.notify_one();
}

void TContext::WriteNotify() noexcept {
    WriteCv.notify_one();
}

void TContext::Stop() noexcept {
    std::unique_lock<std::recursive_mutex> ulock{Mutex};
    if (!StopFlag) {
        StopFlag = true;
        ReadCv.notify_one();
        WriteCv.notify_one();
    }
}

bool TContext::IsStop() {
    std::unique_lock<std::recursive_mutex> ulock{Mutex};
    return StopFlag;
}

void TContext::StorePayload(TPayload&& payload) {
    std::unique_lock<std::recursive_mutex> ulock{Mutex};
    Queue.emplace_back(std::move(payload));
}

TPayload TContext::GetPayload() {
    std::unique_lock<std::recursive_mutex> ulock{Mutex};
    auto payload = std::move(Queue.front());
    Queue.pop_front();
    return payload;
}

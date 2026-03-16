#include "context.h"

bool TContext::IsQueueEmpty() {
    return Queue.empty();
}

void TContext::ReadWait() {
    std::unique_lock<std::mutex> ulock{Mutex};
    ReadCv.wait(ulock, [this] { return IsQueueEmpty() || IsStop(); });
}

void TContext::WriteWait() {
    std::unique_lock<std::mutex> ulock{Mutex};
    WriteCv.wait(ulock, [this] { return !IsQueueEmpty() || IsStop(); });
}

void TContext::ReadNotify() {
    ReadCv.notify_one();
}

void TContext::WriteNotify() {
    WriteCv.notify_one();
}

void TContext::Stop() {
    if (!StopFlag.exchange(true)) {
        Queue.clear();
        ReadCv.notify_one();
        WriteCv.notify_one();
    }
}

bool TContext::IsStop() {
    return StopFlag.load();
}

void TContext::StorePayload(TPayload&& payload) {
    std::unique_lock<std::mutex> ulock{Mutex};
    Queue.emplace_back(std::move(payload));
}

TPayload TContext::GetPayload() {
    std::unique_lock<std::mutex> ulock{Mutex};
    auto payload = Queue.front();
    Queue.pop_front();
    return payload;
}

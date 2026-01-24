#include "context.h"

void TContext::Stop() {
    std::unique_lock<std::mutex> ulock{Mutex};
    End = true;
    Queue.clear();
    ulock.unlock();
    WriteCv.notify_one();
    ReadCv.notify_one();
}

void TContext::Start() {
    std::unique_lock<std::mutex> ulock{Mutex};
    End = false;
}

bool TContext::IsEnd() {
    return End;
}

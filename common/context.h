#pragma once

#include "types.h"

#include <atomic>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>

struct TContext {
public:
    bool IsStop();
    void Stop();

    void StorePayload(TPayload&& payload);
    TPayload GetPayload();

    void ReadWait();
    void WriteWait();
    void ReadNotify();
    void WriteNotify();
private:
    bool IsQueueEmpty();
private:
    std::mutex Mutex;
    std::condition_variable WriteCv;
    std::condition_variable ReadCv;
    std::list<TPayload> Queue;
    std::atomic_bool StopFlag = false;
};

using TContextPtr = std::shared_ptr<TContext>;

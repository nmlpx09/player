#pragma once

#include "types.h"

#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>

struct TContext {
public:
    bool IsStop();
    bool IsQuit();
    bool IsQueueEmpty();

    void Stop();
    void Start();

    void StorePayload(TPayload&& payload);
    TPayload GetPayload();

    void ReadWait();
    void WriteWait();
    void ReadNotify();
    void WriteNotify();
private:
    std::mutex Mutex;
    std::condition_variable WriteCv;
    std::condition_variable ReadCv;
    std::list<TPayload> Queue;
    bool StopFlag = true;
};

using TContextPtr = std::shared_ptr<TContext>;

#pragma once

#include "types.h"

#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>

struct TContext {
public:
    bool IsStop();
    void Stop() noexcept;

    void StorePayload(TPayload&& payload);
    TPayload GetPayload();

    void ReadWait();
    void WriteWait();
    void ReadNotify() noexcept;
    void WriteNotify() noexcept;
    bool IsQueueEmpty();
private:
    std::recursive_mutex Mutex;
    std::condition_variable_any WriteCv;
    std::condition_variable_any ReadCv;
    std::list<TPayload> Queue;
    bool StopFlag = false;
};

using TContextPtr = std::shared_ptr<TContext>;

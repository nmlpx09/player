#pragma once

#include "types.h"

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <list>
#include <memory>
#include <mutex>

struct TContext {
public:
    void Stop();
    void Start();
    bool IsEnd();
public:
    std::list<std::tuple<std::chrono::time_point<std::chrono::steady_clock>, TFormat, std::vector<std::uint8_t>>> Queue;
    std::mutex Mutex;
    std::condition_variable WriteCv;
    std::condition_variable ReadCv;
private:
    bool End = true;
};

using TContextPtr = std::shared_ptr<TContext>;

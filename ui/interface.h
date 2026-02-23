#pragma once

#include "types.h"

#include <memory>

namespace NUI {

struct TInterface {
public:
    virtual void ListDraw(const TFiles&, std::size_t) noexcept = 0;
    virtual void StatusDraw(const std::string&) noexcept = 0;
    virtual void StatusClean() noexcept = 0;
    virtual void Init() noexcept = 0;
    virtual void Close() noexcept = 0;
    virtual ECommands GetCommand() noexcept = 0;
    virtual ~TInterface() = default;
};

using TUIPtr = std::shared_ptr<TInterface>;

}

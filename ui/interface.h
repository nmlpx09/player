#pragma once

#include "types.h"

#include <expected>
#include <memory>
#include <system_error>

namespace NUI {

struct TInterface {
public:
    virtual void ListDraw(const TFiles&, std::size_t) noexcept = 0;
    virtual void StatusDraw(const std::string&) noexcept = 0;
    virtual void StatusClean() noexcept = 0;
    virtual std::error_code Init() noexcept = 0;
    virtual std::expected<ECommands, std::error_code> GetCommand() noexcept = 0;
    virtual ~TInterface() = default;
};

using TUIPtr = std::shared_ptr<TInterface>;

}

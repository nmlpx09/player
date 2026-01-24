#pragma once

#include <common/types.h>

#include <cstdint>
#include <expected>
#include <functional>
#include <memory>
#include <string>
#include <system_error>

namespace NRead {

using TCallback = std::function<void(TData)>;

struct TInterface {
public:
    virtual std::expected<TFormat, std::error_code> Init(std::string) noexcept = 0;
    virtual std::expected<bool, std::error_code> Read(const TCallback& callback) noexcept = 0;
    virtual ~TInterface() {};
};

using TReadPtr = std::unique_ptr<TInterface>;

}

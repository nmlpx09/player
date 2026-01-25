#pragma once

#include <filesystem>
#include <memory>
#include <vector>

namespace NUI {

struct TInterface {
public:
    virtual void ListDraw(const std::vector<std::filesystem::path>&, std::size_t) noexcept = 0;
    virtual void StatusDraw(const std::string&) noexcept = 0;
    virtual void Init() noexcept = 0;
    virtual void Close() noexcept = 0;
    virtual ~TInterface() = default;
};

using TUIPtr = std::shared_ptr<TInterface>;

}

#pragma once

#include <write/interface.h>

#include <pulse/simple.h>

#include <common/types.h>

#include <string>

namespace NWrite {

struct TWrite: TInterface {
public:
    TWrite(std::string device) noexcept;
    TWrite(const TWrite&) = delete;
    TWrite(TWrite&&) noexcept = default;
    ~TWrite() noexcept;
    TWrite& operator=(const TWrite&) = delete;
    TWrite& operator=(TWrite&&) = delete;

    std::error_code Write(const TCallback& callback) override;
    std::error_code Init(TFormat sampleFormat) noexcept override;

private:
    pa_simple* SoundDevice = nullptr;
    TFormat CurrentFormat;
};

}

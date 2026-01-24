#pragma once

#include <system_error>

namespace NWrite {

enum EErrorCode {
    Ok,
    DeviceInit
};

}

namespace std {

template<> struct is_error_code_enum<NWrite::EErrorCode> : std::true_type{};

}

namespace NWrite {

class TErrorCategory final : public std::error_category {
public:
    const char* name() const noexcept override {
        return "write error";
    }

    std::string message(int value) const override {
        switch (value) {
            case Ok:
                return "ok";
            case DeviceInit:
                return "device init error";
        }

        return "unknown error code: " + std::to_string(value);
    }

    static const TErrorCategory& instance() {
        static const TErrorCategory errorCategory;
        return errorCategory;
    }
};

inline std::error_code make_error_code(EErrorCode errorCode) {
    return {errorCode, TErrorCategory::instance()};
}

}

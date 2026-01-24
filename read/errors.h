#pragma once

#include <system_error>

namespace NRead {

enum EErrorCode {
    Ok,
    FileOpen,
    FileFormat,
    FileEnd
};

}

namespace std {

template<> struct is_error_code_enum<NRead::EErrorCode> : std::true_type{};

}

namespace NRead {

class TErrorCategory final : public std::error_category {
public:
    const char* name() const noexcept override {
        return "read error";
    }

    std::string message(int value) const override {
        switch (value) {
            case Ok:
                return "ok";
            case FileOpen:
                return "file open error";
            case FileFormat:
                return "file format error";
            case FileEnd:
                return "end of file";
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
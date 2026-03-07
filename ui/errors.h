#pragma once

#include <system_error>

namespace NUI {

enum EErrorCode {
    Ok,
    SocketOpen,
    SocketBind,
    SocketListen,
    SocketAccept,
    SocketInit
};

}

namespace std {

template<> struct is_error_code_enum<NUI::EErrorCode> : std::true_type{};

}

namespace NUI {

class TErrorCategory final : public std::error_category {
public:
    const char* name() const noexcept override {
        return "ui error";
    }

    std::string message(int value) const override {
        switch (value) {
            case Ok:
                return "ok";
            case SocketOpen:
                return "socket open error";
            case SocketBind:
                return "socket bind error";
            case SocketListen:
                return "socket listen error";
            case SocketAccept:
                return "socket accept error";
            case SocketInit:
                return "socket init error";
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

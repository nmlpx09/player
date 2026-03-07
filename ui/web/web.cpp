#include "http_headers.h"
#include "index.h"
#include "web.h"

#include <ui/errors.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {
    void SockWrite(std::int32_t socketFd, const std::string& response) {
        write(socketFd, response.c_str(), response.size());
        close(socketFd);
    }

    std::string MakeResponse(const std::string& headers, const std::string& payload) {
        return headers + std::to_string(payload.size()) + "\r\n\r\n" + payload;
    }

    std::string MakeResponse(const std::string& headers) {
        return MakeResponse(headers, "");
    }
}

namespace NUI {

TWeb::TWeb(std::size_t winWidth, std::size_t winHight, std::uint16_t port)
: WinWidth(winWidth)
, WinHight(winHight)
, Port(port) { }

TWeb::~TWeb() {
    close(Sockfd);
    Sockfd = -1;
}

std::error_code TWeb::Init() noexcept {
    if (Sockfd = socket(AF_INET, SOCK_STREAM, 0); Sockfd < 0) {
        return EErrorCode::SocketOpen;
    }

    auto servaddr = sockaddr_in {
        .sin_family = AF_INET,
        .sin_port = htons(Port),
        .sin_addr = {
            .s_addr = htonl(INADDR_ANY)
        },
        .sin_zero = {0}
    };

    if (bind(Sockfd, reinterpret_cast<const sockaddr*>(&servaddr), sizeof(servaddr)) < 0) {
        return EErrorCode::SocketBind;
    }

    return {};
}

void TWeb::ListDraw(
    const TFiles& filesList,
    std::size_t current
) noexcept {
    std::size_t begin = 0;
    std::size_t end = WinHight;

    if (current >= WinHight) {
        begin = current - WinHight + 1;
        end = current + 1;
    }

    std::string result;

    for (std::size_t i = begin, j = 1; i != end; ++j, ++i) {
        std::string message;
        if (i < filesList.size()) {
            message = filesList[i].filename().string();

            if (message.size() >= WinWidth - 2) {
                message.resize(WinWidth - 2);
            }

            result += R"({"name":")" + message + R"(")";
            if (current == i) {
                result += R"(, "current":true})";
            } else {
                result += R"(, "current":false})";
            }

            if (j != WinHight && i < filesList.size() - 1) {
                result += ",";
            }
        }
    }
    result = "[" + result + "]";

    std::unique_lock<std::mutex> ulock{Mutex};
    List = result;
}

void TWeb::StatusDraw(const std::string& message) noexcept {
    std::unique_lock<std::mutex> ulock{Mutex};
    Status = message;
}

void TWeb::StatusClean() noexcept {
    std::unique_lock<std::mutex> ulock{Mutex};
    Status.clear();
}

std::string TWeb::GetList() noexcept {
    std::unique_lock<std::mutex> ulock{Mutex};
    return List;
}

std::string TWeb::GetStatus() noexcept {
    std::unique_lock<std::mutex> ulock{Mutex};
    return Status;
}

std::expected<ECommands, std::error_code> TWeb::GetCommand() noexcept {
    if (Sockfd < 0) {
        return std::unexpected(EErrorCode::SocketInit);
    }

    while(true) {
        if ((listen(Sockfd, 8)) != 0) {
            return std::unexpected(EErrorCode::SocketListen);
        }

        sockaddr_in clientaddr;
        socklen_t clientaddrSize = sizeof(clientaddr);

        auto connFd = accept(Sockfd, reinterpret_cast<sockaddr*>(&clientaddr), &clientaddrSize);

        if (connFd < 0) {
            return std::unexpected(EErrorCode::SocketAccept);
        }

        std::string buffer(RequestSize, '\0');
        read(connFd, &buffer[0], RequestSize);

        if (buffer.find("GET / HTTP/1.1") != std::string::npos) {
            SockWrite(connFd, MakeResponse(HTTP_INDEX, INDEX));
        } else if (buffer.find("/list") != std::string::npos) {
            SockWrite(connFd, MakeResponse(HTTP_LIST, GetList()));
        } else if (buffer.find("/status") != std::string::npos) {
            SockWrite(connFd, MakeResponse(HTTP_STATUS, GetStatus()));
        } else if (buffer.find("/up") != std::string::npos) {
            SockWrite(connFd, MakeResponse(HTTP_OK));
            return ECommands::UP;
        } else if (buffer.find("/down") != std::string::npos) {
            SockWrite(connFd, MakeResponse(HTTP_OK));
            return ECommands::DOWN;
        } else if (buffer.find("/enter") != std::string::npos) {
            SockWrite(connFd, MakeResponse(HTTP_OK));
            return ECommands::ENTER;
        } else if (buffer.find("/exit") != std::string::npos) {
            SockWrite(connFd, MakeResponse(HTTP_OK));
            return ECommands::EXIT;
        } else if (buffer.find("/play") != std::string::npos) {
            SockWrite(connFd, MakeResponse(HTTP_OK));
            return ECommands::PLAY;
        } else if (buffer.find("/stop") != std::string::npos) {
            SockWrite(connFd, MakeResponse(HTTP_OK));
            return ECommands::STOP;
        }
    }
}

}

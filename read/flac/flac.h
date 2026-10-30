#pragma once

#include <common/types.h>
#include <read/interface.h>

#include "FLAC++/decoder.h"

#include <cstdint>
#include <vector>
#include <string>

namespace NRead {

struct TFlacDecoder: FLAC::Decoder::File {
public:
    FLAC__StreamDecoderWriteStatus write_callback(const FLAC__Frame* frame, const FLAC__int32* const buffer[]) noexcept override;
    void metadata_callback(const FLAC__StreamMetadata* metadata) noexcept override;
    void error_callback(FLAC__StreamDecoderErrorStatus status) noexcept override;

    TCallback Callback;
    std::size_t DataSize = 0;
    std::uint32_t NumChannels = 0;
    std::uint32_t BitsPerSample = 0;
    std::uint32_t BytesPerSample = 0;
    std::uint32_t SampleRate = 0;

    std::vector<TData::value_type> Buffer;
};

struct TFlac: TInterface {
public:
    TFlac() = default;
    TFlac(const TFlac&) = delete;
    TFlac(TFlac&&) noexcept = delete;
    ~TFlac() noexcept = default;
    TFlac& operator=(const TFlac&) = delete;
    TFlac& operator=(TFlac&&) = delete;

    std::expected<TFormat, std::error_code> Init(std::string fileName) override;
    std::expected<bool, std::error_code> Read(const TCallback& callback) override;

private:
    TFlacDecoder Decoder;
};

}

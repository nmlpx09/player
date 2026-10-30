#include "flac.h"

#include <read/errors.h>

namespace NRead {

void TFlacDecoder::metadata_callback(const FLAC__StreamMetadata *metadata) noexcept {
    if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        SampleRate = metadata->data.stream_info.sample_rate;
        NumChannels = metadata->data.stream_info.channels;
        BitsPerSample = metadata->data.stream_info.bits_per_sample;
        BytesPerSample = BitsPerSample / 8;
    }
}

FLAC__StreamDecoderWriteStatus TFlacDecoder::write_callback(const FLAC__Frame* frame, const FLAC__int32* const frames[]) noexcept {
    try {
        if (!Callback) {
            return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        }

        for (auto index = 0u; index < frame->header.blocksize; ++index) {
            const auto* l = reinterpret_cast<const std::uint8_t*>(&frames[0][index]);
            const auto* r = reinterpret_cast<const std::uint8_t*>(&frames[1][index]);
            Buffer.insert(Buffer.end(), l, l + BytesPerSample);
            Buffer.insert(Buffer.end(), r, r + BytesPerSample);
            if (Buffer.size() == DataSize) {
                Callback(std::move(Buffer));
                Buffer.reserve(DataSize);
            }

        }

        return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;

    } catch (...) {
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
}

void TFlacDecoder::error_callback(FLAC__StreamDecoderErrorStatus) noexcept { }

std::expected<TFormat, std::error_code> TFlac::Init(std::string fileName) {
    auto status = Decoder.init(fileName);

    if(status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        return std::unexpected(EErrorCode::FileOpen);
    }

    Decoder.process_until_end_of_metadata();

    if (Decoder.NumChannels != 2) {
        return std::unexpected(EErrorCode::FileFormat);
    }

    if (!TFormatPermited::SampleRate.contains(Decoder.SampleRate)) {
        return std::unexpected(EErrorCode::FileFormat);
    }

    if (!TFormatPermited::BitsPerSample.contains(Decoder.BitsPerSample)) {
        return std::unexpected(EErrorCode::FileFormat);
    }

    Decoder.DataSize = Decoder.NumChannels * Decoder.SampleRate * Decoder.BytesPerSample;
    Decoder.Buffer.reserve(Decoder.DataSize);

    return TFormat {
        .BitsPerSample = Decoder.BitsPerSample,
        .SampleRate =  Decoder.SampleRate
    };
}

std::expected<bool, std::error_code> TFlac::Read(const TCallback& callback) {
    Decoder.Callback = callback;

    if (!Decoder.process_single()) {
        if (Decoder.get_state() != FLAC__STREAM_DECODER_END_OF_STREAM) {
            return std::unexpected(EErrorCode::Decode);
        }
    }

    if (Decoder.get_state() == FLAC__STREAM_DECODER_END_OF_STREAM) {
        return false;
    }


    return true;
}

}

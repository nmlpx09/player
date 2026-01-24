#include "errors.h"
#include "flac.h"

namespace NRead {

void TFlacDecoder::metadata_callback(const FLAC__StreamMetadata *metadata) {
    if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        SampleRate = metadata->data.stream_info.sample_rate;
        NumChannels = metadata->data.stream_info.channels;
        BitsPerSample = metadata->data.stream_info.bits_per_sample;
        BytesPerSample = BitsPerSample / 8;
    }
}

FLAC__StreamDecoderWriteStatus TFlacDecoder::write_callback(const FLAC__Frame* frame, const FLAC__int32* const frames[]) {
    if (!Callback) {
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }

    for (auto index = 0u; index < frame->header.blocksize; ++index) {
        const auto* l = reinterpret_cast<const std::uint8_t*>(&frames[0][index]);
        const auto* r = reinterpret_cast<const std::uint8_t*>(&frames[1][index]);
        Buffer.insert(Buffer.end(), l, l + BytesPerSample);
        Buffer.insert(Buffer.end(), r, r + BytesPerSample);
    }

    while (Buffer.size() >= DataSize) {
        Callback(TData(Buffer.begin(), Buffer.begin() + DataSize));
        Buffer.erase(Buffer.begin(), Buffer.begin() + DataSize);
    }

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void TFlacDecoder::error_callback(FLAC__StreamDecoderErrorStatus) { }

TFlac::TFlac() {}

TFlac::~TFlac() {}

std::expected<TFormat, std::error_code> TFlac::Init(std::string fileName) noexcept {
    auto status = Decoder.init(fileName);

    if(status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        return std::unexpected(EErrorCode::FileOpen);
    }

    Decoder.process_until_end_of_metadata();

    if (!TFormatPermited::NumChannels.contains(Decoder.NumChannels)) {
        return std::unexpected(EErrorCode::FileFormat);
    }

    if (!TFormatPermited::SampleRate.contains(Decoder.SampleRate)) {
        return std::unexpected(EErrorCode::FileFormat);
    }

    if (!TFormatPermited::BitsPerSample.contains(Decoder.BitsPerSample)) {
        return std::unexpected(EErrorCode::FileFormat);
    }

    Decoder.DataSize = Decoder.NumChannels * Decoder.SampleRate * Decoder.BytesPerSample;

    return TFormat {
        .BitsPerSample = Decoder.BitsPerSample,
        .NumChannels = Decoder.NumChannels,
        .SampleRate =  Decoder.SampleRate
    };
}

std::expected<bool, std::error_code> TFlac::Read(const TCallback& callback) noexcept {
    Decoder.Callback = callback;

    if (Decoder.get_state() == FLAC__STREAM_DECODER_END_OF_STREAM) {
        return false;
    }

    Decoder.process_single();

    return true;
}

}

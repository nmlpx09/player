#include "pulse.h"

#include <write/errors.h>

namespace NWrite {

TWrite::TWrite(std::string) noexcept { }

TWrite::~TWrite() noexcept {
    if (SoundDevice != nullptr) {
        pa_simple_drain(SoundDevice, nullptr);
        pa_simple_free(SoundDevice);
    }
}

std::error_code TWrite::Init(TFormat sampleFormat) noexcept {
    if (SoundDevice != nullptr) {
        pa_simple_drain(SoundDevice, nullptr);
        pa_simple_free(SoundDevice);
        SoundDevice = nullptr;
    }

    pa_sample_format_t format;

    if (!TFormatPermited::NumChannels.contains(sampleFormat.NumChannels)) {
        return EErrorCode::DeviceInit;
    }

    if (!TFormatPermited::BitsPerSample.contains(sampleFormat.BitsPerSample)) {
        return EErrorCode::DeviceInit;
    }

    std::size_t frameSize = 1;
    if (sampleFormat.BitsPerSample == 24) {
        format = PA_SAMPLE_S24LE;
        frameSize = 3 * sampleFormat.NumChannels;
    } else if (sampleFormat.BitsPerSample == 16) {
        format = PA_SAMPLE_S16LE;
        frameSize = 2 * sampleFormat.NumChannels;
    } else {
        return EErrorCode::DeviceInit;
    }

    if (!TFormatPermited::SampleRate.contains(sampleFormat.SampleRate)) {
        return EErrorCode::DeviceInit;
    }

    pa_sample_spec spec = {
        .format = format,
        .rate = sampleFormat.SampleRate,
        .channels = static_cast<std::uint8_t>(sampleFormat.NumChannels)
    };

    pa_buffer_attr bufferAttr = {
        .maxlength = static_cast<std::uint32_t>(-1),
        .tlength = static_cast<std::uint32_t>(frameSize * sampleFormat.SampleRate),
        .prebuf = static_cast<std::uint32_t>(-1),
        .minreq = static_cast<std::uint32_t>(-1),
        .fragsize = static_cast<std::uint32_t>(-1)
    };

    if (SoundDevice = pa_simple_new(nullptr, "play", PA_STREAM_PLAYBACK, nullptr, "playback", &spec, nullptr, &bufferAttr, nullptr); !SoundDevice) {
        return EErrorCode::DeviceInit;
    }

    return {};
}

std::error_code TWrite::Write(const TCallback& callback) {
    auto data = callback();
    
    auto&& [format, buffer] = data;

    if (CurrentFormat != format) {
        if (auto ec = Init(format); ec) {
            return ec;
        } else {
            CurrentFormat = format;
        }
    }

    if (auto err = pa_simple_write(SoundDevice, buffer.data(), buffer.size(), nullptr); err < 0) {
        if (auto ec = Init(format); ec) {
            return ec;
        }
        if (err = pa_simple_write(SoundDevice, buffer.data(), buffer.size(), nullptr); err < 0) {
            return EErrorCode::Error;
        }
    }
    return {};
}

}

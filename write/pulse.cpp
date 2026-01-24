#include "pulse.h"
#include "errors.h"

namespace NWrite {

TWrite::TWrite(std::string) { }

TWrite::~TWrite() {
    if (SoundDevice != nullptr) {
        pa_simple_drain(SoundDevice, nullptr);
        pa_simple_free(SoundDevice);
    }
}

std::error_code TWrite::Init(TFormat sampleFormat) noexcept {
    pa_sample_format_t format;

    if (!TFormatPermited::NumChannels.contains(sampleFormat.NumChannels)) {
        return EErrorCode::DeviceInit;
    }

    if (!TFormatPermited::BitsPerSample.contains(sampleFormat.BitsPerSample)) {
        return EErrorCode::DeviceInit;
    }

    if (sampleFormat.BitsPerSample == 24) {
        format = PA_SAMPLE_S24LE;
    } else if (sampleFormat.BitsPerSample == 16) {
        format = PA_SAMPLE_S16LE;
    } else if (sampleFormat.BitsPerSample == 32) {
        format = PA_SAMPLE_S32LE;
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

    if (SoundDevice = pa_simple_new(nullptr, "play", PA_STREAM_PLAYBACK, nullptr, "playback", &spec, nullptr, nullptr, nullptr); !SoundDevice) {
        return EErrorCode::DeviceInit;
    }

    return {};
}

std::error_code TWrite::Write(const TCallback& callback) noexcept {
    if (auto data = callback(); data) {
        auto&& [format, buffer] = data.value();

        if (CurrentFormat != format) {
            if (auto ec = Init(format); ec) {
                return ec;
            } else {
                CurrentFormat = format;
            }
        }

        pa_simple_write(SoundDevice, buffer.data(), buffer.size(), nullptr);
    }
    return {};
}

}

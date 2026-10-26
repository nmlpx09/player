#include "alsa.h"

#include <write/errors.h>

namespace NWrite {

TWrite::TWrite(std::string device) noexcept : Device(std::move(device)) { }

TWrite::~TWrite() noexcept {
    if (SoundDevice != nullptr) {
        snd_pcm_drain(SoundDevice);
        snd_pcm_close(SoundDevice);
    }
}

std::error_code TWrite::Init(TFormat sampleFormat) noexcept {
    if (SoundDevice != nullptr) {
        snd_pcm_drain(SoundDevice);
        snd_pcm_close(SoundDevice);
        SoundDevice = nullptr;
    }

    if (auto err = snd_pcm_open(&SoundDevice, Device.c_str(), SND_PCM_STREAM_PLAYBACK, 0); err < 0) {
        return EErrorCode::DeviceInit;
    }

    snd_pcm_hw_params_t* hwParams;
    snd_pcm_hw_params_alloca(&hwParams);
 
    if (auto err = snd_pcm_hw_params_any(SoundDevice, hwParams); err < 0) {
        return EErrorCode::DeviceInit;
    }

    if (auto err = snd_pcm_hw_params_set_rate_resample(SoundDevice, hwParams, 1); err < 0) {
        return EErrorCode::DeviceInit;
    }

    if (auto err = snd_pcm_hw_params_set_access(SoundDevice, hwParams, SND_PCM_ACCESS_RW_INTERLEAVED); err < 0) {
        return EErrorCode::DeviceInit;
    }

    if (!TFormatPermited::NumChannels.contains(sampleFormat.NumChannels)) {
        return EErrorCode::DeviceInit;
    }

    if (auto err = snd_pcm_hw_params_set_channels(SoundDevice, hwParams, sampleFormat.NumChannels); err < 0) {
        return EErrorCode::DeviceInit;
    }

    if (!TFormatPermited::BitsPerSample.contains(sampleFormat.BitsPerSample)) {
        return EErrorCode::DeviceInit;
    }

    snd_pcm_format_t format;
    if (sampleFormat.BitsPerSample == 24) {
        format = SND_PCM_FORMAT_S24_3LE;
        FrameSize = 3 * sampleFormat.NumChannels;
    } else if (sampleFormat.BitsPerSample == 16) {
        format = SND_PCM_FORMAT_S16_LE;
        FrameSize = 2 * sampleFormat.NumChannels;
    } else {
        return EErrorCode::DeviceInit;
    }

    if (auto err = snd_pcm_hw_params_set_format(SoundDevice, hwParams, format); err < 0) {
        return EErrorCode::DeviceInit;
    }

    if (!TFormatPermited::SampleRate.contains(sampleFormat.SampleRate)) {
        return EErrorCode::DeviceInit;
    }

    if (auto err = snd_pcm_hw_params_set_rate_near(SoundDevice, hwParams, &sampleFormat.SampleRate, 0); err < 0) {
        return EErrorCode::DeviceInit;
    }

    if (auto err = snd_pcm_hw_params(SoundDevice, hwParams); err < 0) {
        return EErrorCode::DeviceInit;
    }

    if (auto err = snd_pcm_prepare(SoundDevice); err < 0) {
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

    if (FrameSize == 0 || buffer.empty()) {
        return {};
    }

    const auto framesTotal = static_cast<snd_pcm_sframes_t>(buffer.size() / FrameSize);
    auto* ptr = buffer.data();
    auto remaining = framesTotal;

    while (remaining > 0) {
        auto written = snd_pcm_writei(SoundDevice, ptr, remaining);
        if (written > 0) {
            ptr += written * FrameSize;
            remaining -= written;
            continue;
        }

        if (written == 0) {
            return EErrorCode::Error;
        }

        if (written == -ENODEV) {
            if (auto ec = Init(format); ec) {
                return ec;
            }
        } else if (snd_pcm_recover(SoundDevice, static_cast<std::int32_t>(written), 1) < 0) {
            return EErrorCode::DeviceInit;
        }
    }

    return {};
}

}

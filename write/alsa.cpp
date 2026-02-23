#include "alsa.h"
#include "errors.h"

namespace NWrite {

TWrite::TWrite(std::string device) : Device(std::move(device)) { }

TWrite::~TWrite() {
    if (SoundDevice != nullptr) {
        snd_pcm_drain(SoundDevice);
        snd_pcm_close(SoundDevice);
    }
}

std::error_code TWrite::Init(TFormat sampleFormat) noexcept {
    if (auto err = snd_pcm_open(&SoundDevice, Device.c_str(), SND_PCM_STREAM_PLAYBACK, 0); err < 0) {
        return make_error_code(EErrorCode::DeviceInit);
    }

    snd_pcm_hw_params_t* hwParams;
    snd_pcm_hw_params_alloca(&hwParams);
 
    if (auto err = snd_pcm_hw_params_any(SoundDevice, hwParams); err < 0) {
        return make_error_code(EErrorCode::DeviceInit);
    }

    if (auto err = snd_pcm_hw_params_set_rate_resample(SoundDevice, hwParams, 1); err < 0) {
        return make_error_code(EErrorCode::DeviceInit);
    }

    if (auto err = snd_pcm_hw_params_set_access(SoundDevice, hwParams, SND_PCM_ACCESS_RW_INTERLEAVED); err < 0) {
        return make_error_code(EErrorCode::DeviceInit);
    }

    if (!TFormatPermited::NumChannels.contains(sampleFormat.NumChannels)) {
        return EErrorCode::DeviceInit;
    }

    if (auto err = snd_pcm_hw_params_set_channels(SoundDevice, hwParams, sampleFormat.NumChannels); err < 0) {
        return make_error_code(EErrorCode::DeviceInit);
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
    } else if (sampleFormat.BitsPerSample == 32) {
        format = SND_PCM_FORMAT_S32_LE;
        FrameSize = 4 * sampleFormat.NumChannels;
    } else {
        return make_error_code(EErrorCode::DeviceInit);
    }

    if (auto err = snd_pcm_hw_params_set_format(SoundDevice, hwParams, format); err < 0) {
        return make_error_code(EErrorCode::DeviceInit);
    }

    if (!TFormatPermited::SampleRate.contains(sampleFormat.SampleRate)) {
        return EErrorCode::DeviceInit;
    }

    if (auto err = snd_pcm_hw_params_set_rate_near(SoundDevice, hwParams, &sampleFormat.SampleRate, 0); err < 0) {
        return make_error_code(EErrorCode::DeviceInit);
    }

    if (auto err = snd_pcm_hw_params(SoundDevice, hwParams); err < 0) {
        return make_error_code(EErrorCode::DeviceInit);
    }

    if (auto err = snd_pcm_prepare(SoundDevice); err < 0) {
        return make_error_code(EErrorCode::DeviceInit);
    }

    return {};
}

std::error_code TWrite::Write(const TCallback& callback) noexcept {
    auto data = callback();

    auto&& [_, format, buffer] = data;

    if (CurrentFormat != format) {
        if(auto ec = Init(format); ec) {
            return ec;
        } else {
            CurrentFormat = format;
        }
    }

    auto frames = buffer.size() / FrameSize;

    auto err = snd_pcm_writei(SoundDevice, buffer.data(), frames);

    if (err == -ENODEV) {
        if (auto ec = Init(format); ec) {
            return ec;
        }
    } else if (err == -EPIPE) {
        snd_pcm_prepare(SoundDevice);
        snd_pcm_writei(SoundDevice, buffer.data(), frames);
    }

    return {};
}

}

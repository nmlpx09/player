# about

simple console audio flac player

1. audio output - alsa or pulse
2. flac - 192000, 96000, 48000, 44100 Hz, 16 or 24 bit stereo format

# requirement

1. g++ 13 or newer (c++23, std::expected)
2. make
3. libasound2-dev (alsa backend)
4. libpulse-dev (pulse backend)
5. libflac++-dev

# build

make alsa   # build with alsa backend
make pulse  # build with pulse backend

# install

make install
# optional: make install PREFIX=/usr/local DESTDIR=/tmp/pkg

# usage

play dir [device]

dir    - directory or single .flac file
device - audio device name (alsa backend only, default: pulse)

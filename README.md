# about

audio flac player for smallest system

1. ui - ncurses
2. audio output - alsa or pulseaudio
3. flac - 192000, 96000, 48000, 44100 Hz, 16 or 24 bit stereo format

# requirement

1. clang-20
2. make
3. libasound2-dev
4. libflac++-dev
5. libncurses5-dev
6. libpulse0

# build

make

# install

make install

# control

1. KEY_UP - up in  music list
2. KEY_DOWN - down music list
3. q - quit
4. p - play
5. s - stop (with small delay)

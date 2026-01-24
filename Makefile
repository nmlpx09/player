CC = clang++-20
CCFLAGS = -O3 -ffast-math -std=c++23 -pedantic -Wall -W -Werror -Wextra -c -I.
DEFINES =

OBJ = bin/main.o read/flac.o ui/cui.o common/context.o
LDFLAGS =  -lFLAC++ -lncurses

OBJ_ALSA = $(OBJ) write/alsa.o
LDFLAGS_ALSA = $(LDFLAGS) -lasound

OBJ_PULSE =  $(OBJ) write/pulse.o
LDFLAGS_PULSE = $(LDFLAGS) -lpulse-simple

RUN = bin/play

alsa: DEFINES += -DALSA
alsa: $(OBJ_ALSA)
	$(CC) $^ -o $(RUN) $(LDFLAGS_ALSA)

pulse: $(OBJ_PULSE)
	$(CC) $^ -o $(RUN) $(LDFLAGS_PULSE)

%.o: %.cpp
	$(CC) $(DEFINES) $(CCFLAGS) $< -o $@

install:
	strip $(RUN)
	cp $(RUN) /usr/bin/

install_termux:
	cp $(RUN) /data/data/com.termux/files/usr/bin/

clean:
	rm -f $(OBJ_ALSA) $(OBJ_PULSE) $(RUN)

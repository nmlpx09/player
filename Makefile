CC = g++
CCFLAGS = -O3 --std=c++23 -pedantic -Wall -W -Werror -Wextra -c -I.
DEFINES =

OBJ = main.o common/context.o read/flac/flac.o

OBJ_ALSA = $(OBJ) write/alsa/alsa.o
LDFLAGS_ALSA = -lasound -lFLAC++

OBJ_PULSE =  $(OBJ) write/pulse/pulse.o
LDFLAGS_PULSE = -lpulse-simple -lFLAC++

RUN = play
PREFIX ?= /usr
DESTDIR ?=

alsa: DEFINES += -DALSA
alsa: $(OBJ_ALSA)
	$(CC) $^ -o $(RUN) $(LDFLAGS_ALSA)

pulse: $(OBJ_PULSE)
	$(CC) $^ -o $(RUN) $(LDFLAGS_PULSE)

%.o: %.cpp
	$(CC) $(DEFINES) $(CCFLAGS) $< -o $@

install: $(RUN)
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 -s $(RUN) $(DESTDIR)$(PREFIX)/bin/

clean:
	rm -f $(OBJ_ALSA) $(OBJ_PULSE) $(RUN)

.PHONY: alsa pulse install install_termux clean

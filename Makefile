CC = clang++-20
CCFLAGS = -O3 -ffast-math -std=c++23 -pedantic -Wall -W -Werror -Wextra -c -I.
DEFINES =

OBJ = bin/main.o read/flac/flac.o ui/web/web.o common/context.o write/alsa/alsa.o
LDFLAGS= -lFLAC++ -lncurses -lasound

RUN = bin/play

alsa: $(OBJ)
	$(CC) $^ -o $(RUN) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(DEFINES) $(CCFLAGS) $< -o $@

install:
	strip $(RUN)
	cp $(RUN) /usr/bin/

clean:
	rm -f $(OBJ) $(RUN)

CC = clang++-20
CCFLAGS = -O3 -ffast-math -std=c++23 -pedantic -Wall -W -Werror -Wextra -c -I.
DEFINES =

OBJ = bin/main.o read/flac.o ui/cui.o common/context.o write/alsa.o
LDFLAGS= -lFLAC++ -lncurses -lasound

RUN = bin/play

alsa: $(OBJ)
	$(CC) $^ -o $(RUN) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(DEFINES) $(CCFLAGS) $< -o $@

install:
	strip $(RUN)
	cp $(RUN) /usr/bin/

install_termux:
	cp $(RUN) /data/data/com.termux/files/usr/bin/

clean:
	rm -f $(OBJ) $(RUN)

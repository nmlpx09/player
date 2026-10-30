CXX ?= g++
CXXFLAGS ?= -O3 -std=c++23 -pedantic -Wall -W -Werror -Wextra
CXXFLAGS += -I. $(FLAC_CFLAGS)
LDFLAGS ?=

PREFIX ?= /usr
DESTDIR ?=
RUN := play

# pkg-config with fallbacks
FLAC_CFLAGS := $(shell pkg-config --cflags flac++ 2>/dev/null)
FLAC_LIBS   := $(shell pkg-config --libs flac++ 2>/dev/null || echo -lFLAC++)
ALSA_LIBS   := $(shell pkg-config --libs alsa 2>/dev/null || echo -lasound)

OBJ := main.o common/context.o read/flac/flac.o write/alsa/alsa.o
LDLIBS := $(ALSA_LIBS) $(FLAC_LIBS)

DEPS := $(OBJ:.o=.d)

.PHONY: all install clean

all: $(RUN)

$(RUN): $(OBJ)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

install: $(RUN)
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 -s $(RUN) $(DESTDIR)$(PREFIX)/bin/

clean:
	rm -f $(OBJ) $(DEPS) $(RUN)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

LIBS = WPEFrameworkCore WPEFrameworkPlugins
CXXFLAGS += $(shell pkg-config --cflags $(LIBS))
LDFLAGS += $(shell pkg-config --libs $(LIBS))

CXXFLAGS += -std=c++11 -O0 -DUSE_R2=1

all: launcher

launcher: Makefile main.cpp
	$(CXX) -g -I. $(CXXFLAGS) main.cpp -o launcher  $(LDFLAGS)

clean:
	rm -f launcher

.PHONY: all

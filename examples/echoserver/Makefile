CXX = g++
CXXFLAGS = -std=c++17 -Wall -g
EXT = .cpp
OUTEXT = .out

SRCS := $(wildcard *$(EXT))
TARGETS := $(SRCS:$(EXT)=)

.PHONY: all
all: $(TARGETS)

%: %$(EXT)
	$(CXX) $(CXXFLAGS) $< -o $@$(OUTEXT)

.PHONY: clean
clean:
	rm -f *$(OUTEXT)


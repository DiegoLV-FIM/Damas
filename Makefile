ifeq ($(OS),Windows_NT)
    RAYLIB = ./raylib-6.0/mingw
    LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm
    EXT = .exe
else
    RAYLIB = ./raylib-6.0/linux
    LDLIBS = -l:libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11
    EXT =
endif

CXX ?= g++
CXXFLAGS := -std=c++17 -Wall -Wextra -I$(RAYLIB)/include
LDFLAGS  := -L$(RAYLIB)/lib

.PHONY: all run clean

TARGET := Damas$(EXT)
all: $(TARGET)

$(TARGET): Damas.cpp
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe

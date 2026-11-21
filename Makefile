# Settlers - Makefile
# Requires raylib installed:
#   macOS:   brew install raylib
#   Linux:   sudo apt install libraylib-dev   (or build raylib from source)
#   Windows: MSYS2 / MinGW:  pacman -S mingw-w64-ucrt-x86_64-raylib
# If raylib is somewhere else:  make RAYLIB_DIR=/path/to/raylib
#   (expects $(RAYLIB_DIR)/include/raylib.h and $(RAYLIB_DIR)/lib/libraylib.a)

CXX      ?= g++
CXXFLAGS ?= -std=c++20 -Wall -Wextra -O2
TARGET   := settlers
SRC      := main.cpp board.cpp game.cpp player.cpp render.cpp
OBJ      := $(SRC:.cpp=.o)

# ---- raylib flags per platform ----
ifdef RAYLIB_DIR
    CPPFLAGS += -isystem $(RAYLIB_DIR)/include
    LDFLAGS  += -L$(RAYLIB_DIR)/lib
endif

ifeq ($(OS),Windows_NT)
    TARGET := settlers.exe
    LDLIBS += -lraylib -lopengl32 -lgdi32 -lwinmm
    RM     := del /Q
else
    UNAME := $(shell uname -s)
    ifeq ($(UNAME),Darwin)
        CPPFLAGS += -isystem $(shell brew --prefix 2>/dev/null)/include
        LDFLAGS  += -L$(shell brew --prefix 2>/dev/null)/lib
        LDLIBS   += -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    else
        LDLIBS   += -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    endif
    RM := rm -f
endif

# ---- rules ----
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS) $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) $(OBJ) $(OBJ:.o=.d) $(TARGET)

-include $(OBJ:.o=.d)

.PHONY: all run clean
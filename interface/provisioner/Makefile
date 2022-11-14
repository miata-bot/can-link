#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1 and Mac OS X
#
# You will need SDL2 (http://www.libsdl.org):
# Linux:
#   apt-get install libsdl2-dev
# Mac OS X:
#   brew install sdl2
# MSYS2:
#   pacman -S mingw-w64-i686-SDL2
#

ifeq ($(TARGET), emscripten)
CC = emcc
CXX = em++
EMSCRIPTEN_DIR = emscripten
WEB_DIR = web
EXE = $(WEB_DIR)/index.html
EMS =
EMS += -s USE_SDL=2
EMS += -s DISABLE_EXCEPTION_CATCHING=1
LDFLAGS += -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=0 -s ASSERTIONS=1

else
EXE = provision
endif

#CXX = g++
#CXX = clang++

IMGUI_DIR = lib/imgui
IMGUI_COLOR_TEXT_EDIT_DIR = lib/ImGuiColorTextEdit

# SQLite specific
SQLITE_DIR       = lib/sqlite
SQLITE_BUILD_DIR = $(SQLITE_DIR)/build

# NFD
NFD_DIR          = lib/nativefiledialog

SOURCES =  src/gitparams.cpp
SOURCES += src/main.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp
SOURCES += $(IMGUI_DIR)/imgui_demo.cpp
SOURCES += $(IMGUI_DIR)/imgui_draw.cpp
SOURCES += $(IMGUI_DIR)/imgui_tables.cpp
SOURCES += $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl.cpp 
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SOURCES += $(IMGUI_COLOR_TEXT_EDIT_DIR)/TextEditor.cpp

# SQLite Source
SOURCES += $(SQLITE_DIR)/build/sqlite3.c

# App code Source
SOURCES += src/provision.cpp
SOURCES += src/file_dialog.cpp
SOURCES += src/script_editor.cpp
SOURCES += src/spect.cpp
SOURCES += src/spect_config.cpp
SOURCES += src/spect_hardware_channels.cpp
SOURCES += src/spect_hardware_triggers.cpp
SOURCES += src/spect_network_nodes.cpp
SOURCES += src/spect_effect_slots.cpp
SOURCES += src/spect_script.cpp
SOURCES += src/help_modals.cpp

UNAME_S := $(shell uname -s)

# Flags for C and C++
CFLAGS  ?=
CFLAGS  += -Iinclude
CFLAGS  += -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CFLAGS  += -I$(IMGUI_COLOR_TEXT_EDIT_DIR)
CFLAGS  += -I$(SQLITE_DIR)/build
CFLAGS  += -I$(NFD_DIR)/src/include
CFLAGS  += -Wall -Wformat
CFLAGS  += -g -DDEBUG

# Flags for C++
CXXFLAGS = -std=c++11 
LDFLAGS ?=

ifeq ($(TARGET), emscripten)
# Emscripten allows preloading a file or folder to be accessible at runtime.
# The Makefile for this example project suggests embedding the misc/fonts/ folder into our application, it will then be accessible as "/fonts"
# See documentation for more details: https://emscripten.org/docs/porting/files/packaging_files.html
# (Default value is 0. Set to 1 to enable file-system and include the misc/fonts/ folder as part of the build.)
USE_FILE_SYSTEM ?= 1
ifeq ($(USE_FILE_SYSTEM), 0)
LDFLAGS += -s NO_FILESYSTEM=1
CPPFLAGS += -DIMGUI_DISABLE_FILE_FUNCTIONS
endif
ifeq ($(USE_FILE_SYSTEM), 1)
LDFLAGS += --no-heap-copy --preload-file imgui.ini@/imgui.ini
LDFLAGS += --no-heap-copy --preload-file config.db@/config.db
endif

CFLAGS +=  -Os $(EMS)
LDFLAGS += --shell-file $(EMSCRIPTEN_DIR)/shell_minimal.html $(EMS)
ECHO_MESSAGE = "Emscripten"
else

##---------------------------------------------------------------------
## OPENGL ES
##---------------------------------------------------------------------

## This assumes a GL ES library available in the system, e.g. libGLESv2.so
# CXXFLAGS += -DIMGUI_IMPL_OPENGL_ES2
# LINUX_GL_LIBS = -lGLESv2
## If you're on a Raspberry Pi and want to use the legacy drivers,
## use the following instead:
# LINUX_GL_LIBS = -L/opt/vc/lib -lbrcmGLESv2

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"

# SDL
	LINUX_GL_LIBS ?= -lGL
	LDFLAGS += $(LINUX_GL_LIBS) -ldl `sdl2-config --libs`
	CFLAGS += `sdl2-config --cflags`

# GTK for NFD
	LDFLAGS += `pkg-config --libs gtk+-3.0`
	CFLAGS += `pkg-config --cflags gtk+-3.0`
	SOURCES += $(NFD_DIR)/src/nfd_gtk.cpp
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LDFLAGS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo `sdl2-config --libs`
	LDFLAGS += -L/usr/local/lib -L/opt/local/lib

	CFLAGS += `sdl2-config --cflags`
	CFLAGS += -I/usr/local/include -I/opt/local/include
endif

# Windows Build is not done with this Makefile
# but I've retained the configuration just in case
ifeq ($(OS), Windows_NT)
    ECHO_MESSAGE = "MinGW"
    LDFLAGS += -lgdi32 -lopengl32 -limm32 `pkg-config --static --libs sdl2`

    CFLAGS += `pkg-config --cflags sdl2`
endif
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

.DEFAULT_GOAL := all
.PHONY: all run gdb valgrind clean dirclean

OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

all: $(WEB_DIR) $(EXE) 
	@echo Build complete for $(ECHO_MESSAGE)

# this file makes this project a non-reproducable build
# if this bothers you, please open an issue and it
# will be addressed.
include/gitparams_.h:
	echo -n "#define GIT_DESCRIBE " > include/gitparams_.h
	git describe --tags --dirty --match="provisioner-*" >> include/gitparams_.h
	echo -n "#define GIT_COMMIT " >> include/gitparams_.h
	git rev-parse --verify HEAD >> include/gitparams_.h
	echo -n "#define BUILD_EPOCH " >> include/gitparams_.h
	date >> include/gitparams_.h

gitparams.o: src/gitparams.cpp include/gitparams_.h
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c -o $@ src/gitparams.cpp

%.o:src/%.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_COLOR_TEXT_EDIT_DIR)/%.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c -o $@ $<

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CFLAGS) $(CXXFLAGS) $(LDFLAGS)

# build sqlite amalagation - this is overkill
# and a little unnecessary but oh well

$(SQLITE_BUILD_DIR):
	mkdir -p $(SQLITE_BUILD_DIR)

sqlite3.o: $(SQLITE_BUILD_DIR) $(SQLITE_BUILD_DIR)/sqlite3.c
	$(CC) $(CFLAGS) -c -o $@ $(SQLITE_BUILD_DIR)/sqlite3.c

$(SQLITE_BUILD_DIR)/sqlite3.c: 
	cd $(SQLITE_BUILD_DIR)/ &&\
	../configure &&\
	$(MAKE) sqlite3.c

%.o:$(NFD_DIR)/src/%.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(EXE) $(OBJS) $(WEB_DIR) include/gitparams_.h

# clean libs as well as normal clean
dirclean: clean
	rm -rf $(SQLITE_BUILD_DIR)
	
run: $(EXE)
	./$(EXE)

valgrind: $(EXE)
	valgrind ./$(EXE) --leak-check=yes

gdb: $(EXE)
	gdb ./$(EXE)

ifeq ($(TARGET), emscripten)
$(WEB_DIR):
	mkdir $@

serve: all
	python3 -m http.server -d $(WEB_DIR)
endif

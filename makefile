CC = clang
CFLAGS = -Wall -Wextra -Wno-deprecated-declarations -g

BUILD_DIR = build
SRC_DIR = src

LIB_DIRS = -L./dep/libs/GLFW

LIBS_MAC = -lglfw.3 -framework OpenGL
LIBS_WINDOWS = -lglfw3dll -lopengl32
LIBS_LINUX =

CMDS_MAC = install_name_tool -add_rpath @executable_path $(BUILD_DIR)/tailored

SRCS = $(wildcard $(SRC_DIR)/*.c)

ifeq ($(OS), Windows_NT)
	EXE_EXT = .exe
    LIBS = $(LIBS_WINDOWS)
	CMDS =
else
    UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Linux)
		EXE_EXT =
		LIBS = $(LIBS_LINUX)
		CMDS =
	endif
	ifeq ($(UNAME_S), Darwin)
		EXE_EXT =
		LIBS = $(LIBS_MAC)
		CMDS = $(CMDS_MAC)
	endif
endif

all: $(BUILD_DIR)/tailored$(EXE_EXT)

$(BUILD_DIR)/tailored$(EXE_EXT): $(SRCS)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/tailored$(EXE_EXT) $(SRCS) $(LIB_DIRS) $(LIBS)
	$(CMDS)

clean:
	rm -f $(BUILD_DIR)/

CC = clang
CFLAGS = -Wall -Wextra -Wno-deprecated-declarations -g
LDFLAGS = -L./dep/libs/GLFW -lglfw.3
FRAMEWORKS = -framework OpenGL
BUILD_DIR = build
SRC_DIR = src

SRCS = $(wildcard $(SRC_DIR)/*.c)

all: $(BUILD_DIR)/tailored

$(BUILD_DIR)/tailored: $(SRCS)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/tailored $(SRCS) $(LDFLAGS) $(FRAMEWORKS)
	install_name_tool -add_rpath @executable_path $(BUILD_DIR)/tailored
	./$(BUILD_DIR)/tailored

clean:
	rm -f $(BUILD_DIR)/

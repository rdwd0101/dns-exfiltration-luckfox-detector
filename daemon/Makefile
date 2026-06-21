CC := $(HOME)/Projects/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf-c++
CXXFLAGS := -Wall -O2
CPPFLAGS = -I$(SPDLOG_INC)
SPDLOG_INC = ../third_party/spdlog/include
RKNN_LIB_DIR = $(HOME)/Projects/rknn-toolkit2/rknpu2/runtime/Linux/librknn_api/armhf-uclibc/

SRCS = main.cpp

daemon: $(SRCS) features.hpp nn_model.hpp rknn_api.h
	$(CC) $(CXXFLAGS) $(CPPFLAGS) $(SRCS) -L$(RKNN_LIB_DIR) -lrknnmrt -o daemon

clean:
	rm -f daemon

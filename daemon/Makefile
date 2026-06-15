CC := ~/Projects/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf-c++
CXXFLAGS := -Wall -O2
CPPFLAGS = -I$(SPDLOG_INC)
SPDLOG_INC = ../third_party/spdlog/include

daemon: main.cpp features.hpp nn_model.hpp rknn_api.h
	$(CC) $(CXXFLAGS) $(CPPFLAGS) -L. -lrknnmrt $^ -o daemon

clean:
	rm -f main

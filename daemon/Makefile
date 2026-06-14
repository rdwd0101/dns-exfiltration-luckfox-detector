CC := ~/Projects/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf-c++
CFLAGS := -Wall -O2

daemon: main.cpp features.hpp nn_model.hpp rknn_api.h
	$(CC) $(CFLAGS) -L. -lrknnmrt $^ -o daemon

clean:
	rm -f main

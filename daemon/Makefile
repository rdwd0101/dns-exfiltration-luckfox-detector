CC := ~/Projects/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf-c++
CFLAGS := -Wall -O2

daemon: main.cpp features.hpp
	$(CC) $(CFLAGS) $^ -o daemon

clean:
	rm -f main

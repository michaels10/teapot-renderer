CXX = clang++
CXXFLAGS = -Wall -Werror -g --no-undefined -Ofast
SHAREDFLAGS = -fPIC -shared
LD_FLAGS = -lpthread -std=c++17

CACHE_LINE_SIZE = $(cat /sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size)

lib/render.so: src/render.cpp src/python_interface.cpp src/linalg.cpp src/render.h src/python_interface.h src/linalg.h 
	$(CXX)  $(CXXFLAGS) $(SHAREDFLAGS) -o lib/render.so src/render.cpp src/python_interface.cpp src/linalg.cpp $(LD_FLAGS)

clean:
	rm lib/render.so

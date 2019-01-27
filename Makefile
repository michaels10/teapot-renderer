CXX = clang++
CXXFLAGS = -Wall -Werror -g --no-undefined -Ofast
SHAREDFLAGS = -fPIC -shared
LD_FLAGS = -lpthread -std=c++17

CACHE_LINE_SIZE = $(cat /sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size)

libpyrender/librender.so: src/render.cpp src/python_interface.cpp src/linalg.cpp src/render.h src/python_interface.h src/linalg.h 
	$(CXX) $(CXXFLAGS) $(SHAREDFLAGS) -o libpyrender/librender.so src/render.cpp src/python_interface.cpp src/linalg.cpp $(LD_FLAGS)

render-tests: libpyrender/librender.so
	python3 libpyrender/test_void_teapot_refract_behind.py
	python3 libpyrender/test_void_teapot_frosted_front.py
	python3 libpyrender/test.py


clean:
	rm libpyrender/librender.so

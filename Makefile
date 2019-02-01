CXX = clang++
CXXFLAGS = -Wall -Werror -g --no-undefined -Ofast -march=native
SHAREDFLAGS = -fPIC -shared
LD_FLAGS = -lpthread -std=c++17

CACHE_LINE_SIZE = $(cat /sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size)

libpyrender/librender.so: src/render.cpp src/python_interface.cpp src/linalg.cpp src/octree.cpp src/render.h src/python_interface.h src/linalg.h src/octree.h
	$(CXX) $(CXXFLAGS) $(SHAREDFLAGS) -o libpyrender/librender.so src/octree.cpp src/render.cpp src/python_interface.cpp src/linalg.cpp $(LD_FLAGS)

render-tests: images/plane_teapot_frosted_front.png images/plane_teapot_refract_behind.png

images/plane_teacup_front.png: libpyrender/librender.so libpyrender/test_plane_teacup.py
	time python3 libpyrender/test_plane_teacup.py

images/plane_teapot_frosted_front.png: libpyrender/librender.so libpyrender/test_plane_teapot_frosted_front.py
	time python3 libpyrender/test_plane_teapot_frosted_front.py

images/plane_teapot_refract_behind.png: libpyrender/librender.so libpyrender/test_plane_teapot_refract_behind.py
	time python3 libpyrender/test_plane_teapot_refract_behind.py

clean:
	rm libpyrender/librender.so

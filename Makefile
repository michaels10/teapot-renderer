CXX = clang++
CXXFLAGS = -Wall -Werror -pg --no-undefined -Ofast -march=native
SHAREDFLAGS = -fPIC -shared
LD_FLAGS = -lpthread -std=c++2a

CACHE_LINE_SIZE = $(cat /sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size)

libpyrender/librender.so: src/scene.cpp src/python_interface.cpp src/linalg.cpp src/scene.h src/python_interface.h src/linalg.h 
	$(CXX) $(CXXFLAGS) $(SHAREDFLAGS) -o libpyrender/librender.so src/scene.cpp src/python_interface.cpp src/linalg.cpp $(LD_FLAGS)

render-tests: images/plane_teapot_frosted_front.png images/plane_teapot_refract_behind.png images/plane_teacup_front.png

benchmarks:
	mkdir benchmarks

images/plane_teacup_front.png: libpyrender/librender.so libpyrender/test_plane_teacup.py benchmarks
	time -o benchmarks/plane_teacup_front.txt python3 libpyrender/test_plane_teacup.py 

images/plane_teapot_frosted_front.png: libpyrender/librender.so libpyrender/test_plane_teapot_frosted_front.py benchmarks
	time -o benchmarks/plane_teapot_frosted_front.txt python3 libpyrender/test_plane_teapot_frosted_front.py 

images/plane_teapot_refract_behind.png: libpyrender/librender.so libpyrender/test_plane_teapot_refract_behind.py benchmarks
	time -o benchmarks/plane_teapot_refract_behind.txt python3 libpyrender/test_plane_teapot_refract_behind.py 

clean:
	rm libpyrender/librender.so


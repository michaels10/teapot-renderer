lib/render.so: src/render.cpp src/python_interface.cpp src/render.h src/python_interface.h src/linalg.h src/ray_queue.cpp src/ray_queue.h
	clang++ -fPIC --no-undefined -Ofast -shared -o lib/render.so src/render.cpp src/python_interface.cpp src/linalg.cpp src/ray_queue.cpp -std=c++17 -lpthread

clean:
	rm lib/render.so

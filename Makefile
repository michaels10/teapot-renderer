lib/render.so: src/render.cpp src/python_interface.cpp src/render.h src/python_interface.h src/linalg.h
	clang++ -fPIC --no-undefined -Ofast -shared -o lib/render.so src/render.cpp src/python_interface.cpp src/linalg.cpp -std=c++17

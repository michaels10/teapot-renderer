lib/render.so: src/render.cpp src/python_interface.cpp src/render.h src/python_interface.h src/linalg.h 
	clang++ -fPIC -Wall -Werror -g --no-undefined -Ofast -shared -o lib/render.so src/render.cpp src/python_interface.cpp src/linalg.cpp -std=c++17 -lpthread

clean:
	rm lib/render.so

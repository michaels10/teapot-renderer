RESIZE_SMALL_OBJS = resize_small.o

all : resize_small lib/render.so

resize_small : $(RESIZE_SMALL_OBJS)
	clang++ $(RESIZE_SMALL_OBJS) -Wall -Werror -std=c++1y -o resize_small

resize_small.o : resize_small.cpp
	clang++ -Wall -Werror -std=c++1y -c resize_small.cpp 

lib/render.so: src/render.cpp src/python_interface.cpp src/render.h src/python_interface.h
	clang++ -fPIC -Wall -Werror --no-undefined -Ofast -shared -o lib/render.so src/render.cpp src/python_interface.cpp -std=c++17

clean:
	rm lib/render.so
	-rm -f *.o .
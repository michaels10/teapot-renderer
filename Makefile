SQUISHSQUASH_OBJS = squishsquash.o

all : squishsquash lib/render.so

squishsquash : $(SQUISHSQUASH_OBJS)
	clang++ $(SQUISHSQUASH_OBJS) -wall -werror -std=c++1y -o squishsquash

squishsquash.o : squishsquash.cpp
	clang++ -wall -werror -std=c++1y -c squishsquash.cpp 

lib/render.so: src/render.cpp src/python_interface.cpp src/render.h src/python_interface.h
	clang++ -fPIC -wall -werror --no-undefined -Ofast -shared -o lib/render.so src/render.cpp src/python_interface.cpp -std=c++17

clean:
	rm lib/render.so
	-rm -f *.o .
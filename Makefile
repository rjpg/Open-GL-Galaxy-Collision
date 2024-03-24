FLAGS = -L/usr/X11R6/lib/ -lX11 -lglut -lGL -lGLU -lm

all: nbody.o 
	cc -o demo nbody.o ${FLAGS}

planet.o: nbody.c
	cc -c nbody.c

clean:  
	rm -rfv a.out *.o *~ demo

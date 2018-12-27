CC = gcc
CFLAGS = -g
DEPS = vec3d.h
OBJ = \
	camera.o \
	ciao.o \
	obj.o \
	quadtree.o \
	scene.o \
	vec3d.o
	

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

ciao: $(OBJ)
	gcc $(CFLAGS) -o $@ $^ -lm -lstdc++
  
clean: 
	rm *.o ciao

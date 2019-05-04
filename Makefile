CC = gcc
CFLAGS =
LDFLAGS = -lm -lstdc++ -lSDL2

src = $(wildcard *.cpp)
obj = $(src:.cpp=.o)
dep = $(obj:.o=.d)

r0: color.o render.o r0.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

ciao: camera.o ciao.o color.o hdr_image.o obj.o quadtree.o render.o scene.o vec3d.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

colortest: color.o colortest.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

-include $(dep)

%.d: %.c
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean
clean:
	rm -f $(obj) ciao

.PHONY: cleandep
cleandep:
	rm -f $(dep)

CC = gcc
CFLAGS =
LDFLAGS = -lm -lstdc++ -lSDL2

src = $(wildcard *.cpp)
obj = $(src:.cpp=.o)
dep = $(obj:.o=.d)

r0: render.o r0.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

ciao: $(obj)
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

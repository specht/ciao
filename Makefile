CC = gcc
CFLAGS =
LDFLAGS = -lm -lstdc++ -lSDL2

src = $(wildcard *.cpp)
obj = $(src:.cpp=.o)
dep = $(obj:.o=.d)

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

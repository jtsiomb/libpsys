src = $(wildcard *.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)
bin = simple

incdir = ../../src
libdir = ../..
liba = $(libdir)/libpsys.a

CFLAGS = -pedantic -Wall -g -I$(incdir)
LDFLAGS = -L$(libdir) $(liba) $(libgl) -lanim -limago -lm

ifeq ($(shell uname -s), Darwin)
	libgl = -framework OpenGL -framework GLUT
else
	libgl = -lGL -lGLU -lglut
endif

$(bin): $(obj) $(liba)
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

%.d: %.c
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:.o=.d) >$@

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

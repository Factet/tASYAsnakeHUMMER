# gui3.1
LIB = libgui3.a
SRCS = button.cc edit.cc GUI.cc image.cc list.cc scroll.cc text.cc window.cc
HEADERS = button.h context.h edit.h GUI.h image.h list.h mytypes.h scroll.h text.h window.h
OBJS = $(addprefix obj/,$(SRCS:.cc=.o))
CC = g++ -I./include
CFLAGS = -g `pkg-config --cflags gtk+-3.0` -std=c++11
LDFLAGS = `pkg-config --libs gtk+-3.0` # -fsanitize=leak

# examples
EXAMPLES = browse bspline calc clock myprog quad view

BROWSE_SRCS = browse.cc
BROWSE_OBJS = $(addprefix obj/,$(BROWSE_SRCS:.cc=.o))

BSPLINE_SRCS = bspline.cc bspline2d.cc
BSPLINE_OBJS = $(addprefix obj/,$(BSPLINE_SRCS:.cc=.o))

CALC_SRCS = calc.cc
CALC_OBJS = $(addprefix obj/,$(CALC_SRCS:.cc=.o))

CLOCK_SRCS = digit7.cc clock.cc
CLOCK_OBJS = $(addprefix obj/,$(CLOCK_SRCS:.cc=.o))

MYPROG_SRCS = myprog.cc
MYPROG_OBJS = $(addprefix obj/,$(MYPROG_SRCS:.cc=.o))

QUAD_SRCS = quad.cc
QUAD_OBJS = $(addprefix obj/,$(QUAD_SRCS:.cc=.o))

VIEW_SRCS = view.cc
VIEW_OBJS = $(addprefix obj/,$(VIEW_SRCS:.cc=.o))

all: $(LIB) $(EXAMPLES)

$(LIB): $(OBJS)
	ar -r -s $(LIB) $(OBJS)

obj/%.o: source/%.cc $(addprefix include/,$(HEADERS))
	$(CC) $(CFLAGS) -c $< -o $@

obj/%.o: examples/%.cc $(addprefix include/,$(HEADERS))
	$(CC) $(CFLAGS) -c $< -o $@

browse: $(LIB) $(BROWSE_OBJS)
	$(CC) $(BROWSE_OBJS) $(LIB) -o browse $(LDFLAGS)

bspline: $(LIB) $(BSPLINE_OBJS) examples/bspline2d.h
	$(CC) $(BSPLINE_OBJS) $(LIB) -o bspline $(LDFLAGS)

calc: $(LIB) $(CALC_OBJS)
	$(CC) $(CALC_OBJS) $(LIB) -o calc $(LDFLAGS)

clock: $(LIB) $(CLOCK_OBJS) examples/digit7.h
	$(CC) $(CLOCK_OBJS) $(LIB) -o clock $(LDFLAGS)

myprog: $(LIB) $(MYPROG_OBJS) examples/myprog.h
	$(CC) $(MYPROG_OBJS) $(LIB) -o myprog $(LDFLAGS)

quad: $(LIB) $(QUAD_OBJS)
	$(CC) $(QUAD_OBJS) $(LIB) -o quad  $(LDFLAGS)

view: $(LIB) $(VIEW_OBJS)
	$(CC) $(VIEW_OBJS) $(LIB) -o view $(LDFLAGS)

clean:
	rm $(LIB) $(EXAMPLES) obj/*
	
EXAMPLES = browse bspline calc clock myprog quad view
EXAMPLES = browse bspline calc clock myprog quad view snake

SNAKE_SRCS = snake.cc
SNAKE_OBJS = $(addprefix obj/,$(SNAKE_SRCS:.cc=.o))

snake: $(LIB) $(SNAKE_OBJS)
	$(CC) $(SNAKE_OBJS) $(LIB) -o snake $(LDFLAGS)


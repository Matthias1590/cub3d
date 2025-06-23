CFLAGS += -Wall -Wextra -Werror -ggdb
CFLAGS += -I./MLX42/include/MLX42
LDFLAGS += $(shell pkg-config --static --libs glfw3) -lm
RM += -r

SRCDIR := src
SRCS := $(wildcard $(SRCDIR)/*.c)

OBJDIR := obj
OBJS := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

MLX := MLX42/build/libmlx42.a
NAME := cub3d

all: $(NAME)

$(NAME): $(OBJS) $(MLX)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) obj

fclean: clean
	$(RM) $(NAME)

re: fclean all

$(MLX):
	cd MLX42 && cmake -DDEBUG=1 -B build && cmake --build build -j4

obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c -o $@ $^

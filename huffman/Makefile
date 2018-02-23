EXEC		=	huffman
CC			=	clang
CFLAGS		=	-Wall -Wextra -Werror -Ofast
SRC			=	main.c \
				binary_tree_functions.c \
				unpack.c \
				thread_print.c
OBJ			=	$(SRC:.c=.o)

$(EXEC): $(OBJ)
	$(info Compiling $(EXEC))
	@$(CC) $(CFLAGS) -o $@ $^ -I.

all: $(EXEC)

%.o: %.c
	$(info Compiling $< into $@ ...)
	@$(CC) $(CFLAGS) -o $@ -c $<

clean:
	$(info Cleaning .)
	rm -f $(OBJ)
	$(info Done !)

fclean: clean
	$(info Cleaning .)
	@rm -rf $(EXEC)
	$(info Done !)

re: fclean all

.PHONY: all clean fclean re

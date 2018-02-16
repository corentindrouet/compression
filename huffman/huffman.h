#ifndef __HUFFMAN__
# define __HUFFMAN__
# include <stdio.h>
# include <sys/mman.h>
# include <fcntl.h>
# include <unistd.h>
# include <strings.h>
# include <stdlib.h>

# define false 0
# define true 1

typedef struct              s_binary_tree {
    int                     value;
    unsigned char                    character;
    struct s_binary_tree    *parent;
    struct s_binary_tree    *left;
    struct s_binary_tree    *right;
}                           t_binary_tree;

void    create_element_at_addr(int value, unsigned char character, t_binary_tree *sheet, t_binary_tree *left, t_binary_tree *right);
void    *convert_buff_to_binary_tree(int *buff, int *index, int buff_size);
void    write_compressed_datas_on_fd(void *tree, void *mmap_addr, int size, int fd);
int     file_size(int fd);
void    tri(int *tab, int tab_size, int *index);

#endif

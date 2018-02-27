#ifndef HUFFMAN_H
# define HUFFMAN_H
# include <stdio.h>
# include <sys/mman.h>
# include <fcntl.h>
# include <unistd.h>
# include <strings.h>
# include <string.h>
# include <stdlib.h>
# include <math.h>
# include <pthread.h>

# define false 0
# define true 1

typedef struct              s_binary_tree {
    int                     value;
    unsigned char                    character;
    struct s_binary_tree    *parent;
    struct s_binary_tree    *left;
    struct s_binary_tree    *right;
}                           t_binary_tree;

typedef struct 				s_pourcent_datas {
	int 					*size;
	int 					*i;
} 							t_pourcent_datas;

typedef struct 				s_thread_options {
	char 					pack_or_unpack;
	char 					recurrence;
	char 					base_sheet;
	char 					branchs;
	char 					compress_datas;
	char 					write_datas;
	t_pourcent_datas 		actual;
} 							t_thread_options;

pthread_cond_t ret;
pthread_cond_t start;
pthread_cond_t end;
t_thread_options options;
pthread_mutex_t mutex;// = PTHREAD_MUTEX_INITIALIZER;

void	create_element_at_addr(int value, unsigned char character, t_binary_tree *sheet, t_binary_tree *left, t_binary_tree *right);
void	*convert_buff_to_binary_tree(int *buff, int *index, int buff_size);
void	write_compressed_datas_on_fd(void *tree, void *mmap_addr, int size, int fd);
int		file_size(int fd);
void	tri(int *tab, int tab_size, int *index);
void	decrypt(t_binary_tree *bin_tree, void *mmaped_file, int size, int fd);
void	write_tree_base_on_file(t_binary_tree *tree, int fd);
void	*recover_binary_tree(void *base);
void	*thread_print(void *arg);
char	get_opts_value(int index);

#endif

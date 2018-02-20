#include "huffman.h"

void decrypt(t_binary_tree *bin_tree, void *mmaped_file, int size, int fd) {
	int i;
	int final_size;
	char c;
	int	decal;
	char next;
	int nb_byte_writed;
	t_binary_tree	*tmp_tree;
//	char *mmap_decrypt;
//	int offset_mmap_d;

	final_size = *(int*)mmaped_file;
	i = 0;
//	mmap_decrypt = mmap(0, final_size, 3, 34, -1, 0);
//	offset_mmap_d = final_size - 1;
	/*
	 *	Here we are looking for the root of our binary tree
	 */
	while (bin_tree->parent || !bin_tree->value)
		bin_tree++;
	i = size - 2;
	decal = *(char*)(mmaped_file + size - 1);
	nb_byte_writed = 0;
	c = *(char*)(mmaped_file + i);
	while (i >= 4 && nb_byte_writed < final_size) {
		tmp_tree = bin_tree;
		next = 1;
		while (next) {
			if (decal == 8) {
				i--;
				c = *(char*)(mmaped_file + i);
				decal = 0;
			}
			if (*(&(tmp_tree->left) + (c & 1))) {
				tmp_tree = *(&(tmp_tree->left) + (c & 1));
				c = c >> 1;
				decal++;
			} else {
//				printf("writed:%c ", tmp_tree->character);
				write(fd, &(tmp_tree->character), 1);
				nb_byte_writed++;
				next = 0;
			}
//			printf("%p c:%x - decal:%d\n", tmp_tree, c & 1, decal);
		}
	}
	return ;
}

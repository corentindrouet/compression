#include "huffman.h"

void    create_element_at_addr(int value, unsigned char character, t_binary_tree *sheet, t_binary_tree *left, t_binary_tree *right) {
    if (!sheet)
        return ;
    sheet->value = value;
    sheet->character = character;
    sheet->left = left;
    sheet->right = right;
    sheet->parent = NULL;
}

void 	check_nb_no_parent(t_binary_tree *tree, int addr_offset) {
	t_binary_tree 	*end;
	int 	nb_parent;

	nb_parent = 0;
	end = tree + (addr_offset / sizeof(t_binary_tree));
	while (tree < end) {
		if (!(tree->parent) && tree->value) {
			nb_parent++;
		}
		tree++;
	}
	printf("%d\n", nb_parent);
	return ;
}

void    *convert_buff_to_binary_tree(int *buff, int *index, int buff_size) {
    int     i;
    void    *binary_tree_addr;
    int     addr_offset;
    int     sizeof_t_binary_tree;
    t_binary_tree    *tmp_branch;
    t_binary_tree    *tmp_branch_match;
    t_binary_tree    *next_overflow_value;
    char    element_created;
	int max_value;
	int offset;
	char progress[11];

    sizeof_t_binary_tree = sizeof(t_binary_tree);
    binary_tree_addr = malloc((sizeof_t_binary_tree * buff_size) * 2);
    if (!binary_tree_addr)
        return (NULL);
    i = 0;
    addr_offset = 0;
	offset = 1;
	progress[10] = 0;
	memset(progress, ' ', 10);
	printf("\tCreating base sheet: [%s]\r", progress);
	fflush(stdout);
    while (i < buff_size) {
		if (i > ((buff_size * (offset * 10)) / 100)) {
			progress[offset - 1] = '=';
			printf("\tCreating base sheet: [%s]\r", progress);
			fflush(stdout);
			offset++;
		}
        create_element_at_addr(buff[i], index[i], (t_binary_tree*)(binary_tree_addr + addr_offset), NULL, NULL);
        i++;
        addr_offset += sizeof_t_binary_tree;
	}
	progress[offset - 1] = '=';
	printf("\tCreating base sheet: [%s]\r", progress);
	fflush(stdout);
	printf("\n");
	fflush(stdout);
	i = 0;
	max_value = 0;
	while (i < buff_size) {
		max_value += buff[i];
		i++;
	}
	tmp_branch = (t_binary_tree*)(binary_tree_addr);
	memset(progress, ' ', 10);
	printf("\tCreating branchs: [%s]\r", progress);
	fflush(stdout);
	offset = 1;
	while (tmp_branch->value != max_value) {
		if (tmp_branch->value > ((max_value * (offset * 10)) / 100)) {
			progress[offset - 1] = '=';
			printf("\tCreating base sheet: [%s]\r", progress);
			fflush(stdout);
			offset++;
		}
		if (tmp_branch->parent) {
			tmp_branch++;
			continue;
		}
		element_created = false;
		next_overflow_value = NULL;
		tmp_branch_match = binary_tree_addr;
		while (tmp_branch_match < (t_binary_tree*)(binary_tree_addr + addr_offset) && !element_created) {
			if ((tmp_branch_match != tmp_branch) && tmp_branch_match->value && !tmp_branch_match->parent && (tmp_branch_match->value <= tmp_branch->value)) {
				create_element_at_addr(tmp_branch_match->value + tmp_branch->value, 0, (t_binary_tree*)(binary_tree_addr + addr_offset), tmp_branch_match, tmp_branch);
				tmp_branch_match->parent = (t_binary_tree*)(binary_tree_addr + addr_offset);
				tmp_branch->parent = (t_binary_tree*)(binary_tree_addr + addr_offset);
				addr_offset += sizeof_t_binary_tree;
				element_created = true;
			} else if (tmp_branch_match->value && !tmp_branch_match->parent && tmp_branch_match->value > tmp_branch->value) {
				if (!next_overflow_value || (next_overflow_value->value > tmp_branch_match->value))
					next_overflow_value = tmp_branch_match;
			}
			tmp_branch_match++;
			//                    tmp_branch_match += sizeof_t_binary_tree;
		}
		if (!element_created && next_overflow_value) {
			create_element_at_addr(next_overflow_value->value + tmp_branch->value, 0, (t_binary_tree*)(binary_tree_addr + addr_offset), next_overflow_value, tmp_branch);
			next_overflow_value->parent = (t_binary_tree*)(binary_tree_addr + addr_offset);
			tmp_branch->parent = (t_binary_tree*)(binary_tree_addr + addr_offset);
			addr_offset += sizeof_t_binary_tree;
		}
		next_overflow_value = (t_binary_tree*)(binary_tree_addr);
		while (next_overflow_value < (t_binary_tree*)(binary_tree_addr + addr_offset)) {
			if (!next_overflow_value->parent && next_overflow_value->value < tmp_branch->value)
				tmp_branch = next_overflow_value;
			next_overflow_value++;
		}
	}
	memset(progress, '=', 10);
//	progress[offset - 1] = '=';
	printf("\tCreating base sheet: [%s]\n", progress);
	fflush(stdout);
	printf("\tTree size: %d\n", addr_offset);
	check_nb_no_parent(binary_tree_addr, addr_offset);
    return (binary_tree_addr);
}

void    write_compressed_datas_on_fd(void *tree, void *mmap_addr, int size, int fd) {
    int             i;
    char            packed_char;
    int             decal;
    t_binary_tree   *tmp_tree;
	char 			progress[11];
	int 			offset;

	progress[10] = 0;
	memset(progress, ' ', 10);
    i = size - 1;
    decal = 0;
    packed_char = 0;
	write(fd, &size, 4);
	printf("Writing on file: [%s]\r", progress);
	fflush(stdout);
	offset = 0;
    while (i >= 0) {
		if (i < ((size * ((10 - offset) * 10)) / 100)) {
			progress[offset - 1] = '=';
			printf("Writing on file: [%s]\r", progress);
			fflush(stdout);
			offset++;
		}
        tmp_tree = tree;
//		printf("%d\n", *(unsigned char*)(mmap_addr + i));
        while (tmp_tree->character != *(unsigned char*)(mmap_addr + i))
            tmp_tree++;
        while (tmp_tree->parent) {
            if (decal == 8) {
                write(fd, &packed_char, 1);
                decal = 0;
                packed_char = 0;
            }
            packed_char = packed_char << 1;
            decal++;
            packed_char += (tmp_tree->parent->left == tmp_tree) ? 0 : 1;
            tmp_tree = tmp_tree->parent;
        }
//		printf("%p\n", tmp_tree);
        i--;
    }
	write(fd, &packed_char, 1);
	packed_char = 8 - decal;
	write(fd, &packed_char, 1);
	progress[offset - 1] = '=';
	printf("Writing on file: [%s]\n", progress);
}

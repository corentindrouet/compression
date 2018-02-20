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

void    *convert_buff_to_binary_tree(int *buff, int *index, int buff_size) {
    int     i;
    int     tmp_i;
    void    *binary_tree_addr;
    int     addr_offset;
    int     sizeof_t_binary_tree;
    t_binary_tree    *tmp_branch;
    t_binary_tree    *tmp_branch_match;
    t_binary_tree    *next_overflow_value;
    char    element_created;

    sizeof_t_binary_tree = sizeof(t_binary_tree);
    binary_tree_addr = malloc((sizeof_t_binary_tree * buff_size) * 2);
    if (!binary_tree_addr)
        return (NULL);
    i = 0;
    addr_offset = 0;
    while (i < buff_size) {
        create_element_at_addr(buff[i], index[i], (t_binary_tree*)(binary_tree_addr + addr_offset), NULL, NULL);
        i++;
        addr_offset += sizeof_t_binary_tree;
    }
    i = 1;
    while (i <= buff[buff_size - 1]) {
        tmp_i = 0;
        while (tmp_i < addr_offset) {
            tmp_branch = (t_binary_tree*)(binary_tree_addr + tmp_i);
            if (tmp_branch->value == i && !tmp_branch->parent) {
                tmp_branch_match = (t_binary_tree*)(binary_tree_addr);
                element_created = false;
                next_overflow_value = NULL;
                while (tmp_branch_match < (t_binary_tree*)(binary_tree_addr + addr_offset) && !element_created) {
                    if (tmp_branch_match != tmp_branch && tmp_branch_match->value && !tmp_branch_match->parent && tmp_branch_match->value <= tmp_branch->value) {
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
            }
            tmp_i += sizeof_t_binary_tree;
        }
        i++;
    }
    return (binary_tree_addr);
}

void    write_compressed_datas_on_fd(void *tree, void *mmap_addr, int size, int fd) {
    int             i;
    char            packed_char;
    int             decal;
    t_binary_tree   *tmp_tree;

    i = size - 1;
    decal = 0;
    packed_char = 0;
	write(fd, &size, 4);
    while (i >= 0) {
        tmp_tree = tree;
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
        i--;
    }
	write(fd, &packed_char, 1);
	packed_char = 8 - decal;
	write(fd, &packed_char, 1);
}

#include "huffman.h"

int file_size(int fd) {
    int size;

    lseek(fd, 0, 0);
    size = lseek(fd, 0, 2);
    lseek(fd, 0, 0);
    return (size);
}

void tri(int *tab, int tab_size, int *index) {
    int i;

    i = 0;
    tab_size--;
    while (i < tab_size) {
        if (tab[i] > tab[i + 1]) {
            tab[i] += tab[i + 1];
            tab[i + 1] = tab[i] - tab[i + 1];
            tab[i] -= tab[i + 1];
            index[i] += index[i + 1];
            index[i + 1] = index[i] - index[i + 1];
            index[i] -= index[i + 1];
            i = 0;
        } else {
            i++;
        }
    }
    return ;
}

void    find_0(t_binary_tree *tree) {
    int i = 0;

    while (tree->character != 0)
        tree++;
    printf("value left: %d\n value right: %d\n")
/*    while (tree->parent) {
        printf("step %d\n", i);
        i++;
        tree = tree->parent;
    }*/
}

int main(int argc, char **argv) {
    int fd;
    int fd_pack;
    int size;
    char *mmap_addr;
    void *bin_tree;
    int index[256];
    int buff[256];
    int i;

    if (argc != 3)
        return (0);
    fd = open(argv[1], O_RDWR);
    if (fd == -1)
        return (0);
    fd_pack = open(argv[2], O_RDWR | O_TRUNC | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd_pack == -1)
        return (0);
    size = file_size(fd);
    mmap_addr = mmap(0, size, 3, 2, fd, 0);
    if (!mmap_addr)
        return (0);
    i = 0;
    bzero(buff, sizeof(int) * 256);
    while (i < size) {
        buff[(unsigned char)mmap_addr[i]]++;
        i++;
    }
    i = 0;
    while (i < 256) {
        index[i] = i;
        i++;
    }
    tri(buff, 256, index);
    i = 0;
    while (i < 256) {
        printf("%4d: %d - %d\n", index[i], buff[i], (int)((double)((double)(buff[i] * 100) / size) * 10) + 1);
        buff[i] = (int)((double)((double)(buff[i] * 100) / size) * 10) + 1;
        i++;
    }
    printf("%lu\n", sizeof(t_binary_tree));
    bin_tree = convert_buff_to_binary_tree(buff, index, 256);
    find_0(bin_tree);
    write_compressed_datas_on_fd(bin_tree, mmap_addr, size, fd_pack);
    free(bin_tree);
    munmap(mmap_addr, size);
    close(fd);
    close(fd_pack);
    return (0);
}

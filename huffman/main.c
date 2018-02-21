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

void    find_0(t_binary_tree *tree, unsigned char byte) {
    int i = 0;

	printf("%p %p\n", (&(tree->left)), (&(tree->left) + 1));
    while (tree->character != byte)
        tree++;
    while (tree->parent) {
        i++;
        tree = tree->parent;
    }
	printf("steps %d\n", i);
}

void *create_binary_tree(char *mmap_addr, int size) {
	void *tree;
    int index[256];
    int buff[256];
	int i;

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
//	i = 0;
//    while (i < 256) {
//		if (buff[i]) {
//			printf("buff[%d]: %f%% ", i, (double)((double)(buff[i] * 100) / size));
//			buff[i] = (int)round((double)((double)(buff[i] * 100) / size) * 100) + 1;
//			printf("- %d\n", buff[i]);
//		}
//        i++;
//    }
    tree = convert_buff_to_binary_tree(buff, index, 256);
	return (tree);
}

void pack(char *file_to_pack, char *destination) {
    int fd;
    int fd_pack;
    int size;
    char *mmap_addr;
    void *bin_tree;

    fd = open(file_to_pack, O_RDWR);
	printf("Opening %s . . .\n", file_to_pack);
    if (fd == -1) {
		printf("Can't open %s\n", file_to_pack);
        return ;
	}
	printf("Opening %s . . .\n", destination);
    fd_pack = open(destination, O_RDWR | O_TRUNC | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd_pack == -1) {
		printf("Can't open %s\n", destination);
        return ;
	}
    size = file_size(fd);
    mmap_addr = mmap(0, size, 3, 2, fd, 0);
// WAS HERE
	printf("Creating binary tree:\n");
	bin_tree = create_binary_tree(mmap_addr, size);
    write_compressed_datas_on_fd(bin_tree, mmap_addr, size, fd_pack);
    free(bin_tree);
    munmap(mmap_addr, size);
    close(fd);
    close(fd_pack);
    return ;
}

void unpack(char *file_name, char *compressed_file, char *new_file) {
	int fd_base;
	int fd_packed;
	int fd_unpacked;
    int size;
    char *mmap_addr;
    void *bin_tree;

    fd_base = open(file_name, O_RDWR);
    if (fd_base == -1) {
		printf("Can't open %s\n", file_name);
        return ;
	}
    fd_packed = open(compressed_file, O_RDWR);
    if (fd_packed == -1) {
		printf("Can't open %s\n", compressed_file);
        return ;
	}
    fd_unpacked = open(new_file, O_RDWR | O_TRUNC | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd_unpacked == -1) {
		printf("Can't open %s\n", new_file);
        return ;
	}
    size = file_size(fd_base);
    mmap_addr = mmap(0, size, 3, 2, fd_base, 0);
// WAS HERE
	bin_tree = create_binary_tree(mmap_addr, size);
    munmap(mmap_addr, size);
    close(fd_base);
	size = file_size(fd_packed);
	mmap_addr = mmap(0, size, 3, 2, fd_packed, 0);
	decrypt(bin_tree, mmap_addr, size, fd_unpacked);
    munmap(mmap_addr, size);
//    write_compressed_datas_on_fd(bin_tree, mmap_addr, size, fd_pack);
    free(bin_tree);
    close(fd_packed);
    close(fd_unpacked);
    return ;
}

int main(int argc, char **argv) {
    if (argc == 3) {
		pack(argv[1], argv[2]);
        return (0);
	} else if (argc == 4) {
		unpack(argv[1], argv[2], argv[3]);
	} else {
		printf("Not enough arguments");
	}
	return (0);
}

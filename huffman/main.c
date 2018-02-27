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
	pthread_t thread;
	void *mmap_tmp;
	void *mmap_max;
//	char progress[11];
//	int offset;
//	int pourcent;

    if (!mmap_addr)
        return (0);
    i = 0;
    bzero(buff, sizeof(int) * 256);
//	memset(progress, ' ', 10);
//	progress[10] = 0;
//	printf("\tCounting Bytes recurrence: [%s]\r", progress);
//	fflush(stdout);
//	offset = 1;
//	pourcent = (int)((long)((long)size * (long)(offset * 10)) / 100);
	options.actual.size = &size;
	options.actual.i = &i;
	options.recurrence = 1;
	pthread_create(&thread, NULL, thread_print, (void*)&options);
	pthread_mutex_lock(&mutex);
	pthread_cond_wait(&start, &mutex);
	pthread_mutex_unlock(&mutex);
	mmap_tmp = mmap_addr;
	mmap_max = mmap_addr + size;
    while (mmap_tmp < mmap_max) {
		buff[*(unsigned char*)mmap_tmp]++;
		mmap_tmp++;
	}
	pthread_mutex_lock(&mutex);
	options.recurrence = 2;
	pthread_mutex_unlock(&mutex);
	pthread_join(thread, NULL);
//	pthread_cond_signal(&end);
//	progress[offset - 1] = '=';
//	printf("\tCounting Bytes recurrence: [%s] %d/%d\n", progress, i, size);
//	fflush(stdout);
    i = 0;
    while (i < 256) {
        index[i] = i;
        i++;
    }
    tri(buff, 256, index);
    tree = convert_buff_to_binary_tree(buff, index, 256);
	return (tree);
}

void pack(char *file_to_pack, char *destination) {
    int fd;
    int fd_pack;
    int size;
    char *mmap_addr;
    void *bin_tree;

    fd = open(file_to_pack, O_RDONLY);
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
    mmap_addr = mmap(0, size, PROT_READ, 2, fd, 0);
// WAS HERE
//	printf("Creating binary tree:\n");
	bin_tree = create_binary_tree(mmap_addr, size);
//	printf("Compress and Write datas:\n");
    write_compressed_datas_on_fd(bin_tree, mmap_addr, size, fd_pack);
	munmap(bin_tree, (sizeof(t_binary_tree) * 256) * 2);
//    free(bin_tree);
    munmap(mmap_addr, size);
    close(fd);
    close(fd_pack);
//	printf("Datas successfully compressed !\n");
    return ;
}

void unpack(char *file_name, char *compressed_file, char *new_file) {
//	int fd_base;
	int fd_packed;
	int fd_unpacked;
    int size;
    char *mmap_addr;
    void *bin_tree;

	(void)file_name;
/*    fd_base = open(file_name, O_RDONLY);
	printf("Opening %s . . .\n", file_name);
    if (fd_base == -1) {
		printf("Can't open %s\n", file_name);
        return ;
	}*/
    fd_packed = open(compressed_file, O_RDONLY);
	printf("Opening %s . . .\n", compressed_file);
    if (fd_packed == -1) {
		printf("Can't open %s\n", compressed_file);
        return ;
	}
    fd_unpacked = open(new_file, O_RDWR | O_TRUNC | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
	printf("Opening %s . . .\n", new_file);
    if (fd_unpacked == -1) {
		printf("Can't open %s\n", new_file);
        return ;
	}
//    size = file_size(fd_base);
//    mmap_addr = mmap(0, size, PROT_READ, 2, fd_base, 0);
// WAS HERE
//	printf("Creating binary tree:\n");
//	bin_tree = recover_binary_tree(mmap_addr)//create_binary_tree(mmap_addr, size);
//    munmap(mmap_addr, size);
//    close(fd_base);
	size = file_size(fd_packed);
	mmap_addr = mmap(0, size, 3, 2, fd_packed, 0);
	bin_tree = recover_binary_tree(mmap_addr + 4);
//	printf("Decompress and write datas:\n");
	decrypt(bin_tree, mmap_addr, size, fd_unpacked);
    munmap(mmap_addr, size);
//    write_compressed_datas_on_fd(bin_tree, mmap_addr, size, fd_pack);
	munmap(bin_tree, (sizeof(t_binary_tree) * 256) * 2);
    //free(bin_tree);
    close(fd_packed);
    close(fd_unpacked);
//	printf("Datas successfully decompressed !\n");
    return ;
}

void init_opts(int p_o_up) {
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&start, NULL);
	pthread_cond_init(&end, NULL);
	pthread_cond_init(&ret, NULL);
	options.pack_or_unpack = p_o_up;
	options.recurrence = 0;
	options.base_sheet = 0;
	options.branchs = 0;
	options.compress_datas = 0;
	options.write_datas = 0;
	options.actual.size = 0;
	options.actual.i = 0;
}

int main(int argc, char **argv) {

    if (argc == 4) {
		if (!strcmp(argv[1], "pack")) {
			init_opts(1);
//			pthread_create(&thread, NULL, thread_print, (void*)&options);
//			pthread_mutex_lock(&mutex);
//			pthread_cond_wait(&end, &mutex);
//			pthread_mutex_unlock(&mutex);
			pack(argv[2], argv[3]);
//			pthread_join(thread, NULL);
			return (0);
		}
		else if (!strcmp(argv[1], "unpack")) {
			init_opts(0);
//			pthread_create(&thread, NULL, thread_print, (void*)&options);
//			pthread_mutex_lock(&mutex);
///			pthread_cond_wait(&end, &mutex);
//			pthread_mutex_unlock(&mutex);
			unpack(argv[2], argv[2], argv[3]);
//			pthread_join(thread, NULL);
			return (0);
		}
	}
	printf("Invalids arguments\n");
	printf("Usage:");
	printf("\tCompression: %s pack <file_to_compress> <compressed_file_path>\n", argv[0]);
	printf("\tDecompression: %s unpack <compressed_file_path> <decompressed_file_path>\n", argv[0]);
	return (0);
}

#include "huffman.h"

void 	*recover_binary_tree(void *base) {
    int     i;
    void    *binary_tree_addr;
    int     addr_offset;
    int     sizeof_t_binary_tree;
    t_binary_tree    *tmp_branch;
    t_binary_tree    *tmp_branch_match;
    t_binary_tree    *next_overflow_value;
    char    element_created;
	int max_value;
//	int offset;
//	char progress[11];
//	int pourcent;
	short nb_branchs;
	int total;
	pthread_t thread;

    sizeof_t_binary_tree = sizeof(t_binary_tree);
    binary_tree_addr = mmap(0, (sizeof_t_binary_tree * 256) * 2, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);//malloc((sizeof_t_binary_tree * buff_size) * 2);
    if (!binary_tree_addr)
        return (NULL);
    i = 0;
	total = 256;
    addr_offset = 0;
	options.actual.size = &total;
	options.actual.i = &i;
	options.base_sheet = 1;
	pthread_create(&thread, NULL, thread_print, (void*)&options);
	pthread_mutex_lock(&mutex);
	pthread_cond_wait(&start, &mutex);
	pthread_mutex_unlock(&mutex);
//	offset = 1;
//	progress[10] = 0;
//	memset(progress, ' ', 10);
//	printf("\tCreating base sheet: [%s]\r", progress);
//	fflush(stdout);
//	pourcent = (int)((long)((long)256 * (long)(offset * 10)) / 100);
	max_value = 0;
	nb_branchs = *(short*)base;
	base += 2;
    while (i < total) {
//		if (i > pourcent) {
//			progress[offset - 1] = '=';
//			printf("\tCreating base sheet: [%s]\r", progress);
//			fflush(stdout);
//			offset++;
//			pourcent = (int)((long)((long)256 * (long)(offset * 10)) / 100);
//		}
		if (i < (total - nb_branchs)) {
			create_element_at_addr(0, 0, (t_binary_tree*)(binary_tree_addr + addr_offset), NULL, NULL);
		} else {
			create_element_at_addr(*(int*)base, *(unsigned char*)(base + 4), (t_binary_tree*)(binary_tree_addr + addr_offset), NULL, NULL);
			max_value += *(int*)base;
			base += 5;
		}
        i++;
        addr_offset += sizeof_t_binary_tree;
	}
	pthread_mutex_lock(&mutex);
	options.base_sheet = 2;
	pthread_mutex_unlock(&mutex);
	pthread_join(thread, NULL);
//	progress[offset - 1] = '=';
//	printf("\tCreating base sheet: [%s]\n", progress);
//	fflush(stdout);
	tmp_branch = (t_binary_tree*)(binary_tree_addr);
	options.actual.size = &max_value;
	options.actual.i = &(tmp_branch->value);
	options.branchs = 1;
	pthread_create(&thread, NULL, thread_print, (void*)&options);
	pthread_mutex_lock(&mutex);
	pthread_cond_wait(&start, &mutex);
	pthread_mutex_unlock(&mutex);
//	memset(progress, ' ', 10);
//	printf("\tCreating branchs: [%s]\r", progress);
//	fflush(stdout);
//	offset = 1;
//	pourcent = (int)((long)((long)max_value * (long)(offset * 10)) / 100);
	while (tmp_branch->value != max_value) {
//		if (tmp_branch->value > pourcent) {
//			progress[offset - 1] = '=';
//			printf("\tCreating branchs: [%s]\r", progress);
//			fflush(stdout);
//			offset++;
//			pourcent = (int)((long)((long)max_value * (long)(offset * 10)) / 100);
//		}
		if (!tmp_branch->value || tmp_branch->parent) {
			tmp_branch++;
			continue;
		}
		options.actual.i = &(tmp_branch->value);
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
	pthread_mutex_lock(&mutex);
	options.branchs = 2;
	pthread_mutex_unlock(&mutex);
	pthread_join(thread, NULL);
//	memset(progress, '=', 10);
//	progress[offset - 1] = '=';
//	printf("\tCreating branchs: [%s]\n", progress);
//	fflush(stdout);
//	printf("\tTree size: %d\n", addr_offset);
//	check_nb_no_parent(binary_tree_addr, addr_offset);
    return (binary_tree_addr);
}

void decrypt(t_binary_tree *bin_tree, void *mmaped_file, int size, int fd) {
	int i;
	int final_size;
	char c;
	int	decal;
	char next;
	int nb_byte_writed;
	t_binary_tree	*tmp_tree;
//	int offset;
//	char progress[11];
//	int pourcent;
	char *mmap_tmp;
	void *mmap_start;
	pthread_t thread;

	final_size = *(int*)mmaped_file;
	mmap_tmp = mmap(0, final_size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	mmap_start = mmap_tmp;
	i = 0;
//	progress[10] = 0;
//	memset(progress, ' ', 10);
	/*
	 *	Here we are looking for the root of our binary tree
	 */
/*	i = 0;
	while (i < 256) {
		printf("value %d, char %d\n", bin_tree->value, bin_tree->character);
		i++;
	}*/
	while (bin_tree->parent || !bin_tree->value)
		bin_tree++;
	i = size - 2;
	decal = *(char*)(mmaped_file + size - 1);
	nb_byte_writed = 0;
	c = *(char*)(mmaped_file + i);
	options.actual.size = &final_size;
	options.actual.i = &nb_byte_writed;
	options.compress_datas = 1;
	pthread_create(&thread, NULL, thread_print, (void*)&options);
	pthread_mutex_lock(&mutex);
	pthread_cond_wait(&start, &mutex);
	pthread_mutex_unlock(&mutex);
//	printf("\tDecompressing bytes: [%s] %d/%d\r", progress, nb_byte_writed, final_size);
//	fflush(stdout);
//	pourcent = (int)((long)((long)final_size * (long)(offset * 10)) / 100);
	while (i >= 4 && nb_byte_writed < final_size) {
//		if (nb_byte_writed > pourcent) {
//			progress[offset - 1] = '=';
//			printf("\tDecompressing bytes: [%s] %d/%d\r", progress, nb_byte_writed, final_size);
//			fflush(stdout);
//			offset++;
//			pourcent = (int)((long)((long)final_size * (long)(offset * 10)) / 100);
//		}
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
				*mmap_tmp = tmp_tree->character;
				mmap_tmp++;
//				write(fd, &(tmp_tree->character), 1);
				nb_byte_writed++;
				next = 0;
			}
		}
	}
	pthread_mutex_lock(&mutex);
	options.compress_datas = 2;
	pthread_mutex_unlock(&mutex);
	pthread_join(thread, NULL);

	options.write_datas = 1;
	pthread_create(&thread, NULL, thread_print, (void*)&options);
	pthread_mutex_lock(&mutex);
	pthread_cond_wait(&start, &mutex);
	pthread_mutex_unlock(&mutex);
//	progress[offset - 1] = '=';
//	printf("\tDecompressing bytes: [%s] %d/%d\n", progress, nb_byte_writed, final_size);
//	fflush(stdout);
//	printf("\tWriting decompressed datas on file: ..\r");
//	fflush(stdout);
	write(fd, mmap_start, final_size);
	munmap(mmap_start, final_size);
	pthread_mutex_lock(&mutex);
	options.write_datas = 2;
	pthread_mutex_unlock(&mutex);
	pthread_mutex_lock(&mutex);
	pthread_cond_signal(&ret);
	pthread_mutex_unlock(&mutex);
	pthread_join(thread, NULL);
//	printf("\tWriting decompressed datas on file: ok\n");
	return ;
}

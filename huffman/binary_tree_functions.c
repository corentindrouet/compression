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

void 	check_nb_no_parent(t_binary_tree *tree) {
	int i;

	i = 0;
	while (i < 256) {
		printf("value: %d\n", tree->value);
		tree++;
		i++;
	}
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
	pthread_t thread;
	//	int offset;
	//	char progress[11];
	//	int pourcent;

	sizeof_t_binary_tree = sizeof(t_binary_tree);
	binary_tree_addr = mmap(0, (sizeof_t_binary_tree * buff_size) * 2, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);//malloc((sizeof_t_binary_tree * buff_size) * 2);
	if (!binary_tree_addr)
		return (NULL);
	i = 0;
	addr_offset = 0;
	//	offset = 1;
	//	progress[10] = 0;
	//	memset(progress, ' ', 10);
	//	printf("\tCreating base sheet: [%s]\r", progress);
	//	fflush(stdout);
	//	pourcent = (int)((long)((long)buff_size * (long)(offset * 10)) / 100);
	options.actual.size = &buff_size;
	options.actual.i = &i;
	options.base_sheet = 1;
	pthread_create(&thread, NULL, thread_print, (void*)&options);
	pthread_mutex_lock(&mutex);
	pthread_cond_wait(&start, &mutex);
	pthread_mutex_unlock(&mutex);
	while (i < buff_size) {
		//		if (i > pourcent) {
		//			progress[offset - 1] = '=';
		//			printf("\tCreating base sheet: [%s]\r", progress);
		//			fflush(stdout);
		//			offset++;
		//			pourcent = (int)((long)((long)buff_size * (long)(offset * 10)) / 100);
		//		}
		create_element_at_addr(buff[i], index[i], (t_binary_tree*)(binary_tree_addr + addr_offset), NULL, NULL);
		i++;
		addr_offset += sizeof_t_binary_tree;
	}
	pthread_mutex_lock(&mutex);
	options.base_sheet = 2;
	pthread_mutex_unlock(&mutex);
	pthread_join(thread, NULL);
	//	progress[offset - 1] = '=';
	//	printf("\tCreating base sheet: [%s]\r", progress);
	//	fflush(stdout);
	//	printf("\n");
	//	fflush(stdout);
	i = 0;
	max_value = 0;
	while (i < buff_size) {
		max_value += buff[i];
		i++;
	}
	tmp_branch = (t_binary_tree*)(binary_tree_addr);
	//	memset(progress, ' ', 10);
	//	printf("\tCreating branchs: [%s]\r", progress);
	//	fflush(stdout);
	//	offset = 1;
	options.actual.size = &max_value;
	options.actual.i = &(tmp_branch->value);
	options.branchs = 1;
	pthread_create(&thread, NULL, thread_print, (void*)&options);
	pthread_mutex_lock(&mutex);
	pthread_cond_wait(&start, &mutex);
	pthread_mutex_unlock(&mutex);
	//	pourcent = (int)((long)((long)max_value * (long)(offset * 10)) / 100);
	while (tmp_branch->value != max_value) {
		if (!tmp_branch->value || tmp_branch->parent) {
			tmp_branch++;
			continue;
		}
		options.actual.i = &(tmp_branch->value);
		//		if (tmp_branch->value > pourcent) {
		//			progress[offset - 1] = '=';
		//			printf("\tCreating branchs: [%s]\r", progress);
		//			fflush(stdout);
		//			offset++;
		//			pourcent = (int)((long)((long)max_value * (long)(offset * 10)) / 100);
		//		}
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
	//	check_nb_no_parent(binary_tree_addr);
	return (binary_tree_addr);
}

void    write_compressed_datas_on_fd(void *tree, void *mmap_addr, int size, int fd) {
	int             i;
	char            packed_char;
	int             decal;
	t_binary_tree   *tmp_tree;
	//	char 			progress[11];
	//	int 			offset;
	//	int pourcent;
	char *mmap_tmp;
	void *mmap_start;
	pthread_t thread;
	void *buff[256];

	i = 0;
	tmp_tree = tree;
	while (i < 256) {
		buff[tmp_tree->character] = tmp_tree;
		tmp_tree++;
		i++;
	}
	mmap_tmp = mmap(0, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	mmap_start = mmap_tmp;
	//	progress[10] = 0;
	//	memset(progress, ' ', 10);
	i = size - 1;
	decal = 0;
	packed_char = 0;
	write(fd, &size, 4);
	write_tree_base_on_file(tree, fd);
	options.actual.size = &size;
	options.actual.i = &i;
	options.compress_datas = 1;
	pthread_create(&thread, NULL, thread_print, (void*)&options);
	pthread_mutex_lock(&mutex);
	pthread_cond_wait(&start, &mutex);
	pthread_mutex_unlock(&mutex);
	//	printf("\tCompress datas: [%s]\r", progress);
	//	fflush(stdout);
	//	offset = 0;
	//	pourcent = (int)((long)((long)size * (long)((10 - offset) * 10)) / 100);
	while (i >= 0) {
		//		if (i < pourcent) {
		//			progress[offset - 1] = '=';
		//			printf("\tCompress datas: [%s] %d/%d\r", progress, size - i, size);
		//			fflush(stdout);
		//			offset++;
		//			pourcent = (int)((long)((long)size * (long)((10 - offset) * 10)) / 100);
		//		}
		tmp_tree = buff[*(unsigned char*)(mmap_addr + i)];
//		tmp_tree = tree;
//		while (tmp_tree->character != *(unsigned char*)(mmap_addr + i))
//			tmp_tree++;
		while (tmp_tree->parent) {
			if (decal == 8) {
				mmap_tmp++;
				decal = 0;
			}
			*mmap_tmp = *mmap_tmp << 1;
			decal++;
			*mmap_tmp += (tmp_tree->parent->left == tmp_tree) ? 0 : 1;
			tmp_tree = tmp_tree->parent;
		}
		i--;
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
	//	printf("\tCompress datas: [%s] %d/%d\n", progress, size, size);
	//	fflush(stdout);
	//	printf("\tWrite compressed datas on file: ..\r");
	//	fflush(stdout);
	write(fd, mmap_start, (void*)mmap_tmp - mmap_start + 1);
	munmap(mmap_start, size);
	packed_char = 8 - decal;
	write(fd, &packed_char, 1);
	pthread_mutex_lock(&mutex);
	options.write_datas = 2;
	pthread_mutex_unlock(&mutex);
	pthread_mutex_lock(&mutex);
	pthread_cond_signal(&ret);
	pthread_mutex_unlock(&mutex);
	pthread_join(thread, NULL);
	//	printf("\tWrite compressed datas on file: ok\n");
}

void write_tree_base_on_file(t_binary_tree *tree, int fd) {
	int i;
	void *mmap_addr;
	short nb_sheet_writed;
	int offset;

	mmap_addr = mmap(0, 5*256, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	i = 0;
	nb_sheet_writed = 0;
	offset = 0;
	while (i < 256) {
		if (tree->value) {
			*(int*)(mmap_addr + offset) = tree->value;
			*(unsigned char*)(mmap_addr + offset + 4) = tree->character;
			offset += 5;
			nb_sheet_writed++;
		}
		tree++;
		i++;
	}
	write(fd, &nb_sheet_writed, 2);
	write(fd, mmap_addr, nb_sheet_writed * 5);
	munmap(mmap_addr, 5*256);
	return ;
}

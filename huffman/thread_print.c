#include "huffman.h"

char get_opts_value(int index) {
	char mdr;

	pthread_mutex_lock(&mutex);
	mdr = *(char*)((void*)&options + index);
	pthread_mutex_unlock(&mutex);
	return (mdr);
}

void 	recurrence_print(void *arg) {
	t_thread_options *opts = arg;
	int offset;
	int pourcent;
	char progress[11];
	int tmp_size;

	tmp_size = *(opts->actual.size);
	memset(progress, ' ', 10);
	progress[10] = 0;
	offset = 1;
	pourcent = (int)((long)((long)*(opts->actual.size) * (long)(offset * 10)) / 100);
	pthread_mutex_lock(&mutex);
	pthread_cond_signal(&start);
	pthread_mutex_unlock(&mutex);
	while (get_opts_value(1) < 2) {
		if (pourcent < *(opts->actual.i)) {
			progress[offset - 1] = '=';
			offset++;
			pourcent = (int)((long)((long)*(opts->actual.size) * (long)(offset * 10)) / 100);
		}
		printf("\tCounting Bytes recurrence: [%s] %d/%d\r", progress, *(opts->actual.i), *(opts->actual.size));
		fflush(stdout);
	}
	memset(progress, '=', 10);
	printf("\tCounting Bytes recurrence: [%s] %d/%d\n", progress, tmp_size, tmp_size);
}

void 	base_sheet_print(void *arg) {
	t_thread_options *opts = arg;
	int offset;
	int pourcent;
	char progress[11];

	memset(progress, ' ', 10);
	progress[10] = 0;
	offset = 1;
	pourcent = (int)((long)((long)*(opts->actual.size) * (long)(offset * 10)) / 100);
	pthread_mutex_lock(&mutex);
	pthread_cond_signal(&start);
	pthread_mutex_unlock(&mutex);
	while (get_opts_value(2) < 2) {
		if (*(opts->actual.i) > pourcent) {
			progress[offset - 1] = '=';
			offset++;
			pourcent = (int)((long)((long)*(opts->actual.size) * (long)(offset * 10)) / 100);
			printf("\tCreating base sheet: [%s]\r", progress);
		}
		fflush(stdout);
	}
	memset(progress, '=', 10);
	printf("\tCreating base sheet: [%s]\n", progress);
}

void 	branchs_print(void *arg) {
	t_thread_options *opts = arg;
	int offset;
	int pourcent;
	char progress[11];

//	while (opts->branchs < 1) ;
	memset(progress, ' ', 10);
	progress[10] = 0;
	offset = 1;
	pourcent = (int)((long)((long)*(opts->actual.size) * (long)(offset * 10)) / 100);
	pthread_mutex_lock(&mutex);
	pthread_cond_signal(&start);
	pthread_mutex_unlock(&mutex);
	while (get_opts_value(3) < 2) {
		if (*(opts->actual.i) > pourcent) {
			progress[offset - 1] = '=';
			offset++;
			pourcent = (int)((long)((long)*(opts->actual.size) * (long)(offset * 10)) / 100);
			printf("\tCreating branchs: [%s]\r", progress);
		}
		fflush(stdout);
	}
	memset(progress, '=', 10);
	printf("\tCreating branchs: [%s]\n", progress);
}

void 	compress_print(void *arg) {
	t_thread_options *opts = arg;
	int offset;
	int pourcent;
	char progress[11];
	int tmp_size;

	tmp_size = *(opts->actual.size);
	memset(progress, ' ', 10);
	progress[10] = 0;
	offset = 1;
	pourcent = (int)((long)((long)*(opts->actual.size) * (long)((10 - offset) * 10)) / 100);
	pthread_mutex_lock(&mutex);
	pthread_cond_signal(&start);
	pthread_mutex_unlock(&mutex);
	while (get_opts_value(4) < 2) {
		if (*(opts->actual.i) < pourcent) {
			progress[offset - 1] = '=';
			offset++;
			pourcent = (int)((long)((long)*(opts->actual.size) * (long)((10 - offset) * 10)) / 100);
		}
		printf("\tCompress datas: [%s] %d/%d\r", progress, *(opts->actual.size) - *(opts->actual.i), *(opts->actual.size));
		fflush(stdout);
	}
	memset(progress, '=', 10);
	printf("\tCompress datas: [%s] %d/%d\n", progress, tmp_size, tmp_size);
	fflush(stdout);
}

void 	decompress_print(void *arg) {
	t_thread_options *opts = arg;
	int offset;
	int pourcent;
	char progress[11];
	int tmp_size;

	tmp_size = *(opts->actual.size);
	memset(progress, ' ', 10);
	progress[10] = 0;
	offset = 1;
		pourcent = (int)((long)((long)*(opts->actual.size) * (long)(offset * 10)) / 100);
	pthread_mutex_lock(&mutex);
	pthread_cond_signal(&start);
	pthread_mutex_unlock(&mutex);
	while (get_opts_value(4) < 2) {
		if (*(opts->actual.i) > pourcent) {
			progress[offset - 1] = '=';
			offset++;
			pourcent = (int)((long)((long)*(opts->actual.size) * (long)(offset * 10)) / 100);
		}
		printf("\tDecompressing bytes: [%s] %d/%d\r", progress, *(opts->actual.i), *(opts->actual.size));
		fflush(stdout);
	}
	memset(progress, '=', 10);
	printf("\tDecompressing bytes: [%s] %d/%d\n", progress, tmp_size, tmp_size);
	fflush(stdout);
}

void 	write_datas_print(void *arg) {
	t_thread_options *opts = arg;

	pthread_mutex_lock(&mutex);
	pthread_cond_signal(&start);
	pthread_mutex_unlock(&mutex);
//	while (opts->write_datas < 1) ;
	if (opts->pack_or_unpack)
		printf("\tWrite compressed datas on file: ..\r");
	else
		printf("\tWriting decompressed datas on file: ..\r");
	fflush(stdout);
//	while (opts->write_datas < 2)
//		;
	pthread_mutex_lock(&mutex);
	pthread_cond_wait(&ret, &mutex);
	pthread_mutex_unlock(&mutex);
//	if (opts->write_datas == 2) {
	if (opts->pack_or_unpack)
		printf("\tWrite compressed datas on file: ok\n");
	else
		printf("\tWriting decompressed datas on file: ok\n");
	fflush(stdout);
}

void 	*thread_print(void *arg) {
	t_thread_options *opts = arg;

	if (opts->pack_or_unpack && opts->recurrence == 1) {
		printf("Creating binary tree:\n");
		fflush(stdout);
		recurrence_print(arg);
	} else if (opts->base_sheet == 1) {
		if (!opts->pack_or_unpack) {
			printf("Creating binary tree:\n");
			fflush(stdout);
		}
		base_sheet_print(arg);
	} else if (opts->branchs == 1) {
		branchs_print(arg);
	} else if (opts->compress_datas == 1) {
		if (opts->pack_or_unpack) {
			printf("Compress and Write datas:\n");
			compress_print(arg);
		}
		else {
			printf("Decompress and Write datas:\n");
			decompress_print(arg);
		}
	} else if (opts->write_datas == 1) {
		write_datas_print(arg);
		if (opts->pack_or_unpack)
			printf("Datas successfully compressed !\n");
		else
			printf("Datas successfully decompressed !\n");
	}
	pthread_exit(NULL);
}

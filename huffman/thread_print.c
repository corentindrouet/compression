#include "huffman.h"

void	*thread_print(void *arg) {
	t_thread_options *opts = arg;
	int offset;
	int pourcent;
	char progress[11];
	int tmp_size;

//	printf("%p\n", &(opts->recurrence));
	if (opts->pack_or_unpack) {
		while (opts->recurrence < 1);
		tmp_size = *(opts->actual.size);
		printf("Creating binary tree:\n");
		memset(progress, ' ', 10);
		progress[10] = 0;
		offset = 1;
		pourcent = (int)((long)((long)*(opts->actual.size) * (long)(offset * 10)) / 100);
		while (opts->pack_or_unpack && opts->recurrence < 2) {
			if (pourcent < *(opts->actual.i)) {
				progress[offset - 1] = '=';
				offset++;
				pourcent = (int)((long)((long)*(opts->actual.size) * (long)(offset * 10)) / 100);
			}
			printf("\tCounting Bytes recurrence: [%s] %d/%d\r", progress, *(opts->actual.i), *(opts->actual.size));
			fflush(stdout);
		}
		if (opts->pack_or_unpack && opts->recurrence == 2) {
			memset(progress, '=', 10);
			printf("\tCounting Bytes recurrence: [%s] %d/%d\n", progress, tmp_size, tmp_size);
		}
	}

	while (opts->base_sheet < 1) ;
	if (!opts->pack_or_unpack)
		printf("Creating binary tree:\n");
	memset(progress, ' ', 10);
	progress[10] = 0;
	offset = 1;
	pourcent = (int)((long)((long)*(opts->actual.size) * (long)(offset * 10)) / 100);
	while (opts->base_sheet < 2) {
		if (*(opts->actual.i) > pourcent) {
			progress[offset - 1] = '=';
			offset++;
			pourcent = (int)((long)((long)*(opts->actual.size) * (long)(offset * 10)) / 100);
			printf("\tCreating base sheet: [%s]\r", progress);
			fflush(stdout);
		}
	}
	if (opts->base_sheet == 2) {
		memset(progress, '=', 10);
		printf("\tCreating base sheet: [%s]\n", progress);
	}

	while (opts->branchs < 1) ;
	memset(progress, ' ', 10);
	progress[10] = 0;
	offset = 1;
	pourcent = (int)((long)((long)*(opts->actual.size) * (long)(offset * 10)) / 100);
	while (opts->branchs < 2) {
		if (*(opts->actual.i) > pourcent) {
			progress[offset - 1] = '=';
			offset++;
			pourcent = (int)((long)((long)*(opts->actual.size) * (long)(offset * 10)) / 100);
			printf("\tCreating branchs: [%s]\r", progress);
			fflush(stdout);
		}
	}
	if (opts->branchs == 2) {
		memset(progress, '=', 10);
		printf("\tCreating branchs: [%s]\n", progress);
	}

	while (opts->compress_datas < 1) ;
	tmp_size = *(opts->actual.size);
	if (opts->pack_or_unpack)
		printf("Compress and Write datas:\n");
	else
		printf("Decompress and Write datas:\n");
	memset(progress, ' ', 10);
	progress[10] = 0;
	offset = 1;
	if (opts->pack_or_unpack)
		pourcent = (int)((long)((long)*(opts->actual.size) * (long)((10 - offset) * 10)) / 100);
	else
		pourcent = (int)((long)((long)*(opts->actual.size) * (long)(offset * 10)) / 100);
	while (opts->compress_datas < 2) {
		if (opts->pack_or_unpack) {
			if (*(opts->actual.i) < pourcent) {
				progress[offset - 1] = '=';
				offset++;
				pourcent = (int)((long)((long)*(opts->actual.size) * (long)((10 - offset) * 10)) / 100);
			}
			printf("\tCompress datas: [%s] %d/%d\r", progress, *(opts->actual.size) - *(opts->actual.i), *(opts->actual.size));
			fflush(stdout);
		} else {
			if (*(opts->actual.i) > pourcent) {
				progress[offset - 1] = '=';
				offset++;
				pourcent = (int)((long)((long)*(opts->actual.size) * (long)(offset * 10)) / 100);
			}
			printf("\tDecompressing bytes: [%s] %d/%d\r", progress, *(opts->actual.i), *(opts->actual.size));
			fflush(stdout);
		}
	}
	if (opts->compress_datas == 2) {
		memset(progress, '=', 10);
		if (opts->pack_or_unpack)
			printf("\tCompress datas: [%s] %d/%d\n", progress, tmp_size, tmp_size);
		else
			printf("\tDecompressing bytes: [%s] %d/%d\n", progress, tmp_size, tmp_size);
		fflush(stdout);
	}


	while (opts->write_datas < 1) ;
	if (opts->pack_or_unpack)
		printf("\tWrite compressed datas on file: ..\r");
	else
		printf("\tWriting decompressed datas on file: ..\r");
	fflush(stdout);
	while (opts->write_datas < 2) ;
	if (opts->write_datas == 2) {
		if (opts->pack_or_unpack)
			printf("\tWrite compressed datas on file: ok\n");
		else
			printf("\tWriting decompressed datas on file: ok\n");
		fflush(stdout);
	}

	if (opts->pack_or_unpack)
		printf("Datas successfully compressed !\n");
	else
		printf("Datas successfully decompressed !\n");
	pthread_exit(NULL);
}

#include <stdlib.h>
#include <string.h>
#include "encode.h"


FileNode *openfiles = NULL;


int *rlestreamdecode(char *filepath, size_t seg_len, EncType etype) {
	printf("rsd check 1\n");

	FileNode *targetfile = openfiles;

	while (targetfile != NULL) {
		if (!strcmp(filepath, targetfile -> filepath)) {
			break;
		}

		targetfile = targetfile -> next;
	}

	printf("rsd check 2\n");

	if (targetfile == NULL && openfiles == NULL) {
		printf("rsd check 2.1.1\n");

		FILE *infile = fopen(filepath, "r");

		targetfile = openfiles = (FileNode *)malloc(sizeof(FileNode));
		targetfile -> prev = targetfile -> next = NULL;
		targetfile -> filepath = strdup(filepath);
		targetfile -> etype = etype;
		targetfile -> cursor = (void *)malloc((seg_len + 1)*sizeof(RLEPair));

		printf("rsd check 2.2.1 - %s, %p\n", filepath, infile);

		size_t num_read = fread(((RLEPair *)(targetfile -> cursor)) + 1, sizeof(RLEPair), seg_len, infile);

		printf("rsd check 2.3.1\n");

		targetfile -> eofreached = num_read != seg_len;
		((RLEPair *)(targetfile -> cursor)) -> data = num_read;

		targetfile -> fp = infile;
	}
	else if (targetfile == NULL && openfiles != NULL) {
		printf("rsd check 2.1.2\n");

		FILE *infile = fopen(filepath, "r");

		targetfile = (FileNode *)malloc(sizeof(FileNode));
		targetfile -> prev = NULL;
		targetfile -> next = openfiles;
		targetfile -> filepath = strdup(filepath);
		targetfile -> etype = etype;
		targetfile -> cursor = (void *)malloc((seg_len + 1)*sizeof(RLEPair));

		size_t num_read = fread(((RLEPair *)(targetfile -> cursor)) + 1, sizeof(RLEPair), seg_len, infile);

		targetfile -> eofreached = num_read != seg_len;
		((RLEPair *)(targetfile -> cursor)) -> data = num_read;

		targetfile -> fp = infile;

		openfiles = targetfile;
	}

	printf("rsd check 2\n");

	int cursor_len = ((RLEPair *)(targetfile -> cursor)) -> data;

	if (cursor_len < seg_len && !(targetfile -> eofreached)) {
		targetfile -> cursor = (void *)realloc(targetfile -> cursor, (cursor_len + seg_len + 1)*sizeof(RLEPair));

		size_t num_read = fread((RLEPair *)(targetfile -> cursor) + 1 + cursor_len,
			sizeof(RLEPair), seg_len, targetfile -> fp);

		((RLEPair *)(targetfile -> cursor)) -> data = cursor_len + num_read;

		targetfile -> eofreached = (num_read != seg_len);
	}

	printf("rsd check 4\n");

	int *toReturn = (int *)malloc(seg_len*sizeof(int));

	size_t write_ctr = 0;
	size_t read_ctr = 0;

	RLEPair *enc = (RLEPair *)(targetfile -> cursor) + 1;

	cursor_len = ((RLEPair *)(targetfile -> cursor)) -> data;

	printf("rsd check 5\n");

	for (size_t i = 0; i < seg_len; ++i) {
		toReturn[i] = (enc + read_ctr) -> data;
		++write_ctr;
		(enc + read_ctr) -> count = (enc + read_ctr) -> count - 1;
		read_ctr += !((enc + read_ctr) -> count);
		i += (seg_len - read_ctr)*(read_ctr == cursor_len);
	}

	printf("rsd check 6\n");

	memmove(((RLEPair *)(targetfile -> cursor)) + 1, ((RLEPair *)(targetfile -> cursor)) + 1 + read_ctr,
		sizeof(RLEPair)*(cursor_len - read_ctr));

	((RLEPair *)(targetfile -> cursor)) -> data = cursor_len - read_ctr;

	if (write_ctr < seg_len) {
		toReturn = realloc(toReturn, write_ctr*sizeof(int));
	}

	printf("rsd check 7\n");

	return toReturn;
}




















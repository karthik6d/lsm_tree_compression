#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef enum EncType {
	RLE,
	DELTA,
} EncType;

typedef struct RLEPair {
	int data;
	int count;
} RLEPair;

// typedef struct RLECursor {
// 	int rel_len;
// 	RLEPair *enc;
// }

typedef struct FileNode {
	EncType etype;
	char *filepath;
	FILE *fp;
	void *cursor;
	int eofreached;
	struct FileNode *prev;
	struct FileNode *next;
} FileNode;


int *rlestreamdecode(char *filepath, size_t seg_len, EncType etype);






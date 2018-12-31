#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>


#define N_BLOCK_DATA 56
#define N_DATA  80
#define BLOCK_SIZE 4096
#define DATA_SIZE 528
#define DATA_BLOCK_SIZE 512
#define N_CHILDREN_OFFSET 40
#define PATH_OFFSET 44

struct superBlock {
	int totalblocks;
	int totalfreeb;
};

struct block {
	int blockno;
	int valid;
	void *data;
	int size;
	int current_size;
};

struct fileInfo
{
	int fileId;
	nlink_t st_nlink;
	size_t size;
	mode_t mode;
	blkcnt_t blockcount;
	blksize_t block_size;
	int blockno[8];
	char *path;
	int path_length;
	char *name;
	int name_length;
	int uid;
	int gid;
};

struct directory {
	mode_t mode;
	int block_number;
	int offset;
	int index;
	int path_length;
	char *path;
	int name_length;
	char *name;
	struct directory **children;
	struct directory *parent;
	int *fileBlockNumbers;
	int n_children;
	int n_link;
	int filecount;
	int uid;
	int gid;
};

char *BlockBitMap;
char *dataBitMap;

struct superBlock super;


struct directory *root;

void initializeNode(struct directory *node,int blkNumber, int fp);
void loadDataBitMap();
int getFreeBlock();
void writeNode(struct directory *node,int blkNumber,struct directory *parent);
int persistence();
int checkIfDelimiterInPath(const char *path);
void createChild(struct directory *p,char *path,char *name,mode_t x);
struct fileInfo *checkFilePath(const char *path);

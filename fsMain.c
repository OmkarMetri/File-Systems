#include "fs.h"
#include "disk.c"
#include "tree.c"
#include "fuse_operations.c"


static struct fuse_operations operations = {
	.init       = file_init,
    .getattr	= do_getattr,
    .readdir	= do_readdir,
    .mkdir      = file_mkdir,
    .rmdir      = file_rmdir,
    .create     = file_create,
    .open       = file_open,
    .utime      = file_utimes,
    .read       = file_read1,
    .write      = file_write,
    .unlink     = file_unlink
};

int main(int argc,char *argv[])
{
	return fuse_main(argc,argv,&operations,NULL);
}


// for debugging
/*int main() {
	file_init();
	//file_mkdir("/hello1/hello5",0);
	//file_mkdir("/hello1",0);
	//file_mkdir("/hello2",0);
	//file_mkdir("/hello3",0);
	//file_mkdir("/hello4",0);
	//file_rmdir("/hello2");
	//file_mkdir("/hello1/hello5",0);
}*/

//8248
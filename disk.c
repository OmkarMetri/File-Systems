
void loadDataBitMap() {
	int fp = open("dataBitMap",O_RDWR,0644);
	dataBitMap = (char *)malloc(sizeof(char)*N_DATA);
	read(fp,dataBitMap,N_DATA);
	////printf("loading data bit map\n");
	for(int i=0;i<N_DATA;i++) {
		//printf("%c",dataBitMap[i]);
	}
	////printf("\n");
	close(fp);

	int fd = open("BlockBitMap",O_RDWR,0644);
	super.totalblocks = N_BLOCK_DATA;
	super.totalfreeb = N_BLOCK_DATA;
	BlockBitMap = (char *)malloc(sizeof(char)*N_BLOCK_DATA);
	read(fd,BlockBitMap,N_BLOCK_DATA);
	////printf("loading block bit map\n");
	for(int i=0;i<N_BLOCK_DATA;i++) {
		//printf("%c",BlockBitMap[i]);
		if(BlockBitMap[i]=='1')
			super.totalfreeb-=1;

	}
	////printf("\n");
	close(fd);
}

void createDataBitMap() {
	int fp = open("dataBitMap",O_RDWR | O_CREAT,0644);
	dataBitMap = (char *)malloc(sizeof(char)*N_DATA);
	for(int i=0;i<N_DATA;i++) {
		dataBitMap[i] = '0';
	}
	write(fp,dataBitMap,N_DATA);
	close(fp);

	int fd = open("BlockBitMap",O_RDWR | O_CREAT,0644);
	super.totalblocks = N_BLOCK_DATA;
	super.totalfreeb = N_BLOCK_DATA;
	BlockBitMap = (char *)malloc(sizeof(char)*N_BLOCK_DATA);
	for(int i=0;i<N_BLOCK_DATA;i++) {
		BlockBitMap[i]='0';
	}
	write(fp,BlockBitMap,N_BLOCK_DATA);
	////printf("\n");
	close(fd);
}

int getFreeBlock() {
	int fp = open("dataBitMap",O_RDWR | O_CREAT,0644);
	for(int i=0;i<N_DATA;i++) {
		if(dataBitMap[i] == '0') {
			////printf("i = %d\n",i);
			dataBitMap[i] = '1';
			////printf("modifying dataBitMap\n");
			lseek(fp,0,SEEK_CUR);
			write(fp,dataBitMap,N_DATA);
			return i;
		}
	}
	
	return -1;
}
int getFreeDataBlock(){
	int fp = open("BlockBitMap",O_RDWR | O_CREAT,0644);
	for(int i=0;i<N_BLOCK_DATA;i++) {
		if(BlockBitMap[i] == '0') {
			////printf("i = %d\n",i);
			super.totalfreeb-=1;
			BlockBitMap[i] = '1';
			////printf("modifying dataBitMap\n");
			lseek(fp,0,SEEK_CUR);
			write(fp,BlockBitMap,N_BLOCK_DATA);
			return i;
		}
	}
	
	return -1;
}
void unsetDataBit(int pos) {
	////printf("unset block bit %d in databitmap\n",pos);
	int fp = open("dataBitMap",O_RDWR | O_CREAT,0644);
	dataBitMap[pos] = '0';
	lseek(fp,0,SEEK_CUR);
	write(fp,dataBitMap,N_DATA);
}


void initializeTree(struct directory *node,int blkNumber, int fp) {
	
	

	lseek(fp,blkNumber*BLOCK_SIZE,SEEK_SET); // go to offset in disk
	

	// read data
	read(fp,&(node->offset),sizeof(node->offset));
	read(fp,&(node->block_number),sizeof(node->block_number));
	read(fp,&(node->index),sizeof(node->index));
	read(fp,&(node->n_link),sizeof(node->n_link));
	read(fp,&(node->uid),sizeof(node->uid));
	read(fp,&(node->gid),sizeof(node->gid));
	read(fp,&(node->mode),sizeof(node->mode));
	
	read(fp,&(node->path_length),sizeof(node->path_length));
	read(fp,&(node->name_length),sizeof(node->name_length));
	read(fp,&(node->filecount),sizeof(node->filecount));
	read(fp,&(node->n_children),sizeof(node->n_children));
	node->path = (char *)malloc(sizeof(char)*node->path_length+1);
	read(fp,node->path,node->path_length);
	node->path[node->path_length] = '\0';
	node->name = (char *)malloc(sizeof(char)*node->name_length+1);
	read(fp,node->name,node->name_length);
	node->name[node->name_length] = '\0';
	////printf("[initializeTree] initializing node %s\n",node->name);
	

	
	node->fileBlockNumbers = (int *)malloc(sizeof(int)*node->filecount);
	
	int *childBlockNumbers = (int *)malloc(sizeof(int)*node->n_children);

	int i=0;
	int j=0;

	for(int k=0;k<node->n_children+node->filecount;k++) {
		char type;
		read(fp,&type,sizeof(type));
		if(type == 'f') {
			read(fp,&(node->fileBlockNumbers[j]),sizeof(node->fileBlockNumbers[j]));
			j++;
		} else if(type == 'd') {
			read(fp,&(childBlockNumbers[i]),sizeof(node->block_number));
			i++;
		}
	}

	node->children = (struct directory **)malloc(sizeof(struct directory *)*node->n_children);
	for(int i=0;i<node->n_children;i++) {
		node->children[i] = (struct directory *)malloc(sizeof(struct directory));
		initializeTree(node->children[i],childBlockNumbers[i],fp);
	}
}

void writeNode(struct directory *node,int blkNumber,struct directory *parent) {
	////printf("p->name = %s\n",parent->name);
	int fp = open("fsData",O_RDWR | O_CREAT,0644);
	lseek(fp,blkNumber*BLOCK_SIZE,SEEK_SET);
	
	write(fp,&(node->offset),sizeof(node->offset));
	////printf("offset = %ld\n",lseek(fp,0,SEEK_CUR));

	write(fp,&(node->block_number),sizeof(node->block_number));
	////printf("block_number = %ld\n",lseek(fp,0,SEEK_CUR));

	write(fp,&(node->index),sizeof(node->index));
	////printf("index = %ld\n",lseek(fp,0,SEEK_CUR));

	write(fp,&(node->n_link),sizeof(node->n_link));
	////printf("n_link = %ld\n",lseek(fp,0,SEEK_CUR));

	write(fp,&(node->uid),sizeof(node->uid));
	////printf("uid = %ld\n",lseek(fp,0,SEEK_CUR));	

	write(fp,&(node->gid),sizeof(node->gid));
	////printf("gid = %ld\n",lseek(fp,0,SEEK_CUR));	

	write(fp,&(node->mode),sizeof(node->mode));
	////printf("mode = %ld\n",lseek(fp,0,SEEK_CUR));	

	write(fp,&(node->path_length),sizeof(node->path_length));
	////printf("path_length = %ld\n",lseek(fp,0,SEEK_CUR));	

	write(fp,&(node->name_length),sizeof(node->name_length));
	////printf("name_length = %ld\n",lseek(fp,0,SEEK_CUR));	

	write(fp,&(node->filecount),sizeof(node->filecount));
	////printf("filecount = %ld\n",lseek(fp,0,SEEK_CUR));

	write(fp,&(node->n_children),sizeof(node->n_children));
	////printf("n_children = %ld\n",lseek(fp,0,SEEK_CUR));

	write(fp,node->path,(int)node->path_length);
	////printf("path = %ld\n",lseek(fp,0,SEEK_CUR));	

	write(fp,node->name,(int)node->name_length);
	////printf("name = %ld\n",lseek(fp,0,SEEK_CUR));	
	 
	if(parent!=NULL) {
		lseek(fp,(parent->block_number*BLOCK_SIZE)+N_CHILDREN_OFFSET,SEEK_SET);
		write(fp,&(parent->n_children),sizeof(parent->n_children));
		
		lseek(fp,(parent->block_number*BLOCK_SIZE) + parent->offset,SEEK_SET);
		////printf("parent->block_number*BLOCK_SIZE = %d\n",parent->block_number*BLOCK_SIZE);
		char type = 'd';
		////printf("while writing type of child = %ld and type is %c\n",lseek(fp,0,SEEK_CUR),type);
		write(fp,&type,sizeof(type));
		write(fp,&(node->block_number),sizeof(node->block_number));
		
		parent->offset = parent->offset + sizeof(type)+sizeof(node->block_number);
		lseek(fp,(parent->block_number*BLOCK_SIZE),SEEK_SET);
		write(fp,&(parent->offset),sizeof(parent->offset));
		////printf("parent->offset after inserting child %d\n",parent->offset);
	}

}

void removeNode(int child_blkNumber,struct directory *parent) {
	//printf("child_blkNumber = %d\n",child_blkNumber);
	int fp = open("fsData",O_RDWR | O_CREAT,0644);
	int last_element_offset = ((parent->block_number*BLOCK_SIZE)+parent->offset) - (sizeof(char)+sizeof(parent->block_number));
	//printf("[rmdir] parent->offset = %d\n",parent->offset);
	//printf("[rmdir] last_element_offset = %d\n",last_element_offset);

	if(parent->n_children+parent->filecount > 1) {
		//printf("[rmdir] entering if\n");
		int last_file_blockNumber;
		char last_file_type;
		
		// getting last file block and type for swap
		lseek(fp,last_element_offset,SEEK_SET);
		read(fp,&last_file_type,sizeof(char));
		read(fp,&last_file_blockNumber,sizeof(int));
		lseek(fp,last_element_offset,SEEK_SET);

		//printf("last_file_type = %c\n",last_file_type);
		//printf("last_file_blockNumber = %d\n",last_file_blockNumber);
		
		
		int list_offset = PATH_OFFSET + strlen(parent->path) + strlen(parent->name);
		
		lseek(fp,(parent->block_number*BLOCK_SIZE)+list_offset,SEEK_SET);
		
		for(int i=0;i<(parent->n_children+parent->filecount);i++) {
			char type;
			int blkNumber;
			int pos = lseek(fp,0,SEEK_CUR);	
			//printf("[rmdir] pos offset while searching = %d\n",pos);
			read(fp,&type,sizeof(char));
			read(fp,&blkNumber,sizeof(parent->block_number));
			//printf("blkNumber = %d\n",blkNumber);
			//printf("child_blkNumber = %d\n",child_blkNumber);

			if(blkNumber == child_blkNumber && type == 'd') {
				//printf("[rmdir] removing blkNumber = %d\n",blkNumber);
				char check_type;
				int check_blk;
				lseek(fp,pos,SEEK_SET);
				write(fp,&last_file_type,sizeof(char));
				write(fp,&last_file_blockNumber,sizeof(parent->block_number));
				lseek(fp,pos,SEEK_SET);
				read(fp,&check_type,sizeof(char));
				read(fp,&check_blk,sizeof(parent->block_number));
				//printf("[rmdir] check_type = %c\n",check_type);
				//printf("[rmdir] check_blk = %d\n",check_blk);
				break;
			}
		}
	} else {
		lseek(fp,last_element_offset,SEEK_SET);
		// may reset the block later
	}
	
	// reduce the n_children count of parent
	lseek(fp,(parent->block_number*BLOCK_SIZE)+N_CHILDREN_OFFSET,SEEK_SET);
	int new_n_children = (parent->n_children)-1;
	write(fp,&new_n_children,sizeof(int));
	
	parent->offset = parent->offset - sizeof(parent->block_number) - sizeof(char); // reducing offset of parent
	lseek(fp,(parent->block_number*BLOCK_SIZE),SEEK_SET);
	write(fp,&(parent->offset),sizeof(parent->offset));
	
	unsetDataBit(child_blkNumber); // unset data bit
	
	close(fp);
}


int persistence() {
	//loadInodeBitMap();
	int fp;
	if((fp = open("fsData",O_RDWR,0644)) == -1) {
		return 0;
	}
	//printf("Restoring the original file system\n");
	loadDataBitMap();
	root = (struct directory *)malloc(sizeof(struct directory));
	initializeTree(root,0,fp);
	if(root == NULL) {
		//printf("root is null\n");
	}
	return 1;

}
int checkfilenode(struct directory *dir, const char *name){
	if(dir->filecount<1)
		return 0;
	int fp = open("fsData",O_RDWR,0644);
	int i,l;
	for(i=0;i<dir->filecount;i++){
		int offset = ((dir->fileBlockNumbers[i]*BLOCK_SIZE)+sizeof(dir->fileBlockNumbers[i]));
		lseek(fp,offset,SEEK_SET);
		read(fp,&l,sizeof(int));
		char *temp = (char *)malloc(sizeof(char)*l);
		read(fp,temp,sizeof(char)*l);
		//printf("%d :files : %s\n",dir->fileBlockNumbers[i],temp);
		if(strcmp(name,temp)==0){
			//printf("found with blkNumber:%d\n",dir->fileBlockNumbers[i]);
			free(temp);
			close(fp);
			return dir->fileBlockNumbers[i];
		}
		free(temp);
	}
	close(fp);
	return 0;
}

void writefileNode(struct directory *p,struct fileInfo *node){
	int fp = open("fsData",O_RDWR | O_CREAT,0644);
	lseek(fp,node->fileId*BLOCK_SIZE,SEEK_SET);
	//printf("Writing file to persistence storage\n");

	write(fp,&(node->fileId),sizeof(node->fileId));
	//printf("block_number = %d\n",node->fileId);

	write(fp,&(node->name_length),sizeof(node->name_length));
	//printf("name_length = %d\n",node->name_length);	

	write(fp,node->name,(int)node->name_length);
	//printf("name = %s\n",node->name);

	write(fp,&(node->st_nlink),sizeof(node->st_nlink));
	//printf("n_link = %ld\n",lseek(fp,0,SEEK_CUR));

	write(fp,&(node->uid),sizeof(node->uid));
	//printf("uid = %ld\n",lseek(fp,0,SEEK_CUR));	

	write(fp,&(node->gid),sizeof(node->gid));
	//printf("gid = %ld\n",lseek(fp,0,SEEK_CUR));	

	write(fp,&(node->mode),sizeof(node->mode));
	//printf("mode = %ld\n",lseek(fp,0,SEEK_CUR));	

	write(fp,&(node->path_length),sizeof(node->path_length));
	//printf("path_length = %d\n",node->path_length);	

	write(fp,node->path,(int)node->path_length);
	//printf("path = %s\n",node->path);

	write(fp,&(node->blockcount),sizeof(node->blockcount));
	//printf("blockcount = %ld\n",lseek(fp,0,SEEK_CUR));	

	write(fp,&(node->block_size),sizeof(node->block_size));
	//printf("block_size = %ld\n",lseek(fp,0,SEEK_CUR));

	write(fp,&(node->size),sizeof(node->size));
	//printf("size = %ld\n",lseek(fp,0,SEEK_CUR));	

	write(fp,node->blockno,8*sizeof(int));
	
	// modifying directory to indicate that it contains a new file
	lseek(fp,(p->block_number*BLOCK_SIZE)+N_CHILDREN_OFFSET-sizeof(p->n_children),SEEK_SET);
	write(fp,&(p->filecount),sizeof(p->filecount));

	lseek(fp,p->block_number*BLOCK_SIZE+p->offset,SEEK_SET);
	char type = 'f';
	write(fp,&type,sizeof(type));
	write(fp,&(node->fileId),sizeof(node->fileId));
	
	p->offset = p->offset + sizeof(type) + sizeof(node->fileId);

	lseek(fp,p->block_number*BLOCK_SIZE,SEEK_SET);
	write(fp,&(p->offset),sizeof(p->offset));
	
	close(fp);


}

struct fileInfo *get_file(int x){
	int fp = open("fsData",O_RDWR,0644);
	lseek(fp,x*BLOCK_SIZE,SEEK_SET);
	struct  fileInfo *node = (struct fileInfo *)malloc(sizeof(struct fileInfo));
	//printf("Reading file from persistence storage\n");

	read(fp,&(node->fileId),sizeof(node->fileId));

	read(fp,&(node->name_length),sizeof(node->name_length));

	node->name = (char *)malloc(sizeof(char)*node->name_length+1);
	read(fp,node->name,(int)node->name_length);
	node->name[node->name_length] = '\0';
	

	read(fp,&(node->st_nlink),sizeof(node->st_nlink));

	read(fp,&(node->uid),sizeof(node->uid));

	read(fp,&(node->gid),sizeof(node->gid));

	read(fp,&(node->mode),sizeof(node->mode));	

	read(fp,&(node->path_length),sizeof(node->path_length));
	node->path = (char *)malloc(sizeof(char)*node->path_length+1);
	read(fp,node->path,(int)node->path_length);
	node->path[node->path_length] = '\0';

	read(fp,&(node->blockcount),sizeof(node->blockcount));	

	read(fp,&(node->block_size),sizeof(node->block_size));

	read(fp,&(node->size),sizeof(node->size));	

	read(fp,node->blockno,8*sizeof(int));
	//printf("Read from storage\n");
	close(fp);
	return node;
}

int read_block(int blocknr, char *buf,off_t new_off,off_t off,size_t n) {
	if(n>DATA_BLOCK_SIZE)
		return -1;

	int fd = open("Blockdata",O_RDWR|O_CREAT,0644);
	lseek(fd,blocknr*DATA_SIZE+16+new_off,SEEK_SET);
	char *temp_buf = (char *)malloc(sizeof(char)*n);
	read(fd,temp_buf,n);
	printf("read :%d,%ld\n",blocknr,n);
	strncpy(buf+off,temp_buf,n);
	return 0;
}


int write_block(int blocknr,const char *buf,off_t new_off,off_t off,size_t n)
{	
	printf("in writeb %d %ld\n",blocknr,n);
	if(blocknr>N_BLOCK_DATA)
		return -1;
	if(n>DATA_BLOCK_SIZE)
		return -1;
	int fd = open("Blockdata",O_RDWR|O_CREAT,0644);
	lseek(fd,blocknr*DATA_SIZE+16+new_off,SEEK_SET);
	write(fd,buf+off,n);
	close(fd);
	return 0;
}


void write_new(int x)
{
	int fd = open("Blockdata",O_RDWR|O_CREAT,0644);
	struct block *new = (struct block *)malloc(sizeof(struct block));
	new->blockno = x;
	new->valid = 1;
	new->size = DATA_BLOCK_SIZE;
	new->current_size = 0;
	new->data = (void*)malloc(sizeof(char)*DATA_BLOCK_SIZE);
	lseek(fd,x*DATA_SIZE,SEEK_SET);
	write(fd,&(new->blockno),sizeof(new->blockno));
	//printf("block no :%d\n",new->blockno);

	write(fd,&(new->valid),sizeof(new->valid));
	//printf("valid :%d\n",new->valid);

	write(fd,&(new->size),sizeof(new->size));
	//printf("size :%d\n",new->size);

	write(fd,&(new->current_size),sizeof(new->current_size));
	//printf("current_size :%d\n",new->current_size);

	memset(new->data,'0', sizeof(char)*DATA_BLOCK_SIZE);
	write(fd,new->data,sizeof(char)*DATA_BLOCK_SIZE);

	close(fd);
}

void write_size(int blocknr,int n){
	int fd = open("Blockdata",O_RDWR|O_CREAT,0644);
	lseek(fd,blocknr*DATA_SIZE+12,SEEK_SET);
	int s;
	read(fd,&s,sizeof(s));
	lseek(fd,-4,SEEK_CUR);
	s = s+n;
	write(fd,&s,sizeof(s));
	close(fd);
}

void update_file(struct fileInfo *node){
	int offset = node->fileId*BLOCK_SIZE+(int)node->name_length+
				(int)node->path_length+32;
	int fd = open("fsData",O_RDWR,0644);
	lseek(fd,offset,SEEK_SET);
	write(fd,&(node->blockcount),sizeof(node->blockcount));

	lseek(fd,8,SEEK_CUR);
	write(fd,&(node->size),sizeof(node->size));
	write(fd,node->blockno,8*sizeof(int));
	close(fd);
}

void rm(struct directory *node,int k){
		int fp = open("fsData",O_RDWR | O_CREAT,0644);
	char v;int r;
	lseek(fp,node->block_number*BLOCK_SIZE+node->offset-sizeof(char)-sizeof(int),SEEK_SET);
	read(fp,&v,sizeof(char));
	read(fp,&r,sizeof(int));
	
	lseek(fp,(node->block_number*BLOCK_SIZE+N_CHILDREN_OFFSET-sizeof(node->n_children)),SEEK_SET); 
	write(fp,&(node->filecount),sizeof(node->filecount));

	lseek(fp,(int)node->name_length+2*sizeof(node->name_length)+(int)node->path_length+sizeof(int),SEEK_CUR);
	char type;
	int p;
	for(int i=0;i<node->n_children+node->filecount;i++){
		read(fp,&type,sizeof(char));
		read(fp,&p,sizeof(int));
		if(type=='f' && p==k){
			lseek(fp,-(sizeof(type)+sizeof(p)),SEEK_CUR);
			write(fp,&v,sizeof(char));
			write(fp,&r,sizeof(int));
			break;
		}

		}

	lseek(fp,(node->block_number*BLOCK_SIZE),SEEK_SET);
	
	node->offset = node->offset - sizeof(char) - sizeof(int);
	write(fp,&(node->offset),sizeof(node->offset));
	close(fp);

}
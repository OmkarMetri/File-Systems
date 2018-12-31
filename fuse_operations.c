static int do_getattr(const char *path,struct stat *st) {
	//printf( "[getattr] Called\n" );
	//printf( "\tAttributes of %s requested\n", path );
	
	if (strcmp(path,"/") == 0 )
	{
		st->st_mode = root->mode;
		st->st_uid = 1000;
		st->st_gid = 1000;
	} else if(strcmp(path,"/.xdg-volume-info") == 0 ||
		strcmp(path,"/.Trash") == 0 || strcmp(path,"/.Trash-1000") == 0 ||
		strcmp(path,"/autorun.inf") == 0 ) {
		st->st_uid = getuid();
		st->st_gid = getgid();
		st->st_atime = time(NULL);
		st->st_mtime = time(NULL);
	} else {
		char *temp_path = (char *)malloc(sizeof(char)*strlen(path));
		strncpy(temp_path,path+1,strlen(path));
		printf("temp_path = %s\n",temp_path);
		struct directory *dir = checkPath(temp_path);
		if(dir == NULL) {
			//printf("Files\n");
			struct fileInfo *f = find_file(temp_path);
			if(f==NULL){
				printf("File not found %s\n",temp_path);
				return -ENOENT;
			}
			else{
				printf("file found\n");
				st->st_mode   = f->mode;
				st->st_size   = f->size;
				st->st_blocks = f->blockcount;
				st->st_uid    = f->uid;
				st->st_gid = f->gid;
				st->st_atime = time(NULL);
				st->st_mtime = time(NULL);
			}
		}
		else{ 
		st->st_mode = dir->mode;
		st->st_uid = dir->uid;
		st->st_gid = dir->gid;
		st->st_atime = time(NULL);
		st->st_mtime = time(NULL);
		}
	}
	return 0;
}

static int do_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	//printf( "--> Getting The List of Files of %s\n",path);
	
	filler(buffer,".",NULL,0);
	filler(buffer,"..",NULL,0);
	if(root == NULL) {
		//printf("root is NULL\n");
	} else {
		//printf("root is not NULL\n");
	}
	if(strcmp( path, "/" )== 0){
		//printf("number of root children = %d\n",root->n_children);
		//printf("number of files = %d\n",root->filecount);
		for(int i=0;i<root->n_children;i++) {
			if(root->children[i] != NULL)
				filler(buffer,root->children[i]->name,NULL,0);
		}
		if(root->filecount>0){
			//printf("Getting files : %d\n",root->filecount);
			int fp = open("fsData",O_RDWR,0644);
			for (int i = 0; i < root->filecount; ++i){
				int l,offset = ((root->fileBlockNumbers[i]*BLOCK_SIZE)+sizeof(root->fileBlockNumbers[i]));
				lseek(fp,offset,SEEK_SET);
				read(fp,&l,sizeof(int));
				char *temp = (char *)malloc(sizeof(char)*(l+1));
				read(fp,temp,sizeof(char)*l);
				temp[l] = '\0';
				filler(buffer,temp,NULL,0);
				free(temp);
				}
			close(fp);
		}			
	}
	else {
		struct directory *dir = checkPath(path);
		//printf("number of dir children = %d\n",dir->n_children);
		//printf("number of files = %d\n",dir->filecount);
		for(int i=0;i<dir->n_children;i++) {
			filler(buffer,dir->children[i]->name,NULL,0);
		}
		if(dir->filecount>0){
			int fp = open("fsData",O_RDWR,0644);
			for (int i = 0; i < dir->filecount; ++i) {
				int l,offset = ((dir->fileBlockNumbers[i]*BLOCK_SIZE)+sizeof(dir->fileBlockNumbers[i]));
				lseek(fp,offset,SEEK_SET);
				read(fp,&l,sizeof(int));
				char *temp = (char *)malloc(sizeof(char)*(l+1));
				read(fp,temp,sizeof(char)*l);
				temp[l] = '\0';
				filler(buffer,temp,NULL,0);
				free(temp);
			}
			close(fp);
		}

	}
	
	return 0;
}




static void *file_init() {
	printf("[init] entering");
	if(persistence() == 0) {
		createDataBitMap();
		//printf("initializing root\n");
		root = (struct directory *)malloc(sizeof(struct directory));
		root->block_number = getFreeBlock();
		root->index = 0;
		root->n_link = 2;
		root->uid = getuid();
		root->gid = getgid();
		root->mode = S_IFDIR | 0755;

		root->path_length = 1;
		root->path = (char *)malloc(sizeof(char)*2);
		strcpy(root->path,"/");
		
		root->name_length = 1;
		root->name = (char *)malloc(sizeof(char)*2);
		strcpy(root->name,"/");

		root->filecount = 0;
		root->fileBlockNumbers = NULL;
		
		root->n_children = 0;
		root->children = NULL;

		root->offset = N_CHILDREN_OFFSET + sizeof(int) + strlen(root->path) + strlen(root->name);

		writeNode(root,root->block_number,NULL);
	}
	return NULL;
}


static int file_mkdir(const char *path,mode_t x) {
	printf("[mkdir] entering");
	//printf("mkdir entering\n");
	
	//printf("path of requested directory = %s\n",path);

	char *temp_path = (char *)malloc(sizeof(char)*strlen(path)); // remove '/' at the beginning
	strncpy(temp_path,path+1,strlen(path));


	if(checkIfDelimiterInPath(temp_path)) {   
		// creating directory not in root     
		//printf("creating child with delimiter\n");
		

		// extract name from path
		int pos = -1;
		int n = strlen(temp_path);
		for(int i=n-1;i>=0;i--) {
			if(temp_path[i] == '/') {
				pos = i;
				break;
			}
		}
		//printf("[mkdir] pos = %d\n",pos);
		char *temp = (char *)malloc(sizeof(char)*(pos+1)); // path of new directory upto its parent
		strncpy(temp,temp_path,pos);
		temp[pos] = '\0';
		//printf("[mkdir] temp = %s\n",temp);

		// check if the directory upto parent is valid

		struct directory *p = checkPath((const char *)temp);

		if(p == NULL) {
			//printf("the path does not exist => NULL\n");
			return -1;
		}
		

		char *tempName = (char *)malloc(sizeof(char)*(n-pos)); // name of new directory 
		strncpy(tempName,temp_path+pos+1,(n-pos));
		
		//printf("tempName = %s\n",tempName);
		
		createChild(p,(char *)path,tempName,x); // create node in the tree
	
	} else {
		// creating a directory within root
		//printf("creating child without delimiter\n");
		createChild(root,temp_path,temp_path,x);	
	}
	return 0;

}


static int file_rmdir(const char *path) {
	printf("rmdir entering\n");
	//printf("rmdir entering\n");
	char *temp_path = (char *)malloc(sizeof(char)*strlen(path));
	strncpy(temp_path,path+1,strlen(path));
	//printf("temp_path = %s\n",temp_path);
	int index = -1;
	struct directory *child = NULL;
	
	if(checkIfDelimiterInPath(temp_path)) {
		// extract name from path
		int pos = -1;
		int n = strlen(temp_path);
		for(int i=n-1;i>=0;i--) {
			if(temp_path[i] == '/') {
				pos = i;
				break;
			}
		}
		char *temp = (char *)malloc(sizeof(char)*(pos+1)); // name of directory
		strncpy(temp,temp_path,pos);
		temp[pos] = '\0';
		//printf("[rmdir] temp = %s\n",temp);
		struct directory *p = checkPath((const char *)temp);
		char *tempName = (char *)malloc(sizeof(char)*(n-pos));
		strncpy(tempName,temp_path+pos+1,(n-pos));
		//printf("[rmdir] tempName = %s\n",tempName);
		if(p == NULL) {
			//printf("path does not exist\n");
			return -1;
		}
		for(int i=0;i<p->n_children;i++) {
			//printf("[rmdir] strcmp(p->children[i]->name,tempName) = %d\n",strcmp(p->children[i]->name,tempName));
			if(strcmp(p->children[i]->name,tempName) == 0) {
				index = i;
				child = p->children[i];
				break;
			}	
		}
		if(index == -1) {
			//printf("file does not exist\n");
			return -1;
		}
		if(child->n_children > 0 || child->filecount > 0) {
			//printf("invalid operation. Recursive delete not allowed\n");
			return -1;
		}
		removeNode(child->block_number,p);

		if(index!=root->n_children-1) {
			p->children[index] = p->children[p->n_children-1];
			p->children[p->n_children-1] = NULL;
		}
		free(child);
		p->n_children-=1;
		printf("deleted directory successfully\n");

	} else {
		for(int i=0;i<root->n_children;i++) {
			if(strcmp(root->children[i]->name,temp_path) == 0) {
				//printf("found child\n");
				index = i;
				child = root->children[i];
				//printf("child->name = %s\n",child->name);
				//printf("child->block_number = %d\n",child->block_number);
				break;
			}	
		}
		if(index == -1) {
			//printf("file does not exist\n");
			return -1;
		}
		if(child->n_children > 0 || child->filecount > 0) {
			//printf("invalid operation. Recursive delete not allowed\n");
			return -1;
		}
		removeNode(child->block_number,root);
		if(index!=root->n_children-1) {
			root->children[index] = root->children[root->n_children-1];
			root->children[root->n_children-1] = NULL;
		}
		free(child);
		root->n_children-=1;
		printf("deleted directory successfully\n");
	}
	return 0;

}

int file_create(const char *path, mode_t mode,struct fuse_file_info * fi){
    printf("[create] called\n");
    //printf("[create] path = %s\n",path);
	//printf("File Create Called\n");
	int k =strlen(path);
	char* c_path = (char *)malloc(sizeof(char)*(k));
	char *tempName;
	strncpy(c_path,path+1,k);
	struct directory *dir;
    if(checkIfDelimiterInPath(c_path)){
    	int pos = -1;
		int n = strlen(c_path);
		for(int i=n-1;i>=0;i--) {
			if(c_path[i] == '/') {
				pos = i;
				break;
			}
		}
		char *temp_path = (char *)malloc(sizeof(char)*(pos+1));
		strncpy(temp_path,c_path,pos);
		temp_path[pos] = '\0';
		//printf("temp = %s\n",temp_path);
		tempName = (char *)malloc(sizeof(char)*(n-pos));
		strncpy(tempName,c_path+pos+1,(n-pos));
		//printf("tempName = %s\n",tempName);
		dir = checkPath((const char *)temp_path);
		if(dir==NULL){
			//printf("Directory not found\n");
			return -1;
		}
	}
	else{
		dir = root;
		tempName = (char *)malloc(sizeof(char)*strlen(c_path+1));
		strcpy(tempName,c_path);
	}
	int x_f = checkfilenode(dir,tempName);
	if(x_f!=0) {
		//printf("File already exists\n");
		return -1;
	}
    struct fileInfo *temp = (struct fileInfo *)malloc(sizeof(struct fileInfo));
    for (int i = 0; i < 8; ++i)
    {
        temp->blockno[i] = -1;
    }
    temp->name = (char *)malloc(sizeof(char)*strlen(tempName)+1);
    strcpy(temp->name,tempName);
    temp->name_length = strlen(tempName);
    temp->path = (char *)malloc(sizeof(char)*strlen(c_path)+1);
    strcpy(temp->path,c_path);
    //printf("[file_create] temp->path = %s\n",temp->path);
    temp->path_length = strlen(c_path);
    temp->fileId = getFreeBlock();
    temp->size=0;
    temp->mode=S_IFREG | mode;
    temp->blockcount=0;
    temp->block_size=BLOCK_SIZE;
    temp->uid=dir->uid;
    temp->gid=dir->gid;
    temp->st_nlink=0;
    if(dir->fileBlockNumbers==NULL){
    	dir->fileBlockNumbers = (int*)malloc(sizeof(int));
    }
    dir->fileBlockNumbers[dir->filecount++]=temp->fileId;
    writefileNode(dir,temp);
    //printf("Creating File..\n");
    return 0;

}	

static int file_open(const char *path, struct fuse_file_info *fi){
	printf("[open] called\n");
    char* c_path = (char *)malloc(sizeof(char)*strlen(path)+1);
	strcpy(c_path,path);
	if(c_path[0]=='/')
		c_path++;

	struct fileInfo *f = find_file(c_path);
	if(f!=NULL){
		printf("[open] f is not null and c_path is %s\n",c_path);
		f->st_nlink++;
		fi->fh = (uint64_t) f;
		return 0;
	}
	else{
		return -ENOENT;
	}
}
static int file_utimes(const char *path, struct utimbuf *t) {
	printf("[utimes] called\n");
	return 0;
}



static int file_read1(const char *path,char *buf,size_t size,off_t offset,struct fuse_file_info *fi) {
	printf("[read] called\n");
	//printf("READING FILE\n");
	int k = strlen(path);
	char *temp_path = (char *)malloc(k);
	strncpy(temp_path,path+1,k);
	//printf("[read] path = %s\n",path);
	//printf("[read] temp_path = %s\n",temp_path);

	struct fileInfo *f = find_file(temp_path);
	if(f==NULL){
		//printf("File not found\n");
		return -ENOENT;
	}
	if(offset > f->size) {
		//printf("Offset is too large\n");
		return 0;
	}
	size_t avail = f->size - offset; //available size
			
  	size = (size < avail) ? size : avail; //Takes the smaller size
		if(f->blockcount>0)
			{
				int i;
				for(i=0;i<f->blockcount;i++)
				{
					if(offset <= ((DATA_BLOCK_SIZE-1)*(i+1))) //gives the block index
						break;
				}
				//Calculating offset within the block with the index returned
				off_t new_off=offset-(DATA_BLOCK_SIZE * i);	
				size_t new=size;
				off_t off=0;
				avail=DATA_BLOCK_SIZE;
				size_t n = (size < avail) ? size : avail; 	

				while(size>0)
				{
					read_block(f->blockno[i],buf,new_off,off, n);
					i=i+1;
					off=off+n;
					size=size-n;
					
					avail=DATA_BLOCK_SIZE;
					
					
	  				n = (size > avail) ? avail : size;
					new_off=0;	
	 			}
					printf("Read complete..\n");
					return new;
			}
		else{
			return 0;
		}
}

static int file_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
	printf("[write] called\n");
	//printf("[write] size = %zu\n",size); // size => number of bytes to write
	//printf("[write] offset = %zu\n",offset); // position to start writing
	int k = strlen(path);
	char *temp_path = (char *)malloc(k);
	strncpy(temp_path,path+1,k);
	//printf("[write] path = %s\n",path);
	//printf("[write] temp_path = %s\n",temp_path);

	struct fileInfo *f = find_file(temp_path);
	if(f==NULL){
		//printf("File not found\n");
		return -ENOENT;
	}
	blkcnt_t req_blocks =ceil( (offset + size + DATA_BLOCK_SIZE - 1) / DATA_BLOCK_SIZE);
	//printf("offset + size + DATA_BLOCK_SIZE - 1 = %lu\n",(offset + size + DATA_BLOCK_SIZE - 1) / DATA_BLOCK_SIZE);
	//printf("[write] req_blocks = %ld\n",req_blocks);
	if(req_blocks<=f->blockcount){
		int i;
		for(i=0;i<f->blockcount;i++)
			{
				if(offset <= ((DATA_BLOCK_SIZE-1)*(i+1)))
					break;
			}
			off_t new_off=offset-(DATA_BLOCK_SIZE * i);	
			size_t avail = DATA_BLOCK_SIZE-new_off;
  			size_t n = (size > avail) ? avail : size;
			size_t new=size;
			off_t off=0;
			int z;
			while(size>0)
			{
				z=write_block(f->blockno[i],buf,new_off,off, n);
				if(z==-1)
				{	
					//printf("No Space");
					return -errno;
				}	
		
				i=i+1;
				off=off+n;
				size=size-n;
				avail = BLOCK_SIZE;
				if(size>0){
					write_size(f->blockno[i],n);
				}
  				n = (size > avail) ? avail : size;
				
					
				new_off=0;	
			}
			if(f->size<offset+new)
				f->size=offset+new;
			update_file(f);
			return new; 
		}
		else{
			blkcnt_t remain_b=req_blocks-f->blockcount;
			int i,x;
			for(i=0;i<remain_b;i++){
				if(super.totalfreeb<1)
					return -ENOENT;
				x=getFreeDataBlock();
				f->blockno[f->blockcount++]=x;
				write_new(x);
			}
			for(i=0;i<f->blockcount;i++)
			{
				if(offset <= ((DATA_BLOCK_SIZE)*(i+1)))
					break;
			}
			
			off_t new_off=offset-(DATA_BLOCK_SIZE * i);	
			size_t avail = DATA_BLOCK_SIZE-new_off;
  			size_t n = (size > avail) ? avail : size;
			size_t new=size;
			off_t off=0;
			int z=0;
			
			
			while(size>0)
			{
				z=write_block(f->blockno[i],buf,new_off,off, n);
				
				if(z==-1)
				{	
					//printf("no space");
					return -errno;
				}	
				
				
				i=i+1;
				off=off+n;
			
				size=size-n;
				avail = DATA_BLOCK_SIZE;
				
				if(size>0)
				{
				write_size(f->blockno[i],n);
				}
  				n = (size > avail) ? avail : size;
					
				new_off=0;	
			}
			if(f->size<offset+new)
				f->size=offset+new;
			update_file(f);
			return new;
		}
}

int file_unlink(const char *path){
	int k = strlen(path);
	char *c_path = (char *)malloc(k);
	strncpy(c_path,path+1,k);
	struct fileInfo *f = find_file(c_path);
	if(f==NULL){
		return -ENOENT;
	}
	for(int i=0;i<f->blockcount;i++){
		BlockBitMap[f->blockno[i]]='0';
	}
	struct directory *dir;
	int x = getFreeDataBlock();
	if(checkIfDelimiterInPath(c_path)){
    	int pos = -1;
		int n = strlen(c_path);
		for(int i=n-1;i>=0;i--) {
			if(c_path[i] == '/') {
				pos = i;
				break;
			}
		}
		char *temp_path = (char *)malloc(sizeof(char)*(pos+1));
		strncpy(temp_path,c_path,pos);
		temp_path[pos] = '\0';
	dir = checkPath((const char *)temp_path);
}
	else{
		dir = root;
	}
	if(dir->filecount>1){
	for(int j=0;j<dir->filecount;j++){
	if(dir->fileBlockNumbers[j] == f->fileId){
		dir->fileBlockNumbers[j] = dir->fileBlockNumbers[dir->filecount-1];
		dir->filecount-=1;
		break;
			}
		}
	}
	else{
		free(dir->fileBlockNumbers);
		dir->filecount = 0;
	}
	unsetDataBit(f->fileId); 
	rm(dir,f->fileId);
	return 0;

}
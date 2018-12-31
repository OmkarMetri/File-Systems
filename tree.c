int checkIfDelimiterInPath(const char *path) {
	char delim = '/';
	for(int i=1;i<strlen(path);i++) {
		if(path[i] == delim) {
			return 1;
		}
	}
	return 0;
}

void createChild(struct directory *parent,char *path,char *name,mode_t x) {
	//printf("entering create child\n");
	struct directory *child = (struct directory *)malloc(sizeof(struct directory));
	//child->offset = N_CHILDREN_OFFSET+sizeof(parent->block_number)+strlen(path)+strlen;
	child->offset = N_CHILDREN_OFFSET+sizeof(int)+strlen(path)+strlen(name);
	child->block_number = getFreeBlock();
	child->index = 0;
	child->n_link = 1;
	child->uid = parent->uid;
	child->gid = parent->gid;
	child->mode = S_IFDIR | x;
	child->path_length = strlen(path);
	child->path = (char *)malloc(sizeof(char)*strlen(path));
	strcpy(child->path,path);
	child->name_length = strlen(name);
	child->name = (char *)malloc(sizeof(char)*strlen(name));
	strcpy(child->name,name);
	

	child->filecount = 0;
	child->fileBlockNumbers = NULL;
	child->n_children = 0;
	child->children = NULL;
	child->parent = parent;
	child->fileBlockNumbers = NULL;
	child->n_children = 0;
	if(parent->children == NULL) {
		parent->children = (struct directory **)malloc(sizeof(struct directory *));
	}
	parent->children[parent->n_children++] = child;
	//printf("name = %s\n",name);
	writeNode(child,child->block_number,parent);
	
}


struct directory *checkPath(const char *path) {
	char *temp = (char *)malloc(sizeof(char)*strlen(path));
	strcpy(temp,path);
	//printf("path : %s\n",path);
	char *token = strtok(temp,"/");
	struct directory *p = root;
	if(token == NULL) {
		//printf("token is null\n");
	}
	while(token!=NULL) {
		if(p->children == NULL) {
			return NULL;
		} else {
			int found = -1;
			for(int i=0;i<p->n_children;i++) {
				/*printf("p->children[i]->name_length = %d\n",p->children[i]->name_length);
				printf("strlen = %ld\n",strlen(p->children[i]->name));
				printf("strcmp(p->children[i]->name,token) = %d\n",strcmp(p->children[i]->name,token));
				printf("p->children[i]->name = %s\n",p->children[i]->name);
				printf("token = %s\n",token);*/
				if(strcmp(p->children[i]->name,token) == 0) {
					found = i;
					break;
				}
			}
			//printf("checkpath found var = %d\n",found);
			if(found == -1) {
				return NULL;
			} else {
				p = p->children[found];
			}
		}
		token = strtok(NULL,"/");
	}
	if(p == NULL) {
		//printf("p is null\n");
	}
	return p;
}


struct fileInfo * find_file(char *temp_path){
	char *tempName;
	struct directory *dir;
	//printf("temp path:%s\n",temp_path);
	if(checkIfDelimiterInPath(temp_path)){
    	int pos = -1;
		int n = strlen(temp_path);
		for(int i=n-1;i>=0;i--) {
			if(temp_path[i] == '/') {
				pos = i;
				break;
			}
		}
		char *temp_path_ch = (char *)malloc(sizeof(char)*pos);
		strncpy(temp_path_ch,temp_path,pos);
		temp_path_ch[pos] = '\0';
		//printf("temp = %s\n",temp_path_ch);
		tempName = (char *)malloc(sizeof(char)*(n-pos));
		strncpy(tempName,temp_path+pos+1,(n-pos));
		//printf("tempName = %s\n",tempName);
		
		dir = checkPath((const char *)temp_path_ch);
		//printf("returned dir :%s\n",dir->name);
		if(dir == NULL){
			return NULL;
		}
	} else {
		dir = root;
		tempName = (char *)malloc(sizeof(char)*strlen(temp_path));
		strcpy(tempName,temp_path);
	}
	
	int x_f = checkfilenode(dir,tempName);
	//printf("file block number:%d\n",x_f);
	if(x_f==0){
		return NULL;
	} else {
		struct fileInfo *f = get_file(x_f);
		return f;
	}
}

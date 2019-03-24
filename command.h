
int dir();
int cd(const char* dest);
int pwd();
int creat_dir(const char* dir_name);
int delete_dir(const char *dir_name);
int ls(char* show);

int client_put(int sockfd,char* msg);
int server_put(int sockfd,char* msg);
int client_get(int sockfd,char* msg);
int server_get(int sockfd,char* msg);


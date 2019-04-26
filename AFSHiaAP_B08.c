#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#define key 17

static const char *dirpath = "/home/schielen/modul2";
char debug[1000];
char charlist[1000] = "qE1~ YMUR2\"`hNIdPzi%^t@(Ao:=CQ,nx4S[7mHFye#aT6+v)DfKL$r?bkOGB>}!9_wV']jcp5JZ&Xl|\\8s;g<{3.u*W-0";
char temp[1000];
char pathf[1000];
char input[1000];
char result;

void encrypt() {
	int init = 0,
        size = strlen(input);
    
    for (init = 0; init < size; init++){
        int i; //index
        char *e, ch = input[init];
        
        if(ch != '/'){
            int j = key%92; //shift
            e = strchr(charlist, ch);
            i = (int)(e - charlist);
            ch = charlist[i+j];
        }
        input[init] = ch;
    }

}

void decrypt() {
	int init = 0,
        size = strlen(input);

    for (init = 0; init < size; init++){
        int i; //index
        char *e, ch = input[init];

        if(ch != '/'){
            int j = key%92; //shift     
            e = strrchr(charlist, ch);
            i = (int)(e - charlist);
            ch = charlist[i-j];
        }
        input[init] = ch;
    }
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
	char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		sprintf(fpath,"%s",dirpath);
	}
	else {
		strcpy(input,path);
		encrypt(input);
		sprintf(fpath, "%s%s", dirpath, input);
	}
	int res = 0; 
	res = lstat(fpath, stbuf);

	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		sprintf(fpath,"%s",dirpath);
	}
	else {
		strcpy(input,path);
		encrypt(input);
		sprintf(fpath, "%s%s", dirpath, input);
	}
	int res = 0;

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
    char fullpath[1000];
		sprintf(fullpath, "%s/%s",fpath,de->d_name); //stat getting fullpath
		stat(fullpath,&st);
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;


		if(strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0 )
		//&& de->d_type != 4
			{
				// struct passwd *pw = getpwuid(st.st_uid);
				// struct group *gr = getgrgid(st.st_gid);
				// if((!strcmp(pw->pw_name, "chipset") || !strcmp(pw->pw_name, "ic_controller")) && !strcmp(gr->gr_name,"rusak"))  
				// {

				// }  				
				strcpy(input,de->d_name); //jadikan global
				decrypt();
				strcpy(de->d_name, input);
				// char final[1000]; //
				// sprintf(final,"%s%s", dirpath,input);
				
				res = (filler(buf, de->d_name, &st, 0));	
				
			}
		if(res!=0) break;
	}

	closedir(dp);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else {
		strcpy(input,path);
		encrypt();
		sprintf(fpath, "%s%s",dirpath,input);
	}
	// int i = strlen(fpath) - 4;
	// if(fpath[i] == '.') fpath[i] = '\0';

	
	int res = 0;
  int fd = 0 ;

	(void) fi;
	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read			= xmp_read,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}

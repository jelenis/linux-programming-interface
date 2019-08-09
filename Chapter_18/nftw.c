#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>


/**
 * The following definiations are taken from ftw.h source code
 */
enum
{
	  FTW_F,                /* Regular file.  */
#define FTW_F         FTW_F
	    FTW_D,                /* Directory.  */
#define FTW_D         FTW_D
	      FTW_DNR,                /* Unreadable directory.  */
#define FTW_DNR         FTW_DNR
	        FTW_NS,                /* Unstatable file.  */
#define FTW_NS         FTW_NS
		  FTW_SL,                /* Symbolic link.  */
# define FTW_SL         FTW_SL
		  /* These flags are only passed from the `nftw' function.  */
		    FTW_DP,                /* Directory, all subdirs have been visited. */
# define FTW_DP         FTW_DP
		      FTW_SLN                /* Symbolic link naming non-existing file.  */
# define FTW_SLN FTW_SLN
};


/* Flags for fourth argument of `nftw'.  */
enum
{
	  FTW_PHYS = 1,                /* Perform physical walk, ignore symlinks.  */
# define FTW_PHYS        FTW_PHYS
	    FTW_MOUNT = 2,        /* Report only files on same file system as the
				                                argument.  */
# define FTW_MOUNT        FTW_MOUNT
	      FTW_CHDIR = 4,        /* Change to current directory while processing it.  */
# define FTW_CHDIR        FTW_CHDIR
	        FTW_DEPTH = 8                /* Report files in directory before directory itself.*/
# define FTW_DEPTH        FTW_DEPTH
};


struct FTW {
	int base;
	int level;

};

int _nftw(const char* dirpath,
		int (*fn) (const char* fpath, const struct stat* sb, int typeflag, struct FTW *ftwbuf),
		int nopenfd, int flags, int level) {
		if (level == nopenfd) return 0;

		struct FTW ftw;	
		ftw.level = level;

		DIR* dirp;
		struct dirent* entryp;
		dirp = opendir(dirpath);		
		if (dirp == NULL)
			return -1;

		while ((entryp = readdir(dirp)) != NULL) {
			char real_path[PATH_MAX];
			struct stat sb;	
			int type;

			strcpy(real_path, dirpath);
			strcat(real_path, "/");	
			ftw.base = strlen(real_path);
			strcat(real_path, entryp->d_name);
			
			// ignore . and .. files
			if (strcmp(entryp->d_name,"..") == 0 || strcmp(entryp->d_name, ".") == 0)
				continue;

			if (stat(real_path, &sb) == -1) {
				type = FTW_DNR;
			} else {
				switch(sb.st_mode & S_IFMT) {
					case S_IFREG:
						type = FTW_F;
						break;
					case S_IFLNK:
						type = FTW_SL;
						break;
					case S_IFDIR:
						type = FTW_D;	
						break;
					default:
						continue;
				}
			}
		      		
			// ignore symlinks
			if (type == FTW_SL && (FTW_PHYS & flags == FTW_PHYS))
				continue;

		
			// default preorder traversal
			if ((FTW_DEPTH & flags) != FTW_DEPTH) {
				// change the directory during processing
				if (FTW_CHDIR & flags == FTW_CHDIR) {
					if (chdir(real_path) == -1)
						return -1;
				}
				(*fn)(real_path, &sb, type, &ftw); 
			}

			if (type == FTW_D && _nftw(real_path, fn, nopenfd, flags, level + 1) == -1) {
				return -1;
			}
			
			// post order traversal
			if ((FTW_DEPTH & flags) == FTW_DEPTH) {
				// change the directory during processing
				if (FTW_CHDIR & flags == FTW_CHDIR) {
					if (chdir(real_path) == -1)
						return -1;
				}
				
				(*fn)(real_path, &sb, type, &ftw); 
			}
		}

		closedir(dirp);
		return 0;


}

int nftw(const char* dirpath,
		int (*fn) (const char* fpath, const struct stat* sb, int typeflag, struct FTW *ftwbuf),
		int nopenfd, int flags) {
		int level = 0;	
		char real_path[PATH_MAX];
		if (realpath(dirpath, real_path) == NULL)
			return  -1;		
		
		return _nftw(real_path, fn, nopenfd, flags, level);
	}
int walk(const char* fpath, const struct stat* sb, int type, struct FTW* ftwbuf) {
	printf("%s\n", fpath );
	return 0;
}


/**
 * 
 *
 */
int main(int argc, char* argv[]) {

	if (argc < 2) {
		printf("need a path to a directory\n");
		return -1;
	}
	
	if (nftw(argv[1], walk, 32, 0) == -1) {
		perror("ntfw");
		return -1;
	}
	return 0;
}

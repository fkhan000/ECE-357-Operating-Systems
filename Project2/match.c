//Fakharyar Khan, Hengnan Ma
//Professor Hakner
//Operating Systems
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

//OS Problem Set 2: File Scavenger System


//This function checks to see if the given file
//is a duplicate of the target file, a hardlink or a softlink
//to the file, a hardlink or a softlink to a duplicate of that file,
//or none of the above. If a match is found, the function 
//lets the user know and if not, the function outputs nothing.
//It takes in a pointer to the dirent structure representing
//the file, a char that gives the path to the folder the file is in
//relative to the folder this C file is in, and the pathname of the
//target file
void match(struct dirent *de, char* dir, char* target)
{

	int fd_m;
	int fd_t;

	char pathname[500];

	//we concatenate the filename and the path of the directory it is in
	//to get a pathname for the candidate file
	snprintf(pathname, sizeof(pathname), "%s/%s", dir, de -> d_name);

	struct stat stats_target;
	struct stat stats_match;

	stat(target, &stats_target);
	int sym_link = 0;

	//if we can't stat the file then there's no way for us
	//to distinguish the file from being a hardlink or
	//a duplicate to the target file so we terminate the function here

	if(lstat(pathname, &stats_match) < 0)
	{
		return;
	}

	//if the file is a symbolic link
	if ((stats_match.st_mode & S_IFMT) == S_IFLNK)
	{
		//we use the readlink system call to get the pathname of the file
		//it points to
		if(readlink(pathname, pathname, 500) == -1)
		{
			//and if that fails we terminate here
			return;
		}

		//then we would stat the file that it points to
		//instead
		if(stat(pathname, &stats_match) < 0)
		{
			return;
		}
		sym_link = 1;
	}

	//if the size of the files don't match
	if (stats_target.st_size != stats_match.st_size)
	{
		//then we don't have a match
		return;
	}

	//if we can't open the file
	if ((fd_m = open(pathname, O_RDONLY)) < 0)
	{
		//then we terminate
		return;
	} 


	char buf_t[4096];
	char buf_m[4096];

	//we checked to see if the target file could be opened
	//before searching so we should be able to open
	//it through the duration of the program

	fd_t = open(target, O_RDONLY);
	int bytes_read_t;
	int bytes_read_m;


 	//while we haven't encountered an error in reading either file and haven't reached the end of both files
	while( ((bytes_read_t = read(fd_t, buf_t, 4096)) > 0) & ((bytes_read_m = read(fd_m, buf_m, 4096)) > 0))
	{

		//we then compare the contents of the buffers byte by byte
		for (int i = 0; i < bytes_read_t; i++)
		{
			if (buf_m[i] != buf_t[i])
			{
				return; 
			}
		}

	}

	//if there was an error in reading either file, we terminate
	if ((bytes_read_t < 0) | (bytes_read_m < 0))
	{
		return; 
	}

	//if the inodes don't match then file is a duplicate
	if (stats_target.st_ino !=stats_match.st_ino)
	{

		if (sym_link == 0)
		{
			printf("\n%s/%s \t DUPLICATE OF TARGET (nlink=%hu)\n", dir, de -> d_name, stats_match.st_nlink);
			printf("\n");
		}

		//if sym link flag was raised 
		else
		{
			//then we have a symbolic link to a duplicate
			printf("\n%s/%s \t SYMLINK (%s) RESOLVES TO DUPLICATE\n", dir, de -> d_name, pathname);
		}
	}

	//else if the inodes match then the file is a hardlink or it's a symlink
	//to the target
	else if (stats_target.st_nlink > 1)
	{
		
		if (sym_link == 0)
		{
			printf("\n%s/%s \t HARD LINK TO TARGET\n", dir, de -> d_name);
		}

		else
		{
			printf("\n%s/%s \t SYMLINK RESOLVES TO TARGET\n", dir, de -> d_name);
		}

	}
	//finally we close the files
	close(fd_t);
	close(fd_m);

}

//This function recursively searches the given directory
//for hardlinks, symbolic links, and duplicates to the target file

void search(char* dir, char* target)
{

	DIR *dirp;
	struct dirent *de;

	//if we're unable to open the directory
	if (! (dirp = opendir(dir)))
	{
		//we let the user know and terminate the function
		fprintf(stderr,"\nWARNING: CAN'T OPEN DIRECTORY %s:%s\n",dir, strerror(errno));
		return;
	}

	//we set errno to 0
	errno = 0;
	char names[500][500];

	int index = 0;

	//while we haven't reached the end of the directory
	while ((de = readdir(dirp)))
	{
		//we skip the . and .. directories since the first one would get us stuck in a loop
		//and the second one would cause us to explore the entire file system instead of just
		//everything under the directory
 		if ((strcmp(de -> d_name, ".") == 0) |  (strcmp(de -> d_name, "..") == 0))
		{
			continue;
		}

		//if it's a directory
		if (de -> d_type == DT_DIR)
		{
			//we add the pathname of the subdirectory to our list of subdirectories
			snprintf(names[index], sizeof(names[index]), "%s/%s", dir, de -> d_name);
			index ++;
		}

		else if ((de -> d_type == DT_REG) | (de -> d_type == DT_LNK))
		{
			//else we check to see if the object is a match to the target file
    		match(de, dir, target);
		}

	}

	//if there was an error reading the directory
	if (errno)
	{
		//we let the user know and move on. We don't terminate here because we even if
		//there was a problem reading a single file, we still want to explore the directories
		//underneath this one
		fprintf(stderr,"\nWARNING: ERROR READING DIRECTORY %s : %s\\n",dir,strerror(errno));
	}

	//then for each subdirectory
	for (int i = 0; i < index; i++)
	{
		//we search through it
		printf("\nSEARCHING DIRECTORY %s\n\n", names[i]);

		//We decided that the output would be more readable and easier to follow
		//if we processed all of the files in the directory before moving to a subdirectory
		//instead of moving into a subdirectory whenever one is found in the while loop above
		search(names[i], target);
	}


	//we then close the current directory
	closedir(dirp);
}

//this function is the main driver for our program
//the first argument, argv[1] to the program, is the 
//directory that the program should search through,
//the second argument is relative pathname of the target file
//returns 0 on success and -1 on failure
int main(int argc, char* argv[])
{	
	int fd;

	//we first check to see if we can open our target file
	if((fd = open(argv[2], O_RDONLY)) < 0)
	{
		//if not this is a fatal error and we terminate the program here
		fprintf(stderr, "UNABLE TO OPEN TARGET FILE: %s", strerror(errno));
		return -1;
	}
	struct stat *buf;

	//we then check to see if we can stat the file
	if (stat(argv[2], buf) < 0)
	{	
		//if we can't we again terminate the program
		fprintf(stderr, "UNABLE TO READ METADATA OF TARGET FILE: %s", strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);

	//else we begin our search through the given directory
	printf("\nSEARCHING DIRECTORY %s\n\n", argv[1]);
	search(argv[1], argv[2]);

	
	return 0;
}
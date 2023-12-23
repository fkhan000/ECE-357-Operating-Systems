#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    
    
    if(argc < 4)
    {
        fprintf(stderr, "Usage: Smear Target Replacement File1 File2 ... \n");
		exit(-1);
    }
    printf("Length of Target: %zu, Length of Replacement: %zu\n", strlen(argv[1]), strlen(argv[2]));
    printf("%s %s\n", argv[1], argv[2]);
   
    if(strlen(argv[1]) != strlen(argv[2]))
    {
        fprintf(stderr, "lengths are not equal\n");
		return 1;
    }
    if(strcmp(argv[1], argv[2]) == 0)
    {
        fprintf(stderr, "Target and replacement are the same\n");
		return 1;
    }
    
    for(int i = 3; argv[i] != NULL; ++i)
    {
        struct stat statbuf;
        int fd = open(argv[i], O_RDWR);
        if (fd == -1) {
            perror("open");
            return 1;
        }
        if (fstat(fd, &statbuf) == -1) {
            perror("stat");
            return 1;
        }
        printf("The size of the file is %ld bytes\n", statbuf.st_size);
        void *map = mmap(NULL, statbuf.st_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
        
        if(map == MAP_FAILED){
            perror("mmap");
            
            return 1;
        }
        char *found_str = map;
        while ((found_str = strstr(found_str, argv[1])) != NULL) {
            memcpy(found_str, argv[2], strlen(argv[2]));
            found_str += strlen(argv[2]);
        }
        if(munmap(map, statbuf.st_size) == -1)
        {
            perror("munmap");
            return 1;
        }
        if (close(fd) == -1) {
            perror("Error closing file");
            return 1;
        }
    }
    printf("SUCESS\n");
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
int wordGen(int numWords, int nc)
{
    srand(time(NULL));
    int fd = open("/dev/tty", O_RDWR);

    if (fd == -1) {
        perror("Error opening /dev/tty");
        exit(1);
    }
    int currentCount = 0;
    int length = 0;
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    while(currentCount < numWords)
    {
        length = rand() % nc + 3;
        char *word = (char *)malloc(length + 1); // +1 for the null terminator
        if (word == NULL) {
            perror("Memory allocation error");
            exit(1);
        }
        
        for (int i = 0; i < length; i++) {
            int randomIndex = rand() % (sizeof(charset) - 1);
            word[i] = charset[randomIndex];
        }
        word[length] = '\0';
        printf("%s\n", word);
        free(word);
        currentCount++;
    }
    // Assuming the integer won't be too large
    char message[128];
    // Convert the integer to a character string
    snprintf(message, sizeof(message), "Finished generating %d candidate words\n", currentCount);

    // Use the write function to write the character string to /dev/tty
    ssize_t bytes_written = write(fd, message, strlen(message));

    if (bytes_written == -1) {
        perror("Error writing to /dev/tty");
    } 

    close(fd);
    return currentCount;
}

int main(int argc, char* argv[]) {

    
    int nc = 10;
    if(argc < 2 || atoi(argv[1]) == 0)
    {
        wordGen(INT_MAX, nc);
    }
    else{
        wordGen(atoi(argv[1]), nc);
    }
    return 0;
}
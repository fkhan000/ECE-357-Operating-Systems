#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#define MAX_WORD_LENGTH 50
typedef struct {
    char word[MAX_WORD_LENGTH];
} DictionaryWord;
int match = 0;
int isEnglishWord(const char *word) {
    for (int i = 0; word[i] != '\0'; i++) {
        if (!isalpha(word[i])) {
            // If the character is not an English alphabet character or is not uppercase, reject the word
            return 0;
        }
    }
    return 1; // The word contains only English alphabet characters
}

int wordSearch(char txt[])
{
    int rejected = 0;
    int fd = open("/dev/tty", O_RDWR);
    if (fd == -1) {
        perror("Error opening /dev/tty");
        exit(1);
    }
    
    
    FILE *dictionary_file = fopen(txt, "r");
    if (!dictionary_file) {
        perror("Error opening dictionary file");
        return 1;
    }

    // Create a dynamic array to store dictionary words
    DictionaryWord *dictionary = NULL;
    size_t dictionary_size = 0;

    // Read and store the dictionary words in uppercase
    char word[MAX_WORD_LENGTH];

    while (fscanf(dictionary_file, "%s", word) != EOF) {
        int i = 0;
        if (!isEnglishWord(word)) {
            rejected++;
            continue;
        } 
        while (word[i] != '\0') {
            word[i] = toupper(word[i]);
            i++;
        }
        DictionaryWord *new_word = (DictionaryWord *)malloc(sizeof(DictionaryWord));
        if (new_word == NULL) {
            perror("Memory allocation error");
            return 1;
        }

        strcpy(new_word->word, word);


        dictionary_size++;
        dictionary = (DictionaryWord *)realloc(dictionary, sizeof(DictionaryWord) * dictionary_size);
        dictionary[dictionary_size - 1] = *new_word;
        free(new_word);
    }
    char message[128];
    snprintf(message, sizeof(message), "Accepeted %zu words, rejected %d\n", dictionary_size, rejected);
    
    // Use the write function to write the character string to /dev/tty
    ssize_t bytes_written = write(fd, message, strlen(message));
    fclose(dictionary_file);
   
    // Read lines from standard input and check if they match dictionary words
    while (fgets(word, MAX_WORD_LENGTH, stdin) != NULL) {
        
       
        int word_length = strlen(word);
        if (word_length > 0 && word[word_length - 1] == '\n') {
            word[word_length - 1] = '\0';
        }

    
        char uppercase_word[MAX_WORD_LENGTH];
        strcpy(uppercase_word, word);

        for (int i = 0; uppercase_word[i]; i++) {
            uppercase_word[i] = toupper(uppercase_word[i]);
        }
        // Check if the word is in the dictionary and echo it
        for (size_t i = 0; i < dictionary_size; i++) {
            if (strcmp(uppercase_word, dictionary[i].word) == 0) {
                printf("%s\n", word);
                match++;
            }
        }
        
        
    }
    

    // Convert the integer to a character string
    snprintf(message, sizeof(message), "Matched %d words\n", match);

    // Use the write function to write the character string to /dev/tty
    write(fd, message, strlen(message));
    if (bytes_written == -1) {
        perror("Error writing to /dev/tty");
    } 

    close(fd);
    free(dictionary);
    return 0;
}

void sigpipe_handler(int signo) {
    (void)signo; // Silence the warning about unused parameter
    fprintf(stderr, "wordsearch: Broken pipe - pager terminated\n");
    fprintf(stderr, "wordsearch: Number of words matched: %d\n", match);
    exit(EXIT_SUCCESS);
}
int main(int argc, char* argv[]) {
    signal(SIGPIPE, sigpipe_handler);
    if(argc != 2)
    {
        printf("Format: <Executable> <dictionary.txt> \n");
        return 1;
    }
    wordSearch(argv[1]);
    
    
    return 0;
}
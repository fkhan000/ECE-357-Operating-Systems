#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

int main(int argc, char * argv [])
{
	//here we used a static array since
	//all of the words printed by wordSearch
	//are less than 20 characters
	char word[20];
	char term_input;
	int index = 0;

	//instead of stdin, the user will interface with the program
	//through the terminal
	FILE * fp;


	if(!(fp = fopen("/dev/tty", "r")))
	{
		//if we're unable to open the terminal, we inform the user and terminate
		fprintf(stderr, "Unable to open /dev/tty: %s\n", strerror(errno));
		exit(-1);
	}


	//we read in a line from standard input
	while(fgets(word, 20, stdin) != NULL)
	{
		//if 23 lines have been written
		if (index == 23)
		{
			//we prompt the user to press return for more
			fputs("---Press RETURN for more---\n", stdout);

			//while we're waiting for the user
			while ((term_input = getc(fp)) != '\n')
			{
				//if they instead press Q (or q), we terminate the program
				if(toupper(term_input) == 'Q')
				{
					fclose(fp);
					return 0;
				}
			}
			//we reset the index counter
			index = 0;
		}
		//echo the line from standard input to standard output
		fputs(word, stdout);
		//and increment
		index ++;
	}
	fclose(fp);
	return 0;
}
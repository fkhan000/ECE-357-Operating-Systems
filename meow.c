//Hengan Ma, Fakharyar Khan
//ECE-357: Computer Operating Systems
//Professor Hakner
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h> 
#include <stdbool.h>


//This function attempts to write the contents of one file into
//another file, the output file. fd is the file descriptor for 
//the input file and fdw is the file descriptor for the output file.
//in_file and out_file are the filenames for the input and output fiel
//respectively. Upon success, the function will write to standard error
//saying that the operation was successful and returns 0. And if it fails
//then it will output to standard error the error that occurred and return -1

int write_to(int fd, int fdw, char* in_file, char* out_file) 
{
	char buf[4096];
	int tot_bytes = 0;
	int tot_lines = 0;
	int bytes_read = 4096;
	int par_bytes = 0;
	int bytes_transferred = 4096;
	char* x = (char*)malloc (200 * sizeof (char));
	bool rewrite;

	//After reaching the end of the file, any further read system calls
	//will return 0 so we can use this to determine if we reached the end of the file
	while(bytes_read != 0)
	{

		par_bytes = 0;
		rewrite = false;

		//we read from the input file and load it into our buffer
		bytes_read = read(fd, buf, 4096); 
		
		//then we write the contents of the buffer into the output file
		bytes_transferred = write(fdw, buf, bytes_read);
		


		if (bytes_transferred < bytes_read)
		{
			//if this occurs then we have a partial write
			//we attempt to write the remaining bytes to the output file
			par_bytes = write(fdw, &buf[bytes_transferred], bytes_read - bytes_transferred);
			//and set rewrite to be true
			rewrite = true;
		}

		//if bytes_transferred or bytes_read is < 0 then we definitely have an error
		//additonally if we attempted a rewrite and were unable to write the remaining bytes to the output file
		if ( (bytes_transferred < 0) || (bytes_read < 0) || (rewrite && (par_bytes != (bytes_read - bytes_transferred))))
		{
			//we write to standard error saying that the file transfer was unsuccessful
			sprintf(x, "\nUnable to transfer contents of %s to %s\n: %s", in_file, out_file, strerror(errno));
			write(2, x, 200);
			//and return -1
			return -1;
		}
		
		//we then update the total number of bytes transferred
		tot_bytes += bytes_read + par_bytes;
		
		//and also update the number of lines written to the output file
		for(int i = 0; i < bytes_transferred; i++)
		{
			if (buf[i] == '\n')
			{
				tot_lines ++;
			}
		}
	}

	//if our input isn't standard input,
	if (fd != 1)
	{
		//we close the file
		close(fd);
	}
	//and then write to standard error that the transfer was successful
	sprintf(x, "\nSuccessfully wrote %d bytes and %d lines from %s to %s\n", tot_bytes, tot_lines, in_file, out_file);
	write(2, x, 200);
	free(x);
	//and return 0 on success
	return 0;
}


//main function reads from the command line to obtain the input and output filenames
//it returns 0 if the program successfully executed and -1 if an error occured

int main(int argc, char*argv[])
{

	int fd;
	int fdw = 1;
	int start = 1;
	char* x = (char*)malloc (200 * sizeof (char));
	char in_file[100];
	char out_file[100] = "Standard Output";
	
	//if the second argument in the commmand line is -o
	//then we have shouldn't update to standard output but instead to an output file
	if (strcmp(argv[1], "-o") == 0)
	{
		//we try to open the output file and delete its contents its content or create the 
		//the output file if it doesn't exist. If we get an error in opening the file. 
		if ( (fdw = open(argv[2],	 O_WRONLY | O_CREAT | O_TRUNC, 0666))     < 0)
        	{

        		//we output an error message to standard error. 
				sprintf(x, "\nCan't open file %s for reading: %s\n", argv[2], strerror(errno));
              	write(2, x, 200);
            	return -1;
        	}
        //else if we were successful, we copy down the name of the output file in  out_file
		strcpy(out_file, argv[2]);
		//and set the index where the input files are to 3
		start = 3; 
	}
	
	//if there are no input files (start >= argc), that means we write to standard input
	//and if unsuccessful we return -1 along with the appropriate error message to standard error
	if ((argc <= start) && write_to(1, fdw, "Standard Input", out_file)  == -1)
	{
		return -1;
	}
	
	//then we iterate through the input files
	for(int i = start; i < argc; i ++) 
	{
		//we set the file descriptor to 0
		fd = 0;

		//if we get the - argument, then we should read from standard input
		if (strcmp(argv[i], "-") == 0)
		{
			//so we set fd to 1
			fd = 1;
			//and copy "Standard Input" to in_file
			strcpy(in_file, "Standard Input");
		}

		//else if our argument is /- then that means our filename is -
		else if(strcmp(argv[i], "/-") == 0)
		{
			//so we're going to set the ith argument to -
			argv[i] = "-";
		}

		//now it's kind of tricky since we have two cases where the argument is -
		//that should be treated differently. So we use the fact that when 
		//we got the - argument, we set the fd to be 1. So if fd != 0
		//then we short circuit and don't try to open anything. Else
		//we do read from the name provided by the ith argument
		if ((fd == 0) && ((fd = open(argv[i], O_RDONLY)) < 0))
		{
			//if fd < 0 then we had an error opening the input file
			//if the error code is ENOENT
			if(errno == ENOENT)
			{
				//the filename was invalid
				sprintf(x, "\n%s appears to be an invalid filename\n", argv[i]);
			}

			//else we say that we were unable to open the file for reading and 
			//output the error message given by errno
			else
			{
				sprintf(x, "\nCan't open file %s for reading: %s\n", argv[i], strerror(errno));
			}
			write(2, x, 200);
			return -1;
			
		}

		//if we aren't reading from standard input
		else if(fd != 1)
		{
			//copy the filename to be read to in_file
			strcpy(in_file, argv[i]);
		}
		//we attempt to write the input file to the output file and if unsuccessful,
		//we return -1
		if( (write_to(fd, fdw, in_file, out_file) == -1))
		{
			return -1;
		}
		
	}

	free(x);

	//if our output wasn't standard output
	if (fdw != 1)
	{
		//we close the file
		close(fdw);
	}
	//and now if we got here we successfully exectued the program
	//and can therefore return 0
	return 0;
}
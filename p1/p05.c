#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int main(int argc, char** argv){
	
	if(argv[1] == NULL){

		//If no file is inputted, then exit
		fprintf(stderr, "%s: %s\n", argv[0], strerror(errno)); 
		exit(1);	

	}

	FILE *fp = fopen(argv[1], "r");
	
	char* memBuffer;

	//Check that file can be opened, if not, exit

	if(fp == NULL){
		//perror("Couldn't open file, errno");
		//Not sure why perror here doesn't work
		fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
		exit(1);
		//error out
	}else{
		//Get file length and create a buffer in memory
		fseek(fp, 0, SEEK_END);
		long fileLength = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		memBuffer = malloc(fileLength);
	
		//If buffer was allocated properly, read file contents
		if(memBuffer){
			fread(memBuffer, 1, fileLength, fp);
			fclose(fp);
		}else{
			free(memBuffer);
			fclose(fp);
			//perror("Couldn't allocate buffer, errno");
			fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
			exit(1);
			//error out
		}
	}

	//Iterate through each substring argument
	for(int i = 2; i < argc; i++){

		//Reset pointer for each new substring
		char* substring = argv[i];
		int count = 0;
		char* pointer = memBuffer;
		
		//Find the earliest instance of each substring and move the pointer one ahead
		while((pointer = strstr(pointer, substring)) != NULL){
			count++;
			pointer++;
		}
			
		printf("%d\n", count);

	}

}

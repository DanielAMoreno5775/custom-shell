#ifndef MAJOR2_H_
#define MAJOR2_H_

//include various libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <regex.h>
#include <unistd.h>

//include custom stuff
#include "hashMap.h"

//create a macro for an infinite loop
#define infinite for(;;)

//create a struct for the tokenized string
//access the components using the -> operator
typedef struct {
	//stores the number of tokens
	int numOfTokens;
	//if the exit command is not present in the line, then this variable will be false
	//if the exit command is present, then this variable will be true
	bool exitCommandExists;   
	//if there are only symbols in the line, then this variable will be true
	//if there is at least one alphanumeric character, then this variable will be false
	bool isEmptyCommandLine;
	//stores the initial string since strtok is destructive
	char * initialString;
	//stores up to 512 tokens (far more than the maximum possible) in an array
	char * arrayOfTokens[512];
	//get number of semicolons
	int numOfCmds;
	//stores up to 512 indices
	int arrayOfCmdIndices[512];
} tokenizedInputLine;

//function prototypes
bool tokenInputHandler(tokenizedInputLine*, HashMap*, HashMap*);
tokenizedInputLine * tokenizeString(char *);
char * getLimitedStringInput(char *, const int, FILE *);
int flushFile(FILE *);
bool checkRegex (char *, char *);
FILE * openBatchFile(char []);
char * aliasCommandCalled (tokenizedInputLine *, int);
void replaceChars(char *, char *, char *);
void manManPage (FILE*);
void aliasManPage (FILE*);
void exitManPage (FILE*);

#endif

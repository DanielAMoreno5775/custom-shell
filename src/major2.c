/**Primary Author: Daniel Moreno
 * CSCE 3600.003
 * Group 1
 * LAST EDITED: OCTOBER 31
 * 
 * Contains functions main, tokenInputHandler, tokenizeString, getLimitedStringInput, flushFile, checkRegex, openBatchFile, manManPage, exitManPage
 * Will take input, look for valid commands, and execute the programs associated with those valid commands
*/

#include "major2.h"

//define maximum line size of 512 bytes
const int MAX_LINE_SIZE = 512;

//program execution begins here
//will create the variables to pass around, calls the input-getting functions, and calls the tokenInputHandler function to check for valid commands
int main(int argc, char* argv[]) { 
	//create initial, default hash maps for custom commands
    	HashMap* defaultCommands = newHashMap();
    	insertKeyInHashMap(defaultCommands, "cd", "cd");
    	insertKeyInHashMap(defaultCommands, "exit", "exit");
    	insertKeyInHashMap(defaultCommands, "path", "path");
    	insertKeyInHashMap(defaultCommands, "myhistory", "myhistory");
    	insertKeyInHashMap(defaultCommands, "alias", "alias");
    	insertKeyInHashMap(defaultCommands, "man", "man");
    
    	//copy over the default variants after creating a new set of hash maps
    	HashMap* variantCommands = newHashMap();
    	duplicateHashMap(variantCommands, defaultCommands);

    	//If no arguments were passed, then set the program into Interactive Mode and will take input from the user
    	if (argc == 1) {
		infinite {   
			//get the user's input up to a maximum size
			char* userInput = getLimitedStringInput("prompt> ", (MAX_LINE_SIZE - 1), stdin); // pass a value one less than I actually want
			if (userInput == NULL) {
				continue;
			}
		    	    
			//if the line is empty, end this iteration and start the next
			if (strlen(userInput) <= 0) {
		    		continue;
			}
		    			
			//assemble a struct of tokens from the input
			tokenizedInputLine *tokens = tokenizeString(userInput);
			if (tokens == NULL) {
				continue;
			}

			//handle the tokens by checking for and executing commands
			bool validCommandWasPresent = tokenInputHandler(tokens, defaultCommands, variantCommands);
			//print error message if no valid command was detected
			if (!validCommandWasPresent) {
				fprintf(stderr, "A valid command was not detected\n");
			}
				    
			//DON'T DELETE THE FREE STATEMENTS as it could cause a memory leak
		    	free(userInput);
		    	free(tokens->initialString);
		    	    	    
		    	//exits the program after line is done if an exit is detected
		    	if (tokens->exitCommandExists) {
		    		printf("Exiting now...\n");
		    		free(tokens);
		    	    	break;
		    	} else {
		    		free(tokens);
		    	}
		}
			    
		return 0;
    	} 
    	//If 1 argument was passed, then set the program into Batch Mode and read in commands from the batch file passed as an argument when the file was executed
    	else if (argc == 2) {
		//create file pointer for the batch file and catch errors
		FILE *batchFile = openBatchFile(argv[1]);
		    	    
		//loop through the file until the end is reached
		while (!feof(batchFile)) {
			//get a line of text from the file up to a maximum size
		    	char* fileInput = (char*)malloc(sizeof(char) * (MAX_LINE_SIZE + 1)); 
		    	if (!fileInput) {
				fprintf(stderr, "Memory allocation error\n");
				continue;
			}
			
			//get the next line and use the line if it exists
		    	if (fgets(fileInput, (MAX_LINE_SIZE + 1), batchFile) != NULL) {
		    		//will remove newline character or EOF character before storing the token or checking it
		    		char *newlineptr = strrchr(fileInput, '\n');
		    		if (newlineptr != NULL) {
		    			*newlineptr = '\0';
		    		}
		    		//if the line is empty, end this iteration and start the next
		    		if (strlen(fileInput) <= 0) {
		    			continue;
		    		}
		    			
		    		//assemble a struct of tokens from the input
			    	tokenizedInputLine *tokens = tokenizeString(fileInput);
			    	if (tokens == NULL) {
			    		continue;
			    	}
		    			
		    		//if the input string did not contain any alphanumeric characters, a pipe, or a redirection operator, then end this iteration and start the next
				if(tokens->isEmptyCommandLine) {
		    			continue;
				}
		    			
		    	    	//echo the line from the file for the user 	
		    	    	printf("Batch Command: %s\n", tokens->initialString);    		
			    		
			    	//handle the tokens by checking for and executing commands
			    	bool validCommandWasPresent = tokenInputHandler(tokens, defaultCommands, variantCommands);
			    	//print error message if no valid command was detected
				if (!validCommandWasPresent) {
					fprintf(stderr, "A valid command was not detected\n");
				}
				    	
				//DON'T DELETE THE FREE STATEMENTS as it could cause a memory leak
				free(tokens->initialString);
				    	
				//exits the program after line is done if an exit is detected
				if (tokens->exitCommandExists) {
				    	printf("Exiting now...\n");
				    	//DON'T DELETE THE FREE STATEMENTS as it could cause a memory leak
		    	    		free(tokens);
				    	break;
				} else {
				    	free(tokens);
				}
			}
		    	    	
		    	//DON'T DELETE THE FREE STATEMENTS as it could cause a memory leak
		    	free(fileInput);
		}
		    	    
		//close the file
		fclose(batchFile);
		return 0;
    	}
    	//Otherwise, the user called the program with invalid syntax and the program will print the correct usage
    	else {
            fprintf(stderr, "Invalid syntax to execute %s.\nValid usage: %s [batchFile]\n", argv[0], argv[0]);
    	}
    
    	return 0; 
} 



//functions below here

//this function handles all of the text after the input/file line has been tokenized
bool tokenInputHandler(tokenizedInputLine * tokens, HashMap* defaultCommands, HashMap* variantCommands) {
	//variable definitions and initializations
	bool validCommandWasPresent = false;
	char dest[512];
	
	//iterate through each token in the inputted line
	for (int cmdIndex = 0; cmdIndex < tokens->numOfCmds; cmdIndex++) {
		int i = tokens->arrayOfCmdIndices[cmdIndex];
		//if the index of the current token to look at is greater than or equal to the number of tokens, break the loop
		if (tokens->numOfTokens <= i) {
			break;
		}
		    		
		char * currentToken = tokens->arrayOfTokens[i];
		//remove the semicolon at the end of a token
		if ((currentToken)[strlen(currentToken)-1] == ';') {
			(currentToken)[strlen(currentToken)-1] = '\0';
		}
		//if the currentToken is too short to reasonably be a command, start the next loop
		if (strlen(currentToken) < 2) {
			continue;
		}
		//replace the token with the associated command, accounting for aliases
		char * value = getKeyInHashMap(variantCommands, currentToken);
		//catch errors created by the getter function and handle the fact that the token might not be an accepted command/alias
		if (value == NULL) {
			continue;
		}
		
		//tokenize the value from the variantCommands hash map
		//a new array must be used as the tokenizeString function's contents are destructive and would prevent value from being used
		strncpy(dest, value, sizeof(dest));
		tokenizedInputLine *valueTokens = tokenizeString(dest);
		
		//if only one token came out of the variantCommands hash map, execute like normal
		//this will only be false when an alias is in use
		if (valueTokens->numOfTokens == 1) {  	
			//Check whether the cd command is present
			if (strcmp(getKeyInHashMap(defaultCommands, value),"cd") == 0) {
				validCommandWasPresent = true;
				printf("CD detected\n");
			}
			//Check whether the man command is present
			else if (strcmp(getKeyInHashMap(defaultCommands, value),"man") == 0) {
				validCommandWasPresent = true;
				//loop through the tokens to find all tokens relevant to the alias command
				int numOfRelevantTokens = 0;
				char *currentToken;
				char finalCharOfToken;
				//if the initial string contains man; , assume that there is only 1 relevant token
				if (checkRegex("man;", tokens->initialString)) {
					numOfRelevantTokens = 1;
				} 
				//If the initial string didn't contain man; , then count the number of relevant tokens
				else {
					for (int manIndex = cmdIndex; manIndex < tokens->numOfTokens; manIndex++) {
						currentToken = tokens->arrayOfTokens[manIndex];
						finalCharOfToken = currentToken[strlen(currentToken) - 1];
						//increment the number of relevant tokens
						numOfRelevantTokens++;
						//if the final character of a token is a semicolon, assume that the end of the command has been reached and exit the loop, even if there are still more tokens left
						if (finalCharOfToken == ';') {
							currentToken[strlen(currentToken) - 1] = '\0';
							break;
						}
					}
				}
					
				//Replicate the standard man results by piping data into less
				FILE *fp = popen("less", "w");
					
				if (fp != NULL) {
					//if there is only 1 relevant token, assume the user only entered "man"
					if (numOfRelevantTokens == 1) {
						//Basic authorship info
						fprintf(fp, "Group 1's Custom Shell\n");
						fprintf(fp, "Made by Samuel Afon, Alexander Evans, Layth Gharbia, and Daniel Moreno\n");
						fprintf(fp, "-----------------------\n");
						//Instructions and Available Commands
						fprintf(fp, "To execute a command, type in the program name and arguments and hit [Enter]\n");
						fprintf(fp, "The following programs are built-in: \n");
						printHashMapKeysToFile(defaultCommands, fp);
						fprintf(fp, "\nUse the custom man command for more information on other built-in programs\n");
					}
					//if there are 2 relevant tokens and the second token is "man", then print the man page for man
					else if ((numOfRelevantTokens == 2) && (strcmp(tokens->arrayOfTokens[cmdIndex+1],"man") == 0)) {
						manManPage (fp);
					}
					//if there are 2 relevant tokens and the second token is "alias", then print the man page for alias
					else if ((numOfRelevantTokens == 2) && (strcmp(tokens->arrayOfTokens[cmdIndex+1],"alias") == 0)) {
						aliasManPage (fp);
					}
					//if there are 2 relevant tokens and the second token is "alias", then print the man page for exit
					else if ((numOfRelevantTokens == 2) && (strcmp(tokens->arrayOfTokens[cmdIndex+1],"exit") == 0)) {
						exitManPage (fp);
					}
				} else {
					fprintf(stderr, "An error occurred when accessing the more command\n");
				}
					
				//close the pointer to the less command
				pclose(fp);
			}
			//Check whether the exit command is present
			else if (strcmp(getKeyInHashMap(defaultCommands, value),"exit") == 0) {
			    	validCommandWasPresent = true;
			    	int numOfRelevantTokens = 0;
			    	//iterate through all tokens starting at the exit command and until a semicolon or end of string
			    	for (int exitIndex = i; exitIndex < tokens->numOfTokens; exitIndex++) {
					char finalCharOfToken = (tokens->arrayOfTokens[exitIndex])[strlen(tokens->arrayOfTokens[exitIndex]) - 1];
					//increment the number of relevant tokens
					numOfRelevantTokens++;
					//if the final character of a token is a semicolon, assume that the end of the command has been reached and exit the loop, even if there are still more tokens left
					if (finalCharOfToken == ';') {
						break;
					}
				}
			    	//Check the initial string to see if it contains exit; as that will catch a few special cases where the number of relevant tokens gets an incorrect count
			    	if (numOfRelevantTokens == 1 || checkRegex("exit; ", tokens->initialString)) {
				    	//indicates that an exit was detected for later via a flag
				    	tokens->exitCommandExists = true;
			    	} else {
			    		fprintf(stderr, "Invalid syntax for exit command which doesn't take any arguments\n");
			    	}
			    				
			}
			//Check whether the path command is present
			else if (strcmp(getKeyInHashMap(defaultCommands, value),"path") == 0) {
			    validCommandWasPresent = true;
				printf("Path detected\n");
			}
			//Check whether the myhistory command is present
			else if (strcmp(getKeyInHashMap(defaultCommands, value),"myhistory") == 0) {
			    validCommandWasPresent = true;
				printf("Myhistory detected\n");
			}
			//Check whether the alias command is present
			else if (strcmp(getKeyInHashMap(defaultCommands, value),"alias") == 0) {
			    validCommandWasPresent = true;
			    char* finalResultOfAlias = (char*)malloc(sizeof(char) * (MAX_LINE_SIZE + 1)); 
			    if (!finalResultOfAlias) {
			    		fprintf(stderr, "Memory allocation error\n");
			    		continue;
			    }

				//alias;
			    if (tokens->arrayOfCmdIndices[cmdIndex+1] >= 0) {
				    int distanceBetweenNextAndCurrentCmdIndices = tokens->arrayOfCmdIndices[cmdIndex+1] - tokens->arrayOfCmdIndices[cmdIndex];
					if (distanceBetweenNextAndCurrentCmdIndices == 1) {
						//print all of the aliases stored in variantCommands that are not in defaultCommands
						int printedEntries = printHashMapDifference(variantCommands, defaultCommands);
						if (printedEntries == 0) {
							fprintf(stderr, "No aliases are present\n");
						}
						continue;
					}
			    }

			    	finalResultOfAlias = aliasCommandCalled (tokens, i);
			    	//catch errors before the string is tokenized or I attempt to get the addToI value
			    	if (finalResultOfAlias == NULL) {
					continue;
	    			}
				if (finalResultOfAlias[0] == '-') {
				    	fprintf(stderr, "Invalid syntax for alias command\n");
				    	fprintf(stderr, "alias [-c/-r/<alias_name>='<command>']\n");
				    	continue;
				}
			    	//assemble a struct of tokens from the string passed from the aliasCommandCalled function
			    	tokenizedInputLine *aliasTokens = tokenizeString(finalResultOfAlias);
			    	if (aliasTokens == NULL) {
			    		continue;
			    	}
				int addToI = atoi(aliasTokens->arrayOfTokens[1]);
				//alias without arguments
				if (strcmp(aliasTokens->arrayOfTokens[0],"0") == 0) {
					//print all of the aliases stored in variantCommands that are not in defaultCommands
					int printedEntries = printHashMapDifference(variantCommands, defaultCommands);
					if (printedEntries == 0) {
						fprintf(stderr, "No aliases are present\n");
					}
				}
				//alias -c; so reset all aliases
				else if (strcmp(aliasTokens->arrayOfTokens[0],"1") == 0) {
				    	//reset variantCommands by copying defaultCommands over variantCommands
				    	duplicateHashMap(variantCommands, defaultCommands);
				    			
				}
				//alias -r alias_name;
				else if (strcmp(aliasTokens->arrayOfTokens[0],"2") == 0) {
				    	//remove the specified key from variantCommands
				    	int removalResult = removeKeyInHashMap(variantCommands, aliasTokens->arrayOfTokens[2]);
				    	//print error message if that alias was not found
				    	if (removalResult == -1) {
				    		fprintf(stderr, "%s is not a valid alias\n", aliasTokens->arrayOfTokens[2]);
				    	}
				}
				//alias alias_name='cd ..';
				else if (strcmp(aliasTokens->arrayOfTokens[0],"3") == 0) {
				    	//create the aliasBuffer character array
				    	char aliasBuffer[512] = "";
				    	//iterate through the aliasTokens to get the new command and concatenate it on to the buffer
				    	for (int aliasIndex = 3; aliasIndex < (addToI + 2); aliasIndex++) {
				    		strncat(aliasBuffer, aliasTokens->arrayOfTokens[aliasIndex], 128);
				    		strncat(aliasBuffer, " ", 2);
				    	}
				    	//remove the final character (a space) of the aliasBuffer 
					aliasBuffer[strlen(aliasBuffer) - 1] = '\0';
					//add the new alias and its command to the hash map
				    	insertKeyInHashMap(variantCommands, aliasTokens->arrayOfTokens[2], aliasBuffer);			    					    		
				}
				i = i + addToI - 1;
				//DON'T DELETE THE FREE STATEMENTS as it could cause a memory leak
	    	    	    	free(aliasTokens->initialString);
	    	    	    	free(aliasTokens);
			}
			else {
				//does nothing except prevent errors by simply existing
			}
		} 
		//this will only execute if an alias was in use, such that a single token would become a multi-token string
		else {
			validCommandWasPresent = tokenInputHandler(valueTokens, defaultCommands, variantCommands);
		}
	}
	
	return validCommandWasPresent;
}

//assemble a struct for the tokenized strings from a C-string pointer
tokenizedInputLine * tokenizeString (char *input) {
	//create struct and store initial values
    tokenizedInputLine *tokens = malloc(sizeof(tokenizedInputLine));
    if (!tokens) {
		fprintf(stderr, "Memory allocation error\n");
		return NULL;
    }

    tokens->numOfTokens = 0;
    tokens->numOfCmds = 1;
    tokens->exitCommandExists = false;
    tokens->isEmptyCommandLine = true;
    tokens->initialString = strdup(input);
	for (int i = 0; i < 512; i++) {
    	tokens->arrayOfTokens[i] = "";
    }
    for (int i = 0; i < 512; i++) {
    	tokens->arrayOfCmdIndices[i] = 0;
    }
    
    //checks whether the input string contains any alphanumeric characters, a pipe, or a redirection operator
    if(checkRegex("[A-Za-z0-9|<>]", input)) {
    	tokens->isEmptyCommandLine = false;
    }

	//replace any instances of a semicolon with a semicolon-space so that the string-based tokenizer will work
    replaceChars(input, ";", "; ");
    
    //get initial token
    char *p = strtok (input, " ");
    //while there are remaining tokens, keep looping
    while (p != NULL) {
		//will remove newline character before storing the token or checking it
		char *newlineptr = strrchr(p, '\n');
		if (newlineptr != NULL) {
			*newlineptr = '\0';
		}
		//store the token in the struct
		tokens->arrayOfTokens[tokens->numOfTokens] = strdup(p);
		//store 0 as the first index to consider as command
		tokens->arrayOfCmdIndices[0] = 0;
		//see if the token ends with a semicolon
		char * currentToken = tokens->arrayOfTokens[tokens->numOfTokens];
		if ((currentToken)[strlen(currentToken)-1] == ';') {
			//since the token ends with a semicolon, indicate that the next command to look at is the next index
			tokens->arrayOfCmdIndices[tokens->numOfCmds] = tokens->numOfTokens + 1;
			tokens->numOfCmds += 1;
		}
		//increment the number of tokens
		tokens->numOfTokens = tokens->numOfTokens + 1;
		//get the next token
		p = strtok (NULL, " ");
    }
    
    //free up the temporary pointers and return the struct
    free(p);
    return tokens;
}

//returns an user-inputted string based on constraint set by SIZE (1 byte greater than SIZE)
//will loop infinitely until the user inputs a value within the passed SIZE constraint
char * getLimitedStringInput(char *prompt, const int SIZE, FILE *file) {
	//set actual size with 2 additional bytes of memory added as buffer
	int actualSize = SIZE + 2;

	//execute this code if the passed file is stdin
	if (file == stdin) {
		//infinite loop until the function returns using a defined macro constant
		infinite {
			//allocate memory which will reset with each loop iteration
			char* buffer = (char*)malloc(sizeof(char) * (actualSize)); 
			if (!buffer) {
				fprintf(stderr, "Memory allocation error\n");
				continue;
			}

			//flush anything already inside of the input buffer
			flushFile(file);
			//prompt user
			printf("%s", prompt);
			//store input in buffer while preventing array overflows & only continue executing if it actually got something
			if (fgets(buffer, actualSize, file) != NULL) {
				//add 2 to strlen as it doesn't count \n and the fact that \n has to be counted for the maximum
				if ((strlen(buffer) + 2) >= SIZE) {
					if (buffer[strlen(buffer) - 1] != '\n') {
						fprintf(stderr, "Input was too long (over 512 characters including the '\\n')\n");
						continue;
					}
				}
				//flush anything left in the input buffer for future input operations
				flushFile(file);
				return buffer;
			}

			//free up the allocated memory before restarting the loop if return statement wasn't triggered
			free(buffer);
		}
	} 
	//for every other file, do this
	else {
		//allocate memory which will reset with each loop iteration
		char* buffer = (char*)malloc(sizeof(char) * (actualSize)); 
		if (!buffer) {
			fprintf(stderr, "Memory allocation error\n");
			return NULL;
		}
		//store input in buffer while preventing array overflows & only continue executing if it actually got something
		if (fgets(buffer, actualSize, file) != NULL) {
			//add 2 to strlen as it doesn't count \n and the fact that \n has to be counted for the maximum
			if ((strlen(buffer) + 2) >= SIZE) {
				if (buffer[strlen(buffer) - 1] != '\n') {
					fprintf(stderr, "Input was too long (over 512 characters include the '\\n')\n");
					return "Error";
				}
			}
			return buffer;
		}

		//free up the allocated memory before restarting the loop if return statement wasn't triggered
		free(buffer);
	}
	return "This statement should NEVER execute";
}

//make specified file non-blocking, read all input from it, and flush it
//won't prompt user for input as it flushes the stdin file
int flushFile (FILE *file) {
	//Quickly flush the input file which only works on some OS
	//Since this only works on some OSs, the rest of this function was created
	fflush(file);

	//variable definitions
	int dummyChar;
	int flags;
	int fileDescriptor;
	//get the file descriptor for specified file
	fileDescriptor = fileno(file);
		    
	//gets the file status flags or returns the function if there is an error
	if ((flags = fcntl(fileDescriptor, F_GETFL, 0)) < 0) {
		printf("Error opening file descriptor: %d.\n", fileDescriptor);
		return -1;
	}
		    
	//The F_SETFL macro sets the file status flags to the values of the flags variable
	//The logical OR operator combines the bit values of the flags variable and non-blocking flag
	//return -1 (an error condition) if the fcntl returns anything except 0 (a success) when trying to alter the file's flags
	if (fcntl(fileDescriptor, F_SETFL, flags | O_NONBLOCK)) {
		printf("Error changing file flags for file descriptor: %d.\n", fileDescriptor);
		return -1;
	}
		    
	//iterate through the file until the end-of-file is reached and get each character in the file, thereby moving the cursor forward or emptying the file
	//post-test loop so will execute at least once before checking whether the end of the file has been reached, preventing rare issues where the user is still prompted for input
	do {
		dummyChar = fgetc(file);
	} while (dummyChar != EOF);
		    
	//Clears the end-of-file and error bits so that the file becomes readable again
	clearerr(file);
		    
	//return -1 (an error condition) if the fcntl returns anything except 0 (a success) when trying to reset the file's flags
	if (fcntl(fileDescriptor, F_SETFL, flags)) {
		printf("Error resetting file flags for file descriptor: %d.\n", fileDescriptor);
		return -1;
	}

	return 0;
}

//will return a boolean depending on whether the regex pattern exists in the passed string
bool checkRegex (char *passedRegex, char *strToCheck) {
	//variable definitions
	regex_t rx;
	int result;
	
	//create and compile the regex statement
	result = regcomp(&rx, passedRegex, 0);
	
	//compare pattern to target string
	result = regexec(&rx, strToCheck, 0, NULL, 0);
	
	//If the result variable is 0, the pattern in passedRegex was found in the strToCheck
	if (result == 0) {
		return true;
	} 
	//When the result and the REG_NOMATCH constants are equivalent, then a match was not found
	else if (result == REG_NOMATCH) {
		return false;
	} 
	//Otherwise, an error occurred and the program assumes that a match was not found
	else {
		printf("Some error occured when checking the contents of the token\n");
		return false;
	}

	return false;
}

//try to open a file of the passed name and return the file if it succeeded or exit the program if it failed
FILE * openBatchFile(char fileName[]) {
	//Open the file in read-only mode
    	FILE* file = fopen(fileName, "r");
    	if(file == NULL) {
        	fprintf(stderr, "The batch file %s does not exist or cannot be opened.\n", fileName);
        	exit(1);
    	} else if (!file) {
    		fprintf(stderr, "The batch file %s does not exist or cannot be opened.\n", fileName);
        	exit(1);
    	} else if (ferror(file)) {
    		fprintf(stderr, "The batch file %s does not exist or cannot be opened.\n", fileName);
        	exit(1);
    	}
    	return file;
}

//find every instance of the search regex pattern in str and swap if with the replace string
//example usage: char temp[512] = "Hello;world this;is;a;random test string more; text\n";        replaceChars(temp, ";", "; ");         printf(temp);
void replaceChars(char *str, char *search, char *replace) { 
	//variable definitions and initializations
	regex_t regex;
	regmatch_t pmatch[1];
	size_t nmatch = 1;
	size_t offset = 0;
	size_t newOffset = 0;
	size_t replacementStrLen = strlen (replace);
	size_t strLength = strlen (str);
	size_t strPartsLen = 0;
	char newLine[1024];
	char strParts[1024];
	
	//initialize newLine to an empty string
	strcpy (newLine, "");

	//create and compile the regex statement and check whether it succeeded
	if (regcomp (&regex, search, REG_EXTENDED) == 0) {
		//while the offset is less than the length of the passed strength and compare passed regex pattern to the passed string
		//some offset values are used so that all instances of it are found without finding stuff that has already been found
		while ((offset < strLength) && (regexec(&regex, str + offset, nmatch, pmatch, 0) == 0)) {
			//copy everything after the offset into strParts, null terminate it, and get the new length
			strncpy (strParts, &str[offset], pmatch[0].rm_so);
			strParts[pmatch[0].rm_so] = '\0';
			strPartsLen = strlen(strParts);
			
			//if the offset of the beginning of the substring is larger than 0
			if (pmatch[0].rm_so > 0){
				//concatenate strParts on to newLine
				strcat (newLine, strParts);
				//increase the newOffset
				newOffset += strPartsLen;
				//null terminate newLine's string
				newLine[newOffset] = '\0';
			}
			
			//concatenate the passed replacement substring on to newLine
			strcat (newLine, replace);
			//increase the newOffset
			newOffset += replacementStrLen;
			//null terminate newLine's string
			newLine[newOffset] = '\0';

			//increase offset by the offset of the end of the substring
			offset += pmatch[0].rm_eo;
		}
		
		//if the current offset is less than the passed string's length
		if (offset < strLength) {
			//concatenate everything after the offset in str on to newLine
			strcat (newLine, &str[offset]);
			//increase newOffset by str's length minus the offset
			newOffset += (strLength - offset);
			//null terminate newLine
			newLine[newOffset] = '\0';
		}
	}
	else {
		printf ("regcomp failed\n");
	}
	
	//free up regex
	regfree (&regex);
	
	//store newLine into first newOffset bytes of str
	memmove (str, newLine, newOffset);
	//null terminate it
	memset (str + newOffset, '\0', 1);
} 

//print the man page for the man command to the passed file
void manManPage (FILE* fp) {
	fprintf(fp, "Manual pager utils\n");
	//NAME
	fprintf(fp, "\nNAME\n");
	fprintf(fp, "\tman - an interface to the system reference manuals\n");
	//SYNOPSIS
	fprintf(fp, "\nSYNOPSIS\n");
	fprintf(fp, "\tman <built-in program>\n");
	//DESCRIPTION
	fprintf(fp, "\nDESCRIPTION\n");
	fprintf(fp, "\tman  is  the  system’s manual pager. Unlike the standard man utility, a built-in program should be passed as an argument to this version of man. This version of the man command will function as internal documentation for the custom shell.\n");
	fprintf(fp, "\n\tA manual page consists of several sections.\n");
	fprintf(fp, "\n\tConventional section names include NAME, SYNOPSIS, CONFIGURATION, DESCRIPTION, OPTIONS, EXIT STATUS, RETURN VALUE, ERRORS, ENVIRONMENT, FILES, VERSIONS, CONFORMING TO, NOTES, BUGS, EXAMPLE, AUTHORS, and SEE ALSO.\n");
	//EXAMPLES
	fprintf(fp, "\nEXAMPLES\n");
	fprintf(fp, "\tman alias\n");
	fprintf(fp, "\t\tDisplay the manual page for the item (program) alias.\n");
	//AUTHOR of the command
	fprintf(fp, "\nAUTHOR\n");
	fprintf(fp, "\tDaniel Moreno\n");
}

//print the man page for the exit command to the passed file
void exitManPage (FILE* fp) {
	fprintf(fp, "Manual pager utils\n");
	//NAME
	fprintf(fp, "\nNAME\n");
	fprintf(fp, "\texit — cause the shell to exit\n");
	//SYNOPSIS
	fprintf(fp, "\nSYNOPSIS\n");
	fprintf(fp, "\texit\n");
	//DESCRIPTION
	fprintf(fp, "\nDESCRIPTION\n");
	fprintf(fp, "\tThe exit program shall cause the shell to exit from its current execution environment. Since current execution environment is a subshell environment, the shell shall exit from the subshell environment and continue in the environment from which that subshell environment was invoked.\n");
	fprintf(fp, "\n\tIf there are other valid commands on the same line as exit, those programs will executed before the exit is executed\n");
	//OPTIONS
	fprintf(fp, "\nOPTIONS\n");
	fprintf(fp, "\tNone\n");
	//EXAMPLES
	fprintf(fp, "\nEXAMPLES\n");
	fprintf(fp, "\texit\n");
	fprintf(fp, "\t\tExits the current custom execution environment\n");
	fprintf(fp, "\n\talias; myhistory; exit\n");
	fprintf(fp, "\t\tWill execute the alias and myhistory commands before exiting the current custom execution environment\n");
	//STDOUT
	fprintf(fp, "\nSTDOUT\n");
	fprintf(fp, "\tNot used except to print a status message of \"Exiting now ...\"\n");
	//AUTHOR of the command
	fprintf(fp, "\nAUTHOR\n");
	fprintf(fp, "\tDaniel Moreno\n");
}

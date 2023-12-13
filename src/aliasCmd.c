/**Primary Author: Daniel Moreno
 * CSCE 3600.003
 * Group 1
 * LAST EDITED: OCTOBER 31
 * 
 * Contains functions aliasCommandCalled, aliasManPage
 * Will evaluate the tokens to identify the relevant tokens, determine the nature of the alias command, and return the pertinent info 
 * Also contains the man page for the alias command
*/

#include "major2.h"

//evaluates the tokens after the alias token to identify the significant tokens and ensure proper syntax
char * aliasCommandCalled (tokenizedInputLine *tokens, int cmdPosition) {
	//initialize and define variables
	char * finalResult = {"-"};
	int numOfRelevantTokens = 0;
	int sizeOfRelevantTokens = 0;
	char *currentToken;
	char finalCharOfToken;
	
	//loop through the tokens to find all tokens relevant to the alias command
	for (int i = cmdPosition; i < tokens->numOfTokens; i++) {
		currentToken = tokens->arrayOfTokens[i];
		finalCharOfToken = currentToken[strlen(currentToken) - 1];
		//increment the number of relevant tokens
		numOfRelevantTokens++;
		//store number of relevant tokens
		sizeOfRelevantTokens = sizeOfRelevantTokens + sizeof(currentToken);
		//if the final character of a token is a semicolon, assume that the end of the command has been reached and exit the loop, even if there are still more tokens left
		if (finalCharOfToken == ';' || finalCharOfToken == '\'') {
			currentToken[strlen(currentToken) - 1] = '\0';
			break;
		}
	}
	
	//if the last-checked token is the command that triggered this function, then no arguments were passed
	if (numOfRelevantTokens == 1) {
		//convert int to character
		char buffer[32];
		snprintf(buffer, sizeof(buffer), "%d", numOfRelevantTokens);
		//allocate space for finalResult and concatenate in info
		finalResult = "";
		finalResult = (char *)calloc(1, 4);
		if (!finalResult) {
			fprintf(stderr, "Memory allocation error\n");
			return NULL;
    		}
		strcat(finalResult, "0 ");
		strcat(finalResult, buffer);
		strcat(finalResult, " ");
	}
	//check whether there are at least two tokens to prevent segmentation faults without worrying about short-circuit evaluation
	else if (numOfRelevantTokens >= 2) {
		//if the token after the triggering command is "-c", then set the final result
		if ((numOfRelevantTokens == 2) && (strcmp(tokens->arrayOfTokens[cmdPosition + 1],"-c") == 0)) {
			//convert int to character
			char buffer[32];
			snprintf(buffer, sizeof(buffer), "%d", numOfRelevantTokens);
			//allocate space for finalResult and concatenate in info
			finalResult = "";
			finalResult = (char *)calloc(1, 4);
			if (!finalResult) {
				fprintf(stderr, "Memory allocation error\n");
				return NULL;
    			}
			strcat(finalResult, "1 ");
			strcat(finalResult, buffer);
			strcat(finalResult, " ");
		}
		//if the token after the triggering command is "-r", then set the final result
		else if ((numOfRelevantTokens == 3) && (strcmp(tokens->arrayOfTokens[cmdPosition + 1],"-r") == 0)) {
			//convert int to character
			char buffer[32];
			snprintf(buffer, sizeof(buffer), "%d", numOfRelevantTokens);
			//reset finalResult
			finalResult = "";
			//allocate space for <alias_name> + command indicator + space + null terminator
			finalResult = (char *)calloc(1, strlen(tokens->arrayOfTokens[cmdPosition + 2]) + 3);
			if (!finalResult) {
				fprintf(stderr, "Memory allocation error\n");
				return NULL;
    			}
			//append command indicator and <alias_name>
			strcat(finalResult, "2 ");
			strcat(finalResult, buffer);
			strcat(finalResult, " ");
			strcat(finalResult, tokens->arrayOfTokens[cmdPosition + 2]);
		}
		//if there are at least 2 relevant tokens and the second token contains the pattern of =' with characters on either side
		else if ((numOfRelevantTokens >= 2) && checkRegex(".*='.*", tokens->arrayOfTokens[cmdPosition + 1])) {
			//convert int to character
			char buffer[32];
			snprintf(buffer, sizeof(buffer), "%d", numOfRelevantTokens);
			//reset finalResult
			finalResult = "";
			//allocate space for everything
			finalResult = (char *)calloc(1, sizeOfRelevantTokens + numOfRelevantTokens);
			if (!finalResult) {
				fprintf(stderr, "Memory allocation error\n");
				return NULL;
    			}
			//append command indicator and <alias_name>
			strcat(finalResult, "3");
			//cut up the second token around the equal sign and get rid of the single quote
			char newAlias[100] = {0};
			char cmdToTarget[100] = {0};
			sscanf(tokens->arrayOfTokens[cmdPosition + 1], "%[^=]=%s", newAlias, cmdToTarget);
			cmdToTarget[0] = ' ';
			//iterate through the relevant tokens and start appending the stuff to the final result
			strcat(finalResult, " ");
			strcat(finalResult, buffer);
			strcat(finalResult, " ");
			strcat(finalResult, newAlias);
			strcat(finalResult, " ");
			strcat(finalResult, cmdToTarget);
			//get the final position for the following for loop
			int tempFinalPosition = cmdPosition + numOfRelevantTokens;
			//iterate through the command tokens and concatenate them into the string
			for (int i = (cmdPosition + 2); i < tempFinalPosition; i++) {
				strcat(finalResult, " ");
				strcat(finalResult, tokens->arrayOfTokens[i]);
				//look for the closing single quote to mark the end of the command
				if (finalResult[strlen(finalResult) - 1] == '\'') {
					//remove the final character (a single quote) of the finalResult 
					finalResult[strlen(finalResult) - 1] = '\0';
					//exit the loop
					break;
				}
			}
		}
		else {
			finalResult = "-"; //indicates invalid syntax
		}
	}
	else {
		finalResult = "-"; //indicates invalid syntax
	}
	
	//DON'T DELETE THE FREE STATEMENTS as it could cause a memory leak
	free(currentToken);
	return finalResult;
}

//print the man page for the alias command to the passed file
void aliasManPage (FILE* fp) {
	fprintf(fp, "Manual pager utils\n");
	//NAME
	fprintf(fp, "\nNAME\n");
	fprintf(fp, "\talias — define or display aliases\n");
	//SYNOPSIS
	fprintf(fp, "\nSYNOPSIS\n");
	fprintf(fp, "\talias\n");
	fprintf(fp, "\n\talias -c\n");
	fprintf(fp, "\n\talias -r [alias_name]\n");
	fprintf(fp, "\n\talias [alias_name[='string']]\n");
	//DESCRIPTION
	fprintf(fp, "\nDESCRIPTION\n");
	fprintf(fp, "\tThe alias utility shall create, remove, or redefine alias definitions or write the values of existing alias definitions to standard output. An alias definition provides a string value that shall replace a command name when it is encountered\n");
	fprintf(fp, "\n\tAn alias definition shall affect the current shell execution environment and the execution environments of the subshells of the current shell. The alias definition shall not affect the parent process of the current shell nor any utility environment invoked by the shell\n");
	//OPTIONS
	fprintf(fp, "\nOPTIONS\n");
	fprintf(fp, "\t-c\n");
	fprintf(fp, "\t\tClears all created aliases\n");
	fprintf(fp, "\n\t-r\n");
	fprintf(fp, "\t\tRemoves the specified alias\n");
	//EXAMPLES
	fprintf(fp, "\nEXAMPLES\n");
	fprintf(fp, "\talias\n");
	fprintf(fp, "\t\tDisplay all created aliases\n");
	fprintf(fp, "\n\talias -c\n");
	fprintf(fp, "\t\tClears all created aliases\n");
	fprintf(fp, "\n\talias -r testName\n");
	fprintf(fp, "\t\tRemoves the created alias of 'testName'\n");
	fprintf(fp, "\n\talias testName='path + ./bin'\n");
	fprintf(fp, "\t\tCreates an alias of 'testName' which will trigger the command 'path + ./bin' when it is entered\n");
	fprintf(fp, "\t\tIf an alias of 'testName' already exists, then this will overwrite that entry of 'testName'\n");
	//STDOUT
	fprintf(fp, "\nSTDOUT\n");
	fprintf(fp, "\tThe format for displaying aliases (when no operands or only name operands are specified) shall be:\n");
	fprintf(fp, "\t\t\"%%s=%%s\\n\", alias, command\n");
	fprintf(fp, "\tThe value string shall be written with appropriate quoting so that it is suitable for reinput to the shell.\n");
	//AUTHOR of the alias command
	fprintf(fp, "\nAUTHOR\n");
	fprintf(fp, "\tDaniel Moreno\n");
}

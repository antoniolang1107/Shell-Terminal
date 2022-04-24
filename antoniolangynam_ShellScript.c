#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdbool.h>

/*
    Program Name: antoniolangynam_ShellScript.c
    Authors: Antonio Lang, Yoonsung Nam
    Purpose: Simulate the functionality of a shell using the C language
*/

void promptUser(bool isBatch);
void printError();
int parseInput(char *input, char *splitWords[]);
char *redirectCommand(char *special, char *cmd, bool *isRedirect, char *tokens[], char *outputTokens[]);
char *executeCommand(char *cmd, bool *isRedirect, char* tokens[], char* outputTokens[],  bool *isExits);
void printHelp(char* tokens[], int numTokens);
bool exitProgram(char *tokens[], int numTokens);
void launchProcess(char *tokens[], int numTokens, bool isRedirect);
void changeDirectories(char *tokens[], int numTokens);

int main(int argc, char*argv[]) {
    bool isBatch, continueShell = true, isRedirect = false;
    char input[50], c[500];
    char *temp;
    char *inputTokens, *outputFile, *fileData, *inputCopy;
    int result, exitStatus;
    FILE *fp, *fpw;
    
    if (argc == 2) {
        isBatch = true;
        // if the given batchfile exists, run its commands
        if (access(argv[1], F_OK) == 0) { 
            fp = fopen(argv[1], "r");
            while(fgets(input, sizeof(input), fp)){
                    inputCopy = strdup(input);
                    inputTokens = strtok(inputCopy, " ");
                    outputFile = executeCommand(input, &isRedirect, &inputTokens, &fileData, &continueShell);
            }

            fclose(fp);
        }
        else {
            printError();
            printf("File does not exist\n");
        }
    }
    else if (argc > 2) {
        printError();
        printf("Too many arguments passed\n");
    }
    else if (argc < 2){
      
       do {
            // interactive mode
            isBatch = false;
            promptUser(isBatch);
            fgets(input, 500, stdin);
            inputCopy = strdup(input);
            inputTokens = strtok(inputCopy, " ");
            
            outputFile = executeCommand(input, &isRedirect, &inputTokens, &fileData, &continueShell);

            if (isRedirect) {
                if (access(inputTokens, F_OK) == 0) {
                // write fileData into outputFile
                    fp = fopen(inputTokens, "r");
                    fpw = fopen(outputFile, "w");

                    while(fgets(c, sizeof(c), fp) != NULL){
                        fprintf(fpw, "%s", c);
                        fscanf(fp, "%s\n", c);
                        fprintf(fpw, "%s\n", c);
                    }

                    fclose(fp); 
                    fclose(fpw);
                }
                else {
                    printError();
                    printf("File does not exist\n");
                }
           }


            continueShell = exitProgram(&inputTokens, 0);
        } while (continueShell != 0);
    }
    
    char* killArgs = {"kill", getpid(), NULL};
    launchProcess(killArgs, 2, false);
    return 0;
}



int parseInput(char *input, char *splitWords[]){
        int wordInd = 0;
        splitWords[0] = strtok(input, " ");
        while(splitWords[wordInd] != NULL){
            splitWords[++wordInd] = strtok(NULL, " ");
        }

      return wordInd;
}

// function outputs the user, machine name, and current working directory
void promptUser(bool isBatch) {
    if (!isBatch) {
        int length = 100;
        char machineName[length], workingDir[length];
        gethostname(machineName, length);
        printf("%s@%s:%s$ ", getenv("USER"), machineName, getcwd(workingDir, 100));
    }
}

void printError() {
    printf("Shell Program Error Encountered\n");
}

// function parses the output file name to write to
char *redirectCommand(char *special, char *cmd, bool *isRedirect, char *tokens[], char *outputTokens[]){
    FILE *fp;
    char stuff[50], c[500];
    char *temp, *temp2, *temp3, *temp4, *temp5;
    //we are going to parse out the whitespace and the > sign. This way, when we return tokens[2], we are returning
    char **tempThing = tokens;
    *tokens = strtok(cmd, special);
    *tokens = strtok(cmd, " ");
    temp = *tokens;
    temp2 = (temp+4);
    temp3 = strtok(temp2, " ");
    *tokens = temp3;

    //We will be writing from the input file to the output file. (Done in main).
    temp4 = (temp+15);
    temp5 = strtok(temp4, "\n");
    //This is the parsing to find the outputfile name.
    return temp5;   //we are returning the output file.
  
}

// calls prompter function based on inputted tokens
char *executeCommand(char *cmd, bool *isRedirect, char* tokens[], char* outputTokens[],  bool *isExits){
    char *cmd2 = strdup(cmd); //makes a copy of the command.
    char *outputFile = "";
    char *found;
    int returnedTokens;
    char deliminator = '>';
    strcat(cmd2, "\n");//append a new line character.
    found = strchr(cmd2, '>');

    // determines if the command is a redirect or not
    if(found != NULL){
        outputFile = redirectCommand(&deliminator, cmd, isRedirect, tokens, outputTokens);
        *isRedirect = 1;
    }
 
    else if(found == NULL){
        returnedTokens = parseInput(cmd, tokens);
        *isRedirect = 0;
    }

    // checks the first token to call appropriate function
    if(returnedTokens == 0){
        return outputFile;
    }
    if(strcmp(outputFile, "") == 0){
        if (strcmp(tokens[0], "\n") == 0) {
            return outputFile;
        }
        else if((strcmp(tokens[0], "help") == 0) || strcmp(tokens[0], "help\n") == 0){ //Change this to do string compare.
            printHelp(tokens, returnedTokens);
        }
        else if((strcmp(tokens[0], "exit") == 0)||(strcmp(tokens[0], "exit\n") == 0)){
            *isExits = exitProgram(&tokens[0], returnedTokens);
            return outputFile;
        }
        else if((strcmp(tokens[0], "cd") == 0)||(strcmp(tokens[0], "cd\n") == 0)){
            changeDirectories(tokens, returnedTokens);
        }
        else{
            launchProcess(tokens, returnedTokens, isRedirect);
        }
    }
    return outputFile;
}

void printHelp(char* tokens[], int numTokens) {
    if (strcmp(tokens[0], "help") == 0) {

        if (numTokens > 1)
            printError();
        else
            printf("Tony's and Chris's example linux shell.\n"
                "These shell commands are defined internally.\n"
                "help -prints this screen so you can see available shell commands.\n"
                "cd -changes directories to specified pth; if not given, it defaults to home.\n"
                "exit -closes the example shell.\n"
                "[input] > [output] -pipes input file into output file\n\n"
                "And more! If it's not explicitily defined here (or in the documentation for the assignement), "
                "then the command should try to be executed by launchProcess.\n"
                "That's how we get ls -la to work here!\n\n");
    }
}

bool exitProgram(char **argument, int numTokens){
    if (numTokens > 1) {
        printError();
        return 1;
    }
	if(strcmp(*argument, "exit\n") == 0){
		return 0;
	}
	else{
		return 1;
	}
}

// forks the program and runs the given process on the child process
void launchProcess(char *tokens[], int numTokens, bool isRedirect) {
    int success = -1;
    pid_t child = fork();
    if (child == -1) {
        printf("Cannot fork process, error occurred\n");
    }
    else if (child == 0) { // child
        int c = 0;
        char *found = strchr(tokens[0], '\n');
        if (found != NULL) {
            while(tokens[numTokens-1][c] != '\0')
                c++;
            tokens[numTokens-1][c-1] = '\0';
        }
        success = execvp(tokens[0], tokens);
        if (success == -1) {
            printError();
        }
    }
        
    else { // parent
        wait(&child);
    }

    if (child != 0) {
        // wait for child process execution to complete
        wait(&child);
    }
}

void changeDirectories(char *tokens[], int numTokens) {
    int success = -1;
    if (strcmp(tokens[0], "cd") == 0) {
        if (numTokens == 1)
            printError();
        else {
            int c = 0;
            while(tokens[1][c] != '\0')
                c++;
            tokens[1][c-1] = '\0'; // removes '\n' from directory name
            success = chdir(tokens[1]);
            if (success == -1)
                printError();
        }
    }
    else
        printError();
}
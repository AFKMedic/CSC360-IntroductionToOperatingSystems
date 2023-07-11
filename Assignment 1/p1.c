/*
Assignment P1
Chris Wong
V00780634
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

typedef struct node {
	pid_t pid;
	char *programName;
	char *arguments;
	struct node * next;
}node_t;

int runningBackgroundProcesses = 0;
node_t * head;

//Gets the string for the current working directory
void getWorkingDir(){
	char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%s", cwd);
}

//Get Username/Hostname
void getUserHost(){
	char host_name[1024];
	char *user_name;
	user_name = getenv("USER");
	gethostname(host_name, 1024);
	printf("%s@%s:",user_name,host_name);
}

//Print prompt
void systemPrompt(){
	getUserHost();
	getWorkingDir();
}

//Split input on spaces
void processInput(char* input, char** processed){
	int i = -1;
	do {
		i++;
		processed[i] = strsep(&input, " ");
	}
	while (processed[i] != NULL);
}

//Run a given command
void runCommand(char** processed){
	pid_t pid = fork();
	if (pid == 0){
		int status = execvp(processed[0], processed);
		if (status < 0){
			printf("Error: Command not found\n");
		}
		exit(0);
	}else{		
		wait(NULL);
		return;
	}
}

//Run a command in the background
void runBackgroundCommand(char** processed){
	processed++;
	pid_t pid = fork();
	if (pid == 0){
		execvp(processed[0], processed);
		exit(0);
	}else{
		runningBackgroundProcesses++;
		//Add first item if list is empty
		if(head == NULL){
			head = (node_t *) malloc(sizeof(node_t));
			head->pid = pid;
			head->programName = processed[0];
			processed++;
			head->arguments = *processed;
			head->next = NULL;
		}
		//Add to end of the list
		else{
			node_t * temp = head;
			while (temp->next != NULL) {
        		temp = temp->next;
			}
			temp->next = (node_t *) malloc(sizeof(node_t));
			temp->next->pid = pid;
			temp->next->programName = processed[0];
			processed++;
			temp->next->arguments = *processed;
			temp->next->next = NULL;
		}
	}
	return;
}

//Print background processes
void printBackground() {
    //node_t * current = head;
	//If list is empty
	if(head == NULL){
		printf("There are no running processes\n");
		return;
	}
	
	printf("%d:\t%s\t%s\n", head->pid, head->programName, head->arguments);
	//Print list
	node_t * temp = head;
    while (temp->next != NULL) {
        temp = temp->next;
		printf("%d:\t%s\t%s\n", temp->pid, temp->programName, temp->arguments);
    }
	printf("Total Background Jobs: %d\n", runningBackgroundProcesses);
	return;
}

//Check if background processes have terminated
void checkBackground(int ter){
	node_t * current;
	while (ter > 0){
		current = head;
		if (ter > 0){
			//If the head of the list is the terminated process
			if(current->pid == ter){
				printf("%d: %s %s has terminated", current->pid, current->programName, current->arguments);
				//Removing if head is the only node
				if(current->next == NULL){
					free(current);
					runningBackgroundProcesses--;
				//Remove if there are more nodes
				}else{
					node_t * temp = current;
					current = current->next;
					free(temp);
					runningBackgroundProcesses--;
				}
			//Iterate until process is found
			}else{
				while(current->next != NULL){
					if(current->next->pid == ter){
						//If current->next is the last item
						if(current->next->next == NULL){
							current = current->next;
							printf("%d: %s %s has terminated", current->pid, current->programName, current->arguments);
							free(current);
							runningBackgroundProcesses--;
						}else{
							node_t * temp = current->next;
							current->next = current->next->next;
							printf("%d: %s %s has terminated", temp->pid, temp->programName, temp->arguments);
							free(temp);
							runningBackgroundProcesses--;
						}
					}else{
						current = current->next;
					}
				}
			}
		}
		ter = waitpid(0, NULL, WNOHANG);
	}
	return;
}

//Kill all background processes before exiting
void cleanup(){
	while(head != NULL){
		kill(head->pid, SIGKILL);
		head = head->next;
	}
}

//Execute input depending on what command is given
void executeInput(char** processed){
	//Set up possible commands
	int command = 4;
	char* commands[4];
	commands[0] = "exit";
	commands[1] = "cd";
	commands[2] = "bg";
	commands[3] = "bglist";

	//Check what command was input
	for (int i = 0; i < 4; i++){
		if (strcmp(processed[0], commands[i]) == 0){
			command = i;
			break;
		}
	}

	switch (command){
		//Exit command
		case 0:
			//Check and kill running background processes before exiting
			if(runningBackgroundProcesses > 0){
				cleanup();
			}
			exit(0);
			break;
		//cd command
		case 1:
			if(processed[1] == NULL || strcmp(processed[1], "~") == 0){
				chdir(getenv("HOME"));
			}else{
				chdir(processed[1]);
			}
			break;
		//bg command
		case 2:
			runBackgroundCommand(processed);
			break;
		//bglist command
		case 3:
			printBackground(head);
			break;
		//Any other command
		case 4:
			runCommand(processed);
			break;
		default:
			break;
	}
	return;
}

//Main fuction waits on input from user
int main(){
	//Initialize
	char *input = NULL;
	char *processedInput[1024];
	node_t * head = NULL;
	pid_t ter;
	//pid_t ter;
	while (1){
		//Print (Username)@(Hostname): (CWD) $
		systemPrompt();
		//Take input
		input = readline("$ ");
		//Process input
		processInput(input, processedInput);
		//Use Input
		executeInput(processedInput);
		//Check for terminated processes
		if (runningBackgroundProcesses > 0){
			ter = waitpid(0, NULL, WNOHANG);
			checkBackground(ter);
		}
	}
	free(input);
	return 0;
}
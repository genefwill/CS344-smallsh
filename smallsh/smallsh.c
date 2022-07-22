#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define MAX_INPUT_LEN 2048
#define MAX_ARGS 512

char *DIRECT;
int running = 1;
int pid_count = 0;
int fgFlag = 0;
int controlCZUsed = 0;
int foreground = 0;
int child_bpid = 0;
int child_fpid = 0;
int exit_status = 0;


struct commands_entered {
    char * commands;
    char * args_num[MAX_ARGS];
    int count;
    char *file_input;
    char *file_output;
    int background_running;
};

void handle_SIGTSTP(int signo)
{
	fflush(stdout);


	if (foreground) {
		char* message = "\nExiting foreground-only mode\n";
		write(STDOUT_FILENO, message, 30);
		foreground = 0;
	} else {
		char* message = "\nEntering foreground-only mode (& is now ignored)\n";
		write(STDOUT_FILENO, message, 50);
		foreground = 1;
	}

	fflush(stdout);
	controlCZUsed = 1;			
}

void handle_SIGINT(int signo)
{

	if (getpgid(child_fpid) == getpid())
	{
		kill(child_fpid, 0);
	}
	char* message = "\n";				
	write(STDOUT_FILENO, message, 1);
	fflush(stdout);
	controlCZUsed = 1;					

} 


int runcommands(char* command[], char* inputFile, char* outputFile, int runBG)
{	
	int childStatus;
	int exitStatus = 1;
	int sourceFD;				
	int targetFD;	
    char * change_dir;
    char* token;
    char* dir_name;			
	
	char *newargv[] = {command[0], command[1], command[2]};
	struct sigaction SIGINT_action = {0};

        if (strcmp(command[0], "exit") == 0) {
        if(child_bpid != 0) {
            kill(child_bpid, 9);
        }
        exit(0);
    } else if(strcmp(command[0], "cd") == 0) {
        printf("%s", command[1]);
        change_dir = calloc(strlen(command[1]) + 1, sizeof(char));
        strcpy(command[1], token);
        if (token) {
            dir_name = calloc(strlen(token), sizeof(char));
            strncpy(dir_name, token, strlen(token)-1);

            if (strstr(dir_name, "$$")) {
                int pid = getpid();
                char* temp_name = calloc(strlen(dir_name)-2, sizeof(char));
                strncpy(temp_name, dir_name, strlen(dir_name)-2);

                sprintf(dir_name, "%i", pid);
                strcat(temp_name, dir_name);
                strcpy(dir_name, temp_name);
                free(temp_name);
            }

            if (chdir(dir_name) != 0) {
                perror(dir_name);
                memset(dir_name, '\0', sizeof(dir_name));
                fflush(stdout);
            }


        } else  {
            chdir(getenv("HOME"));
    } 
    }
    else if (strcmp(command[0], "status") == 0) {
            if (exit_status == 0 || exit_status == 1) {
                printf("exit value %i\n", exit_status);
                return exit_status;
            } else {
                printf("terminated by signal %i\n", exit_status);
                return exit_status;
            }
            fflush(stdout);
    }

	pid_t spawnpid = fork();

	switch(spawnpid)
	{
		case -1:
			perror("fork()\n");
			fflush(stdout);
			exitStatus = 1;
			exit(1);

		case 0:

			SIGINT_action.sa_handler = handle_SIGINT;
			sigfillset(&SIGINT_action.sa_mask);
			SIGINT_action.sa_flags = 0;
			sigaction(SIGINT, &SIGINT_action, NULL);

			if (inputFile)
			{
				sourceFD = open(inputFile, O_RDONLY);

				if (sourceFD == -1)
    			{
					perror("source open()");
					fflush(stdout);
					exitStatus = 1;
					exit(1);
    			}	

				int result = dup2(sourceFD, 0);

				if (result == -1)
				{
					perror("source dup2()");
					fflush(stdout);
					exitStatus = 1;
					exit(1);
				}
			}

			if (outputFile)
			{
				targetFD = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0777);

				if (targetFD == -1)
    			{
        			perror("target open()");
					fflush(stdout);
					exitStatus = 1;
					exit(1);
    			}

				int result = dup2(targetFD, 1);

				if (result == -1)
				{
					perror("target dup2()");
					fflush(stdout);
					exitStatus = 1;
					exit(1);
				}
			}
		
			execvp(newargv[0], newargv);

			perror("Error");
			fflush(stdout);
			exitStatus = 1;
			exit(1);
		
		default:

			if (runBG && !foreground)
			{
				child_fpid = 0;	
				child_bpid = spawnpid;

				pid_t childPid = waitpid(spawnpid, &childStatus, WNOHANG);
				printf("background pid is %i\n", spawnpid);
				fflush(stdout);
			} else {
				child_fpid = spawnpid;		

				pid_t childPid = waitpid(spawnpid, &childStatus, 0);		
			}
			
			if (WIFEXITED(childStatus))
			{
				exitStatus = (WEXITSTATUS(childStatus));
			}

			else
			{
				printf("terminated by signal %i\n", WTERMSIG(childStatus));
				fflush(stdout);
				exitStatus = (WTERMSIG(childStatus));
			}

			child_fpid = 0;

			pid_t checkChildinBG = waitpid(-1, &childStatus, WNOHANG);
			while (checkChildinBG > 0)
			{
				printf("background pid %i is done: ", checkChildinBG);
				if (WIFEXITED(childStatus))
				{
					printf("exit status %i\n", WEXITSTATUS(childStatus));
				}
				else
				{
					printf("terminated by signal %i\n", WTERMSIG(childStatus));
				}
				fflush(stdout);

				child_bpid = 0;

				checkChildinBG = waitpid(-1, &childStatus, WNOHANG);
			}

			if (sourceFD)
			{
				close(sourceFD);
			}
			if (targetFD)
			{
				close(targetFD);
			}
			
	}
	return exitStatus;
}

int parse_input(char* input) {
    char* command;
    char* command2 = NULL;
    char* input_file = NULL;
    char* output_file = NULL;
    char* save_pointer;
    char* comm_array[MAX_ARGS];
    int run_background = 0;
    int comm_count = 1;
    int exit_status = 1;

    for (int i = 0; i < strlen(input); i++) {
        if (input[i] == '\n') {
            input[i] = '\0';
        }
    }

    for (int j = 0; j < 512; j++) {
        comm_array[j] = NULL;
    }

    char* token = strtok_r(input, " ", &save_pointer);
    command = calloc(strlen(token) + 1, sizeof(char));
    strcpy(command, token);

    comm_array[0] = command;

    token = strtok_r(NULL, " ", &save_pointer);
    while(token) {
        if (strcmp(token, "<") == 0) {
            token = strtok_r(NULL, " ", &save_pointer);
            input_file = calloc(strlen(token) + 1, sizeof(char));
            strcpy(input_file, token);
        } else if (strcmp(token, ">") == 0) {
            token = strtok_r(NULL, " ", &save_pointer);
            output_file = calloc(strlen(token)+1, sizeof(char));
            strcpy(output_file, token);
        } else if (strcmp(token, "&") == 0) {
            run_background = 1;
        } else {
            command2 = calloc(strlen(token) + 1, sizeof(char));
            strcpy(command2, token);
            if (strstr(command2, "$$")) {
                int pid = getpid();
                char* temp_name = calloc(strlen(command2)-2, sizeof(char));
                strncpy(temp_name, command2, strlen(command2)-2);

                sprintf(command2, "%i", pid);
                strcat(temp_name, command2);

                strcpy(command2, temp_name);
                free(temp_name);
            }
            comm_array[comm_count] = command2;
            comm_count = comm_count + 1;
        }

        token = strtok_r(NULL, " ", &save_pointer);
    }

    exit_status = runcommands(comm_array, input_file, output_file, run_background);

    free(input_file);
    free(output_file);
    free(command);
    free(command2);

    return exit_status;
}

char* prompt_line() {
    char* prompt;
    char* curr_prompt = NULL;
    ssize_t line_read;
    size_t buffer_size = 0;
    char* pointer_save;
    char* change_dir;
    char* dir_name; 
    printf(": ");
    fflush(stdout);
    line_read = getline(&curr_prompt, &buffer_size, stdin);
    //printf("%s", curr_prompt);
    //printf("%ld", line_read);
    if (line_read == -1){
        printf("Error: line could not be read\n");
    }
    /**
    if (strncmp(curr_prompt, "exit", 4) == 0) {
        if(child_bpid != 0) {
            kill(child_bpid, 9);
        }
        exit(0);
    } else if(strncmp(curr_prompt, "cd", 2) == 0) {
        char* token = strtok_r(curr_prompt, " ", &pointer_save);
        change_dir = calloc(strlen(token) + 1, sizeof(char));
        strcpy(change_dir, token);

        token = strtok_r(NULL, " ", &pointer_save);
        if (token) {
            dir_name = calloc(strlen(token), sizeof(char));
            strncpy(dir_name, token, strlen(token)-1);

            if (strstr(dir_name, "$$")) {
                int pid = getpid();
                char* temp_name = calloc(strlen(dir_name)-2, sizeof(char));
                strncpy(temp_name, dir_name, strlen(dir_name)-2);

                sprintf(dir_name, "%i", pid);
                strcat(temp_name, dir_name);
                strcpy(dir_name, temp_name);
                free(temp_name);
            }

            if (chdir(dir_name) != 0) {
                perror(dir_name);
                memset(dir_name, '\0', sizeof(dir_name));
                fflush(stdout);
            }


        } else  {
            chdir(getenv("HOME"));
        } 
    } else if ((strncmp(curr_prompt, "#", 1) == 0) || (strncmp(curr_prompt, "\n", 1) == 0)) {
            return curr_prompt;
    } else if (strncmp(curr_prompt, "status", 6) == 0 && !controlCZUsed) {
            if (exit_status == 0 || exit_status == 1) {
                printf("exit value %i\n", exit_status);
            } else {
                printf("terminated by signal %i\n", exit_status);
            }
            fflush(stdout);
    } else {
        if (!controlCZUsed) {
            exit_status = parse_input(curr_prompt);
        }
    }
    controlCZUsed = 0;
    **/

    //parse_input(curr_prompt);
    //printf("%d", curr_prompt);
    return curr_prompt;


}

void commands(const char * args) {
    printf("%s", args);
    if (strcmp(args, "#") == 10) {
        printf("blank");
    } else if (strcmp(args, "cd") == 10){
        
        chdir(getenv("PWD"));
    } else if (strcmp(args, "exit") == 10) {
        printf("exit");
        exit(0);

    }else if (strcmp(args, "status") == 10) {
        printf("status");
    } else {
        printf("Something else: ");
        printf("%s\n", args);
        printf("%d", strcmp(args, "cd"));
    }
}


int main (int argc, char* argv[]) {

    DIRECT = getenv("PWD");
    int pid = getpid();

    struct sigaction SIGINT_action = {0};

    SIGINT_action.sa_handler = SIG_IGN;

    sigfillset(&SIGINT_action.sa_mask);

    SIGINT_action.sa_flags = 0;

    sigaction(SIGINT, &SIGINT_action, NULL);

    struct sigaction SIGTSTP_action = {0};

    SIGTSTP_action.sa_handler = handle_SIGTSTP;

    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = 0;

    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    fflush(stdout);

    while(running) {

        prompt_line();
               

    }
    return 0;




}
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define MAX_INPUT_LEN 2048
#define MAX_ARGS 512

char *DIRECT;
int running = 1;
int pid_count = 0;
bool fgFlag = false;


void commands(const char * args);

struct commands_entered {
    char * commands;
    char * args_num[MAX_ARGS];
    int count;
    char *file_input;
    char *file_output;
    int background_running;
}

struct commands_structure input_parse(char* curr_prompt, bool fgFlag) {
    struct commands_structure* new_command = malloc(sizeof(struct commands_structure));
}

char* prompt_line() {
    char* prompt;
    char* curr_prompt = NULL;

    size_t buffer_size = 0;    
    printf(": ");
    fflush(stdout);
    getline(&curr_prompt, &buffer_size, stdin);
    if (strlen(curr_prompt) == 1) {
        return curr_prompt;
    }
    //curr_prompt[strcspn(curr_prompt, "\n")] = "\0";
    //printf("Here is the prompt entered: %s\n", curr_prompt);
    fflush(stdout);
    commands(curr_prompt);
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

    while(running) {

        prompt_line();

    }
    return 0;




}
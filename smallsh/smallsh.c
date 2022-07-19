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

char *DIRECT;

void commands(const char * args);

char* prompt_line() {
    char* prompt;
    char* curr_prompt = NULL;

    size_t buffer_size = 0;    
    printf(": ");
    fflush(stdout);
    getline(&curr_prompt, &buffer_size, stdin);
    //printf("Here is the prompt entered: %s\n", curr_prompt);
    fflush(stdout);
    commands(curr_prompt);
    return prompt;


}

void commands(const char * args) {
    if ((args == NULL || strcmp(args, "#") == 10)) {
    } else if (strcmp(args, "cd") == 10){
        printf("cd");
    } else if (strcmp(args, "exit") == 10) {
        printf("exit");
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
    while(1) {

        prompt_line();

    }
    return 0;




}
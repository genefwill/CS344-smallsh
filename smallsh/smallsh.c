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

char* prompt_line() {
    char* prompt;
    char* curr_prompt = NULL;

    size_t buffer_size = 0;    
    printf(": ");
    fflush(stdout);
    getline(&curr_prompt, &buffer_size, stdin);
    printf("Here is the prompt entered: %s\n", curr_prompt);

    return prompt;


}


int main (int argc, char* argv[]) {

    prompt_line();
    return 0;




}
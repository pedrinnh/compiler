#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <stdbool.h>
#include <regex.h>
#include <sys/types.h>

#include <include/__prelex.h>
#include <include/compiler.h>
#include <include/bake_config.h>

#define MAX_GROUPS 10

#define pwd() ({ char cwd[255]; \
                getcwd(cwd, sizeof(cwd)); \
                cwd; \
              })

FILE* ___init___(char* filename);
void correct_file(FILE* file);
char* correct_line(char* lineBuffer);
regex_t compile_regex(char* c_regex, char* lineBuffer);

int
main (int argc, char *argv[]) {
    char* filename = argv[1];
    FILE* file  = ___init___(filename);

    correct_file(file);

    return 0;
}

FILE*
___init___(char* filename) {
    FILE* file = fopen(filename, "r");
    
    if (file == NULL) {
        printf("[ERROR] source code: %s not found in %s \n", filename, pwd());
        exit(EXIT_FAILURE);
    } else 
        printf("[SUCESS] using source code: %s in %s \n", filename, pwd());

    return file;
}

void
correct_file(FILE* file) {
    int maxLineLength = 128;
    int count_all  = 0;
    int lineCount  = 0;
    char character = getc(file);
    
    char* lineBuffer = (char*) malloc(sizeof(char) * maxLineLength);
    int count_line = 0;
    

    while((character != EOF)) {
        
        if (count_line == maxLineLength) {
            maxLineLength += 128;
            lineBuffer = realloc(lineBuffer, maxLineLength);
            
            if (lineBuffer == NULL) {
                printf("Error while reallocating line buffer");
                exit(EXIT_FAILURE);
            }
        }
        
        if (character == '\n') {                    
            lineBuffer[count_line] = '\0';
            
            fflush(stdout);
            lineBuffer = correct_line(lineBuffer);
            
            fflush(stdout);
            printf("[MODIFIED] line: %d, content: '%s'\n", lineCount, lineBuffer);
            
            ++lineCount;
            count_line = 0;
            character = ' ';
        }
        
        lineBuffer[count_line] = character;
        ++count_line;
        ++count_all;
        character = getc(file);
    }
    
    free(lineBuffer);
}


regex_t
compile_regex(char* c_regex, char* lineBuffer) {
    regex_t regex;
    int reti = regcomp(&regex, c_regex, REG_EXTENDED);
    
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        exit(EXIT_FAILURE);
    }
    
    return regex;
}


char* 
correct_line(char* lineBuffer) {
    printf("\n\n[ANALYZE]: %s \n", lineBuffer);
    
    regex_t regex;
    regmatch_t matches[MAX_GROUPS];
    char* c_regex = "(\\/\\/)(.*?$)";
    
    regex = compile_regex(c_regex, lineBuffer);
    
    if (regexec(&regex,
            lineBuffer,
            MAX_GROUPS,  // limit of matches ?
            matches,     // array of matches
            0) == 0) {
                
                for (unsigned int i = 0; i < MAX_GROUPS; ++i) {
                    if (matches[i].rm_so == (size_t) -1)
                        break;
                        
                    char sourceCopy[strlen(lineBuffer) + 1];
                    strcpy(sourceCopy, lineBuffer);
                    sourceCopy[matches[i].rm_eo] = 0;
                    
                    printf("Group: %u: [%2u - %2u]: %s\n",
                            i, matches[i].rm_so - 1, matches[i].rm_eo - 1, 
                                sourceCopy + matches[i].rm_so);
                }
            }

    regfree(&regex);
    
    return lineBuffer;
}

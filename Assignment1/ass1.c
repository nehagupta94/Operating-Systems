#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

//limits
#define MAX_TOKENS 100
#define MAX_STRING_LEN 100

size_t MAX_LINE_LEN = 10000;


// builtin commands
#define EXIT_STR "exit"
#define INPUT "<"
#define OUTPUT ">"
#define PIPE "|"
#define EXIT_CMD 0
#define EXIT_ERR 1
#define UNKNOWN_CMD 99


FILE *fp; // file struct for stdin
char **tokens;
char *line;
char *inputFile,*outputFile;
pid_t pid;
int status, pipeCount;

int checkInputRedirection(char **tokens){
    int i = 0;
    for(i = 0; tokens[i] != '\0' ; i++){
        if(strcmp(tokens[i],INPUT) == 0){
            //printf("Found!");
            tokens[i] = NULL;
            inputFile = tokens[i+1];
            return 1;
        }
    }
    return 0;
}

int checkOutputRedirection(char **tokens){
    int i = 0;
    for(i = 0; tokens[i] != '\0' ; i++){
        if(strcmp(tokens[i],OUTPUT) == 0){
            //printf("Found!");
            tokens[i] = NULL;
            outputFile = tokens[i+1];
            return 1;
        }
    }
    return 0;
}

int checkPipes(char** tokens){
    /*printf("\n in check pipes");
    int i =0;
    int *token;
    for(i = 0; tokens[i] != '\0' ; i++){
        if(strcmp(tokens[i],PIPE) == 0){
            pipeCount ++;
        }
    }
    if(pipeCount > 1)
        return 1;
    else
        return 0;*/
    return 1;
}

void processPipes(char** tokens){
    
    for(int i =0 ; tokens[i] != '\0' ; i++){
        printf("token[%d]: %s\n",i,tokens[i]);
    }
    
    char * argv1[] = {"ls", "-al", "/", 0};
    char * argv2[] = {"grep", "Jun", 0};

    setbuf(stdout, NULL);

    int status;
    int pipefd[2];
    pid_t cpid1;
    pid_t cpid2;

    // create a pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    cpid1 = fork();
    // child1 executes
    if (cpid1 == 0) {
        printf("In CHILD-1 (PID=%d): executing command %s ...\n", getpid(), argv1[0]);
        dup2(pipefd[1], 1);
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(argv1[0], argv1);
    }

    cpid2 = fork();
    // child2 executes
    if (cpid2 == 0) {
        printf("In CHILD-2 (PID=%d): executing command %s ...\n", getpid(), argv2[0]);
        dup2(pipefd[0], 0);
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(argv2[0], argv2);
    }

    // parent executes
    close(pipefd[0]);
    close(pipefd[1]);
    
    waitpid(cpid1, &status, WUNTRACED);
    printf("In PARENT (PID=%d): successfully reaped child (PID=%d)\n", getpid(), cpid1);
    waitpid(cpid2, &status, WUNTRACED);
    printf("In PARENT (PID=%d): successfully reaped child (PID=%d)\n", getpid(), cpid2);
    exit(0);

    //return 0;
}

void initialize()
{

    // allocate space for the whole line
    assert( (line = malloc(sizeof(char) * MAX_STRING_LEN)) != NULL);

    // allocate space for individual tokens
    assert( (tokens = malloc(sizeof(char*)*MAX_TOKENS)) != NULL);

    // open stdin as a file pointer
    assert( (fp = fdopen(STDIN_FILENO, "r")) != NULL);

}

void tokenize (char * string)
{
    int token_count = 0;
    int size = MAX_TOKENS;
    char *this_token;

    while ( (this_token = strsep( &string, " \t\v\f\n\r")) != NULL) {

        if (*this_token == '\0') continue;

        tokens[token_count] = this_token;

        printf("Token %d: %s\n", token_count, tokens[token_count]);

        token_count++;

        // if there are more tokens than space ,reallocate more space
        if(token_count >= size){
            size*=2;

            assert ( (tokens = realloc(tokens, sizeof(char*) * size)) != NULL);
        }
    }
}

void read_command()
{

    // getline will reallocate if input exceeds max length
    assert( getline(&line, &MAX_LINE_LEN, fp) > -1);

    printf("Shell read this line: %s\n", line);

    tokenize(line);
}

int run_command() {

    if (strcmp( tokens[0], EXIT_STR ) == 0)
        return EXIT_CMD;
    
    if(checkPipes(tokens)){
        processPipes(tokens);
        return EXIT_CMD;
    }

    else{
        pid = fork();
        
            if (pid < 0) {
            perror("fork failed:");
            exit(1);
            }

        if (pid == 0) {// Child executes this block printf(“This is the child\n");
            if(checkInputRedirection(tokens)){
                int in = open(inputFile, O_RDONLY, 0);
                if(in < 0){
                    printf("\nError!\n");
                }
                dup2(in,0);
                close(in);
            }
            else if(checkOutputRedirection(tokens)){
                int in = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
                if(in < 0){
                    printf("\nError\n");
                }
                dup2(in,1);
                close(in);
            }
            execvp(tokens[0],tokens);
            exit(0);
        }
               
        if (pid > 0) { //Parent executes this block
        int ret = waitpid(pid, &status, 0);
                      
            
        if (ret < 0) {
            perror("waitpid failed:");
            exit(2);
        }
        exit(0);
               
        }
    }
    return UNKNOWN_CMD;
}

int main()
{
    initialize();

    do {
        printf("sh550> ");
        read_command();
        
    } while( run_command() != EXIT_CMD );

    return 0;
}

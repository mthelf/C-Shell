// Initial code for shell along with header files which maybe required for reference


# include <stdio.h> 
# include <stdlib.h>   // used to execute subprocess and commands
# include <string.h>   
# include <unistd.h>   // used for exit, getcwd,read, write, exec
# include <sys/wait.h>  
# include <sys/types.h>
# include <dirent.h> // for ls
# include <errno.h>
# include <sys/stat.h>
# include <fcntl.h>  // used for open
# define LIMIT 256

// color definitions
# define BLUE "\x1b[34m"
# define WHITE "\x1b[37m"
# define YELLOW "\x1b[33m"
# define RED "\x1b[31m"

// change directory command
int helfCD(char* args[]){

	if (args[1] == NULL) {
        fprintf(stderr, RED"HelfShell: argument expected to \"cd\"\n"WHITE);
    } 
    else if (chdir(args[1]) != 0) {
        perror("HelfShell");
    }
    
    return 1;
}

// grep command
void helfGrep(char *expression, char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        // prints only if the file is not found
        printf(RED"Error: File not found\033[0m\n"WHITE);
        return;
    }

    char line[1024];
    // while loop to read the file line by line
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strstr(line, expression) != NULL) {
            // prints the line if the search term is found
            printf("%s", line);
        }
    }
    // close the file
    fclose(file);
    printf("\n");
}

int helfHandler(char * args[]){
    char cwd[LIMIT];

    // exit command
    if(strcmp(args[0], "exit") == 0 ) {
        exit(0);
    }

    // clears the terminal
    else if (strcmp(args[0], "clear") == 0) {
            system("clear");
    }

    // help command
    else if(strcmp(args[0], "help") == 0){
        printf("\n\n");
        printf(BLUE"Welcome to the Helf Shell help page.\n");
        printf("To run a command: type program name and arguments, then hit enter.\n");
        printf("The following are built in to the helf shell:\n");
        printf("\thelp, cd [dir], pwd, mkdir [dir], rmdir[dir], ls, grep [file] [term], \ncat [file], touch [file], and exit\n");
        printf("Use the --help flag after the command or man for information on the programs.\n");
        printf("\n\n"WHITE);
        return 1;
    }
    
    // changes directory
	else if (strcmp(args[0], "cd") == 0) {
        helfCD(args);
    }

    // prints working directory
    else if(strcmp(args[0], "pwd") == 0) {
        printf("%s\n",getcwd(cwd,1024));
        return 0;
    }

    // ls command that lists directories contents
    else if(strcmp(args[0], "ls") == 0){
        DIR *dir;
        struct dirent *entry;
        dir = opendir(getcwd(cwd,1024));
        // first checks if the directory exists
        if(dir == NULL){
            printf(RED"Cannont open directory %s: "WHITE, cwd);
            return 0;
        }
        // loops through output if directory already exists
        while((entry = readdir(dir)) != NULL) {
            printf("%s\n",entry->d_name);
        }

        closedir(dir); 
    }

    // mkdir command
    else if (strcmp(args[0], "mkdir") == 0) {
        // checks if a name has been passed for the command argument
        if (args[1] == NULL) {
            printf(RED"mkdir: missing operand\n"WHITE);
            return -1;
        }
        // sets status to make the directory with permissions 0766
        int status = mkdir(args[1], 0766);
        // checks if mkdir call works
        if (status == -1) {
            printf(RED"mkdir: cannot create directory '%s': %s\n"WHITE, args[1], strerror(errno));
            return -1;
        }
    }

    // remove directory command
    else if (strcmp(args[0], "rmdir") == 0) {
    if (args[1] == NULL) {
        printf(RED"rmdir: missing operand\n"WHITE);
        return -1;
        }
        int status = rmdir(args[1]);
        if (status == -1) {
            printf(RED"rmdir: failed to remove '%s': %s\n"WHITE, args[1], strerror(errno));
            return -1;
        }
    }

    // cat command that prints out file
    else if(strcmp(args[0], "cat") == 0){
        if(args[1] == NULL){
            printf(RED"cat: missing operand\n"WHITE);
            return -1;
        }
        FILE *file = fopen(args[1], "r");
        if(file == NULL){
            printf(RED"cat: %s: No such file or directory\n"WHITE, args[1]);
            return -1;
        }
        char ch;
        while((ch = fgetc(file)) != EOF){
            printf("%c", ch);
        
        }
        printf("\n");
        fclose(file);
        return 0;
    }

    // touch command that creates a file
    else if(strcmp(args[0], "touch") == 0) {
        if (args[1] == NULL) {
            printf(RED"touch: missing operand\n"WHITE);
            return -1;
        }
        // use open with O_CREAT and O_RDWR flags 
        int fd = open(args[1], O_CREAT | O_RDWR, 0766);
        if (fd == -1) {
            printf(RED"touch: failed to create file %s\n"WHITE, args[1]);
            return -1;
        }
        close(fd);
    }

    // grep command call
    else if(strcmp(args[0], "grep") == 0) {
        if (args[1] == NULL || args[2] == NULL) {
            printf(RED"grep: missing operand\n"WHITE);
            return -1;
        }
        helfGrep(args[2], args[1]);
    }


    else{
        printf(RED"Unknown unput.\n"WHITE);
        return 0;
    }
    return 0;
}

// read command line for the input line in shell
char *read_line(void)
{
  char *my_line = NULL;
  ssize_t bufsize = 0; // getline allocates buffer

  if (getline(&my_line, &bufsize, stdin) == -1){
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);  // exit if received EOF
    } else  {
      perror(RED"readline"WHITE);
      exit(EXIT_FAILURE);
    }
  }
  return my_line;
}

// parse the input command
#define SHELL_TOK_BUFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"
char **parse(char *my_line)
{
    
    int buffsize = SHELL_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(buffsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, RED"HelfShell: allocation error\n"WHITE);
        exit(EXIT_FAILURE);
    }

    token = strtok(my_line, SHELL_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= buffsize) {
            buffsize += SHELL_TOK_BUFSIZE;
            tokens = realloc(tokens, buffsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, RED"HelfShell: allocation error\n"WHITE);
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, SHELL_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

void helfShellLoop(void)
{
    char cd[LIMIT];
    while (1) {
        // display terminal cwd in yellow
        printf(YELLOW"%s > "WHITE, getcwd(cd, 1024));

        // read in line from user
        char *line = read_line();

        // parse line into tokens
        char **tokens = parse(line);

        helfHandler(tokens);
        // free memory allocated
        free(line);
        free(tokens);
    }

}

int main(int argc, char** argv) {
    // welcome text in blue and switches back to white
    printf(BLUE"Welcome to the Helf Shell(Hell for short >:) ) \n");
    printf("This shell was created by Michael Helf | RUID: 190006742\n");
    printf("Type 'help' to see a starting guide for this shell.\n"WHITE);

    // runs shell loop
    helfShellLoop();

    return EXIT_SUCCESS;
    return 0;
}
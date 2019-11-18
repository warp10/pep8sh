#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PEP8SH_RL_BUFSIZE 79
#define PEP8SH_TOK_BUFSIZE 64
#define PEP8SH_TOK_DELIM " \t\r\n\a"
#define HOSTNAME_MAX 50

int pep8sh_cd(char **args);
int pep8sh_help(char **args);
int pep8sh_exit(char **args);
void pep8sh_loop(void);
char *pep8sh_read_line(void);
char **pep8sh_split_line(char *line);
int pep8sh_launch(char **args);
int pep8sh_num_builtins();
int pep8sh_cd(char **args);
int pep8sh_help(char **args);
int pep8sh_exit(char **args);
int pep8sh_execute(char **args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[])(char**) = {
    &pep8sh_cd,
    &pep8sh_help,
    &pep8sh_exit
};

int main(int argc, char **argv) {
    pep8sh_loop();
    return EXIT_SUCCESS;
}

void pep8sh_loop(void) {
    char *line;
    char **args;
    int status = 1; // Set a default value to keep pep8sh running if the first command is > 79 chars

    do {
        char hostname[HOSTNAME_MAX+1];
        gethostname(hostname, sizeof(hostname));
        printf("%s@%s:%s$ ", getenv("USER"), hostname, getenv("PWD"));
        line = pep8sh_read_line();

        if (*line == '\0') {
            continue;
        }
        args = pep8sh_split_line(line);
        status = pep8sh_execute(args);

        free(line);
        free(args);
    } while (status);
}

char *pep8sh_read_line(void) {
  char *line = NULL;
  size_t bufsize = 0;
  getline(&line, &bufsize, stdin);

  if (strlen(line) > PEP8SH_RL_BUFSIZE) {
      printf("OMFG, you are violating the PEP8! No more than 79 chars, please!\n");
      return "\0";
  }

  return line;
}

char **pep8sh_split_line(char *line){
    int bufsize = PEP8SH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "pep8sh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, PEP8SH_TOK_DELIM);
    while (token!= NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += PEP8SH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if(!tokens) {
                fprintf(stderr, "pep8sh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, PEP8SH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int pep8sh_launch(char **args){
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("pep8sh");
        }
        exit (EXIT_FAILURE);
    } else if (pid < 0) {
        perror("pep8sh");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int pep8sh_num_builtins(){
    return sizeof(builtin_str) / sizeof(char *);
}

int pep8sh_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "pep8sh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("pep8sh");
        }
    }
    return 1;
}

int pep8sh_help(char **args) {
    int i;
    printf("pep8sh: The shell that respects PEP8 even for commands\n");
    printf("Type program names and arguments, and hit enter, as long as they are shorter than 79 chars.\n");
    printf("The following are builtins:\n");

    for (i=0; i<pep8sh_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int pep8sh_exit(char **args) {
    return 0;
}

int pep8sh_execute(char **args){
    int i;

    if (args[0] == NULL) {
        return 1;
    }

    for (i=0; i<pep8sh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0){
            return (*builtin_func[i])(args);
        }
    }

    return pep8sh_launch(args);
}

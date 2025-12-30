#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1

int should_run = 1;

int uinxsh_cd(char **args);
int uinxsh_pwd(char **args);
int uinxsh_help(char **args);
int uinxsh_exit(char **args);
int uinxsh_echo(char **args);

char *builtin_str[] = {"cd", "pwd", "help", "exit", "echo"};
int (*builtin_func[])(char **) = {&uinxsh_cd, &uinxsh_pwd, &uinxsh_help, &uinxsh_exit, &uinxsh_echo};

int run_builtin(char **args) {
    for (int i = 0; i < 5; i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) return (*builtin_func[i])(args);
    }
    return 0;
}

int uinxsh_cd(char **args) {
    char *path = (args[1] == NULL || strcmp(args[1], "~") == 0) ? getenv("HOME") : args[1];
    if (chdir(path) != 0) perror("uinxsh");
    return 1;
}

int uinxsh_pwd(char **args) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) printf("%s\n", cwd);
    return 1;
}

int uinxsh_exit(char **args) { should_run = 0; return 1; }

int uinxsh_help(char **args) {
    printf("uinxsh: Built-ins: cd, pwd, help, exit, echo. Supports '&' and '|'.\n");
    return 1;
}

int uinxsh_echo(char **args) {
    for (int i = 1; args[i]; i++) printf("%s ", args[i]);
    printf("\n");
    return 1;
}
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
void execute_command(char **args, int is_bg) {
    pid_t pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) perror("uinxsh");
        exit(EXIT_FAILURE);
    } else if (!is_bg) waitpid(pid, NULL, 0);
}

void execute_pipe(char **args, int p_idx) {
    int fd[2];
    pipe(fd);
    args[p_idx] = NULL;
    if (fork() == 0) {
        dup2(fd[1], 1); close(fd[0]); close(fd[1]);
        execvp(args[0], args); exit(0);
    }
    if (fork() == 0) {
        dup2(fd[0], 0); close(fd[1]); close(fd[0]);
        execvp(args[p_idx+1], &args[p_idx+1]); exit(0);
    }
    close(fd[0]); close(fd[1]);
    wait(NULL); wait(NULL);
}

char **parse(char *in, int *bg) {
    char **tokens = malloc(64 * sizeof(char *));
    char *token = strtok(in, " \t\n");
    int i = 0;
    while (token) { tokens[i++] = token; token = strtok(NULL, " \t\n"); }
    tokens[i] = NULL;
    if (i > 0 && strcmp(tokens[i-1], "&") == 0) { *bg = 1; tokens[--i] = NULL; }
    return tokens;
}

int main() {
    while (should_run) {
        while (waitpid(-1, NULL, WNOHANG) > 0);
        char *in = readline("uinxsh> ");
        if (!in) break;
        if (strcmp(in, "!!") == 0) {
            HIST_ENTRY *h = history_get(history_length);
            if (h) { printf("%s\n", h->line); in = strdup(h->line); }
        }
        add_history(in);
        int bg = 0;
        char **args = parse(in, &bg);
        if (args[0]) {
            int p_idx = -1;
            for (int i = 0; args[i]; i++) if (strcmp(args[i], "|") == 0) p_idx = i;
            if (!run_builtin(args)) {
                if (p_idx != -1) execute_pipe(args, p_idx);
                else execute_command(args, bg);
            }
        }
        free(args); free(in);
    }
    return 0;
}
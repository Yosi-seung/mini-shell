#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#define MAX_ARGS 10

int main(void)
{
    char input[100];
    char *args[MAX_ARGS];

    while (1)
    {
        printf("myshell$ ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            break;
        }

        // 줄바꿈 제거
        input[strcspn(input, "\n")] = '\0';

        // 빈 입력
        if (strlen(input) == 0)
        {
            continue;
        }

        // 공백 기준으로 분리
        int argc = 0;
        char *token = strtok(input, " \t");

        while (token != NULL && argc < MAX_ARGS - 1)
        {
            args[argc] = token;
            argc++;
            token = strtok(NULL, " \t");
        }

        args[argc] = NULL;

        // exit
        if (strcmp(args[0], "exit") == 0)
        {
            break;
        }

        // cd
        if (strcmp(args[0], "cd") == 0)
        {
            if (args[1] == NULL)
            {
                chdir(getenv("HOME"));
            }
            else if (chdir(args[1]) != 0)
            {
                perror("cd");
            }

            continue;
        }

        // >
        int redirect = -1;

        for (int i = 0; i < argc; i++)
        {
            if (strcmp(args[i], ">") == 0)
            {
                redirect = i;
                break;
            }
        }

        // 자식 프로세스 생성
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");
            continue;
        }

        if (pid == 0)
        {
            // 출력 리다이렉션
            if (redirect != -1)
            {
                if (args[redirect + 1] == NULL)
                {
                    fprintf(stderr, "파일 이름이 필요합니다.\n");
                    exit(1);
                }

                int fd = open(
                    args[redirect + 1],
                    O_WRONLY | O_CREAT | O_TRUNC,
                    0644
                );

                if (fd < 0)
                {
                    perror("open");
                    exit(1);
                }

                dup2(fd, STDOUT_FILENO);
                close(fd);

                // > 이후 부분 제거
                args[redirect] = NULL;
            }

            execvp(args[0], args);

            perror("execvp");
            exit(1);
        }
        else
        {
            wait(NULL);
        }
    }

    printf("myshell 종료\n");

    return 0;
}
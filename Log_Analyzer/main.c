#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <regex.h>
#include <fcntl.h>

//  Constants & Config 
#define MAX_PATH 1024
#define MAX_LINE 2048
#define MAX_FILES 100

#define PROTO_LOG "LOG:"
#define PROTO_BUG "BUG:"

typedef struct
{
    char target_severity[20];
    char output_path[MAX_PATH];
    char logs_dir[MAX_PATH];
} Config;

typedef struct
{
    char filename[256];
    char filepath[MAX_PATH];
    char dependency[256];
    int bug_count;
    int pipe_fd[2];
} LogFile;

LogFile files[MAX_FILES];
int file_count = 0;
regex_t regex;

const char *LOG_PATTERN ="^(ERROR|INFO|WARNING) \\| [0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2} \\| (.+)$";

void trim_newline(char *s)
{
    int l = strlen(s);
    while (l > 0 && (s[l - 1] == '\n' || s[l - 1] == '\r'))
        s[--l] = 0;
}

int is_valid_log(const char *line)
{
    return regexec(&regex, line, 0, NULL, 0) == 0;
}

int matches_severity(const char *line, const char *severity)
{
    return strncmp(line, severity, strlen(severity)) == 0;
}

void execute_worker(int file_idx, const char *target_severity)
{
    LogFile *f = &files[file_idx];
    close(f->pipe_fd[0]);

    FILE *fp = fopen(f->filepath, "r");
    if (!fp)
        exit(1);

    char line[MAX_LINE];
    int first_line = 1;
    int local_bugs = 0;

    while (fgets(line, sizeof(line), fp))
    {
        trim_newline(line);

        if (first_line && strncmp(line, "...", 3) == 0)
        {
            first_line = 0;
            continue;
        }
        first_line = 0;

        if (is_valid_log(line))
        {
            if (matches_severity(line, target_severity))
                dprintf(f->pipe_fd[1], "%s%s\n", PROTO_LOG, line);
        }
        else
        {
            local_bugs++;
        }
    }

    fclose(fp);

    dprintf(f->pipe_fd[1], "%s%d\n", PROTO_BUG, local_bugs);
    close(f->pipe_fd[1]);
    exit(0);
}

void sort_files_by_dependency()
{
    for (int i = 0; i < file_count - 1; i++)
    {
        for (int j = 0; j < file_count - i - 1; j++)
        {
            if (strlen(files[j].dependency) &&
                strcmp(files[j].dependency, files[j + 1].filename) == 0)
            {
                LogFile tmp = files[j];
                files[j] = files[j + 1];
                files[j + 1] = tmp;
            }
        }
    }
}

int main()
{
    Config cfg;
    strcpy(cfg.target_severity, "ERROR");
    strcpy(cfg.output_path, "output.txt");
    strcpy(cfg.logs_dir, "logs");

    if (regcomp(&regex, LOG_PATTERN, REG_EXTENDED))
        return 1;

    DIR *d = opendir(cfg.logs_dir);
    if (!d)
        return 1;

    struct dirent *dir;
    while ((dir = readdir(d)))
    {
        if (dir->d_type == DT_REG && strstr(dir->d_name, ".txt"))
        {
            LogFile *f = &files[file_count++];
            memset(f, 0, sizeof(LogFile));

            strcpy(f->filename, dir->d_name);
            snprintf(f->filepath, MAX_PATH, "%s/%s",
                     cfg.logs_dir, f->filename);

            FILE *fp = fopen(f->filepath, "r");
            if (fp)
            {
                char line[MAX_LINE];
                if (fgets(line, sizeof(line), fp))
                {
                    trim_newline(line);
                    if (strncmp(line, "...", 3) == 0)
                        strcpy(f->dependency, line + 4);
                }
                fclose(fp);
            }
        }
    }
    closedir(d);

    for (int i = 0; i < file_count; i++)
    {
        pipe(files[i].pipe_fd);

        pid_t pid = fork();
        if (pid == 0)
            execute_worker(i, cfg.target_severity);
        else
            close(files[i].pipe_fd[1]);
    }

    while (wait(NULL) > 0)
        ;

    sort_files_by_dependency();

    FILE *out = fopen(cfg.output_path, "w");
    if (!out)
        return 1;

    for (int i = 0; i < file_count; i++)
    {
        FILE *in = fdopen(files[i].pipe_fd[0], "r");
        char line[MAX_LINE];

        while (fgets(line, sizeof(line), in))
        {
            if (strncmp(line, PROTO_LOG, 4) == 0)
                fprintf(out, "%s\n", line + 4);
            else if (strncmp(line, PROTO_BUG, 4) == 0)
                files[i].bug_count = atoi(line + 4);
        }
        fclose(in);
    }

    for (int i = 0; i < file_count; i++)
        fprintf(out, "%s: %d bugs\n",
                files[i].filename, files[i].bug_count);

    fclose(out);
    regfree(&regex);
    return 0;
}

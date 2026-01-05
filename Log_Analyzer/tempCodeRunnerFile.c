
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <regex.h>
#include <fcntl.h>

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

// Global Variables
LogFile files[MAX_FILES];
int file_count = 0;
regex_t regex;

// Regex Pattern: LEVEL | YYYY-MM-DD HH:MM:SS | Message
const char *LOG_PATTERN = "^(ERROR|INFO|WARNING) \\| [0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2} \\| (.+)$";


void trim_newline(char *s)
{
    if (!s) return;
    int l = (int)strlen(s);
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
    close(f->pipe_fd[0]); // close unused read end

    FILE *file = fopen(f->filepath, "r");
    int local_bug = 0;
    if (file)
    {
        char line[MAX_LINE];
        int first_line = 1;
        while (fgets(line, sizeof(line), file))
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
                {
                    // write trimmed line (no extra newline)
                    dprintf(f->pipe_fd[1], PROTO_LOG "%s\n", line);
                }
            }
            else
            {
                local_bug++;
            }
        }
        fclose(file);
    }
    else
    {
        local_bug += 1;
    }

    dprintf(f->pipe_fd[1], PROTO_BUG "%d\n", local_bug);
    close(f->pipe_fd[1]);

    _exit(0);
}


void sort_files_by_dependency()
{
    for (int i = 0; i < file_count - 1; i++)
    {
        for (int j = 0; j < file_count - i - 1; j++)
        {
            if (files[j].dependency[0] != '\0' &&
                strcmp(files[j].dependency, files[j + 1].filename) == 0)
            {
                LogFile temp = files[j];
                files[j] = files[j + 1];
                files[j + 1] = temp;
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

    if (regcomp(&regex, LOG_PATTERN, REG_EXTENDED | REG_NEWLINE))
    {
        perror("Regex compilation failed");
        return 1;
    }

    
    DIR *d = opendir(cfg.logs_dir);
    if (!d)
    {
        perror("Failed to open logs dir");
        regfree(&regex);
        return 1;
    }

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL)
    {
        if (dir->d_type == DT_REG && strstr(dir->d_name, ".txt"))
        {
            // Initialize LogFile struct in 'files' array.
            if (file_count >= MAX_FILES) break;
            LogFile *f = &files[file_count++];
            memset(f, 0, sizeof(LogFile));
            f->bug_count = 0;
            f->pipe_fd[0] = -1;
            f->pipe_fd[1] = -1;

            strncpy(f->filename, dir->d_name, sizeof(f->filename) - 1);
            // Construct filepath.
            snprintf(f->filepath, sizeof(f->filepath), "%s/%s", cfg.logs_dir, f->filename);
            f->dependency[0] = '\0';

            // Open file briefly to check first line for "..." dependency.
            FILE *file = fopen(f->filepath, "r");
            if (file)
            {
                char line[MAX_LINE];
                if (fgets(line, sizeof(line), file))
                {
                    trim_newline(line);
                    if (strncmp(line, "...", 3) == 0)
                    {
                        // copy after "... " (allowing either "... " or "...")
                        char *dep = line + 3;
                        while (*dep == ' ' || *dep == '\t') dep++;
                        strncpy(f->dependency, dep, sizeof(f->dependency) - 1);
                        f->dependency[sizeof(f->dependency) - 1] = '\0';
                    }
                }
                fclose(file);
            }
        }
    }
    closedir(d);

    if (file_count == 0)
    {
        printf("No .txt files found in %s\n", cfg.logs_dir);
        regfree(&regex);
        return 0;
    }

    // 3. Concurrent Processing
    // Create pipes and fork for each file
    for (int i = 0; i < file_count; i++)
    {
        if (pipe(files[i].pipe_fd) == -1)
        {
            perror("pipe");
            regfree(&regex);
            return 1;
        }

        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork");
            regfree(&regex);
            return 1;
        }

        if (pid == 0)
        {
            // child
            execute_worker(i, cfg.target_severity);
            // not reached
        }
        else
        {
            // parent: close write end
            close(files[i].pipe_fd[1]);
        }
    }

    // 4. Wait for completion
    while (wait(NULL) > 0)
        ;

    // 5. Output Generation
    sort_files_by_dependency();

    FILE *f_out = fopen(cfg.output_path, "w");
    if (!f_out)
    {
        perror("Output file error");
        regfree(&regex);
        return 1;
    }

    // Iterate through sorted files and read from pipes
    for (int i = 0; i < file_count; i++)
    {
        if (files[i].pipe_fd[0] < 0) continue;
        FILE *f_in = fdopen(files[i].pipe_fd[0], "r");
        if (!f_in) continue;

        char line[MAX_LINE];
        while (fgets(line, sizeof(line), f_in))
        {
            trim_newline(line);
            if (strncmp(line, PROTO_LOG, strlen(PROTO_LOG)) == 0)
            {
                const char *rest = line + strlen(PROTO_LOG);
                fprintf(f_out, "%s\n", rest);
            }
            else if (strncmp(line, PROTO_BUG, strlen(PROTO_BUG)) == 0)
            {
                const char *rest = line + strlen(PROTO_BUG);
                files[i].bug_count = atoi(rest);
            }
        }
        fclose(f_in);
        files[i].pipe_fd[0] = -1;
    }

    // Print bug report summary at bottom of f_out.
    for (int i = 0; i < file_count; i++)
    {
        fprintf(f_out, "%s: %d bugs\n", files[i].filename, files[i].bug_count);
    }

    fclose(f_out);
    regfree(&regex);
    return 0;
}

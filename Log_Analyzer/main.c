// phase1.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <regex.h>

#define MAX_PATH 1024
#define MAX_LINE 2048
#define MAX_FILES 100

#define PROTO_LOG "LOG:"
#define PROTO_BUG "BUG:"

typedef struct {
    char filename[256];
    char filepath[MAX_PATH];
    char dependency[256];
    int bug_count;
    int pipe_fd[2];
} LogFile;

LogFile files[MAX_FILES];
int file_count = 0;
regex_t regex;

const char *LOG_PATTERN =
"^(ERROR|INFO|WARNING) \\| [0-9]{4}-[0-9]{2}-[0-9]{2} "
"[0-9]{2}:[0-9]{2}:[0-9]{2} \\| (.+)$";

void trim_newline(char *s) {
    int l = strlen(s);
    while (l > 0 && (s[l-1] == '\n' || s[l-1] == '\r'))
        s[--l] = 0;
}

int is_valid_log(const char *line) {
    return regexec(&regex, line, 0, NULL, 0) == 0;
}

int matches_severity(const char *line, const char *sev) {
    return strncmp(line, sev, strlen(sev)) == 0;
}

/* ------------ Worker ------------ */
void execute_worker(int idx, const char *severity) {
    LogFile *f = &files[idx];
    close(f->pipe_fd[0]);

    FILE *fp = fopen(f->filepath, "r");
    int bugs = 0;

    if (fp) {
        char line[MAX_LINE];
        int first = 1;

        while (fgets(line, sizeof(line), fp)) {
            trim_newline(line);

            if (first && strncmp(line, "...", 3) == 0) {
                first = 0;
                continue;
            }
            first = 0;

            if (is_valid_log(line)) {
                if (matches_severity(line, severity)) {
                    dprintf(f->pipe_fd[1], "LOG:%s\n", line);
                }
            } else {
                bugs++;
            }
        }
        fclose(fp);
    }

    dprintf(f->pipe_fd[1], "BUG:%d\n", bugs);
    close(f->pipe_fd[1]);
    exit(0);
}

/* ------------ Dependency sort (output only) ------------ */
void sort_files_by_dependency() {
    for (int i = 0; i < file_count - 1; i++) {
        for (int j = 0; j < file_count - i - 1; j++) {
            if (strcmp(files[j].dependency, files[j+1].filename) == 0) {
                LogFile tmp = files[j];
                files[j] = files[j+1];
                files[j+1] = tmp;
            }
        }
    }
}

int main() {
    const char *logs_dir = "logs";
    const char *output = "output.txt";
    const char *target_sev = "ERROR";

    regcomp(&regex, LOG_PATTERN, REG_EXTENDED);

    DIR *d = opendir(logs_dir);
    struct dirent *ent;

    while ((ent = readdir(d)) != NULL) {
        if (strstr(ent->d_name, ".txt")) {
            LogFile *f = &files[file_count++];
            strcpy(f->filename, ent->d_name);
            snprintf(f->filepath, sizeof(f->filepath),
                     "%s/%s", logs_dir, f->filename);

            FILE *fp = fopen(f->filepath, "r");
            if (fp) {
                char line[MAX_LINE];
                if (fgets(line, sizeof(line), fp)) {
                    trim_newline(line);
                    if (strncmp(line, "...", 3) == 0)
                        strcpy(f->dependency, line + 4);
                }
                fclose(fp);
            }
        }
    }
    closedir(d);

    /* Fork workers */
    for (int i = 0; i < file_count; i++) {
        pipe(files[i].pipe_fd);

        if (fork() == 0)
            execute_worker(i, target_sev);
        else
            close(files[i].pipe_fd[1]);
    }

    while (wait(NULL) > 0);

    sort_files_by_dependency();

    FILE *out = fopen(output, "w");

    for (int i = 0; i < file_count; i++) {
        FILE *in = fdopen(files[i].pipe_fd[0], "r");
        char buf[MAX_LINE];

        while (fgets(buf, sizeof(buf), in)) {
            if (strncmp(buf, PROTO_LOG, 4) == 0)
                fprintf(out, "%s", buf + 4);
            else if (strncmp(buf, PROTO_BUG, 4) == 0)
                files[i].bug_count = atoi(buf + 4);
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

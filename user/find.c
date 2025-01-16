#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

int is_space(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
}

char* trim_whitespace(char* str) {
    char* end;

    // Trim leading space
    while (is_space((unsigned char)*str)) str++;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && is_space((unsigned char)*end)) end--;

    // Write new null terminator
    *(end + 1) = '\0';

    return str;
}

char* get_file_name(char* path) {
    int i;
    int len = strlen(path);

    // Start from the end of the string and look for the last '/'
    for (i = len - 1; i >= 0; i--) {
        if (path[i] == '/') {
            return &path[i + 1]; // Return a pointer to the character after the last '/'
        }
    }

    // If no '/' is found, the entire path is the file name
    return path;
}

void recursively_cd(char *path, char *file) {
    char buf[1024], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, O_RDONLY)) < 0) {
        fprintf(2, "ls: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type) {
    case T_FILE:
        // printf("%s\n", path);
        // printf("file: '%s', name: '%s', strcmp: %d\n", file, get_file_name(fmtname(path)), strcmp(file, get_file_name(fmtname(path))));
        if (strcmp(file, trim_whitespace(get_file_name(fmtname(path)))) == 0) {
            printf("%s\n", path); // Print full path
        }
        break;

    case T_DIR:
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
            fprintf(2, "ls: path too long\n");
            break;
        }
        // printf("%s\n", path);
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';

        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.inum == 0)
                continue;

            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;

            if (stat(buf, &st) < 0) {
                printf("ls: cannot stat %s\n", buf);
                continue;
            }

            if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue; // Skip "." and ".."

            recursively_cd(buf, file);
        }
        break;
    }
    close(fd);
}

int
main(int argc, char *argv[])
{
  if (argc != 3) {
    printf("Please follow the format: first specify the directory path as the first parameter, then specify the name of the file as the second parameter.\n");
    exit(1);
  }

  recursively_cd(argv[1], argv[2]);
  
  exit(0);
}
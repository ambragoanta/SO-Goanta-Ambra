#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define BUFFER_SIZE 2048

void getBMPDetails(const char *name, char *buffer) {
    int bmpFile = open(name, O_RDONLY);
    if (bmpFile == -1) {
        perror("eroare deschidere fisier bmp");
        return;
    }

    lseek(bmpFile, 18, SEEK_SET);
    int32_t lungime, inaltime;
    read(bmpFile, &lungime, sizeof(lungime));
    read(bmpFile, &inaltime, sizeof(inaltime));
    close(bmpFile);

    sprintf(buffer + strlen(buffer),
            "nume fisier: %s\n"
            "inaltime: %d\n"
            "lungime: %d\n",
            name, inaltime, lungime);
}

void getStatistics(const char *fileName, struct stat *fileStat, int statFile, int isBmp, int isDir, int isLnk) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    int length = 0;

    if (isBmp) {
        getBMPDetails(fileName, buffer);
    } else if (isDir) {
        length = sprintf(buffer, "nume director: %s\n", fileName);
    } else if (isLnk) {
        length = sprintf(buffer, "nume legatura: %s\n", fileName);
    } else {
        length = sprintf(buffer, "nume fisier: %s\n", fileName);
    }

    length += sprintf(buffer + length,
                      "dimensiune: %ld\n"
                      "identificatorul utilizatorului: %d\n"
                      "timpul ultimei modificari: %s"
                      "contorul de legaturi: %ld\n"
                      "drepturi de acces user: %c%c%c\n"
                      "drepturi de acces grup: %c%c%c\n"
                      "drepturi de acces altii: %c%c%c\n",
                      fileStat->st_size,
                      fileStat->st_uid,
                      ctime(&fileStat->st_mtime),
                      fileStat->st_nlink,
                      (fileStat->st_mode & S_IRUSR) ? 'R' : '-',
                      (fileStat->st_mode & S_IWUSR) ? 'W' : '-',
                      (fileStat->st_mode & S_IXUSR) ? 'X' : '-',
                      (fileStat->st_mode & S_IRGRP) ? 'R' : '-',
                      (fileStat->st_mode & S_IWGRP) ? 'W' : '-',
                      '-',
                      (fileStat->st_mode & S_IROTH) ? 'R' : '-',
                      (fileStat->st_mode & S_IWOTH) ? 'W' : '-',
                      '-'
    );

    write(statFile, buffer, length);
}


void processDirectoryEntries(DIR *dir, const char *dirName, int statFile) {
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char path[1024];
        sprintf(path, "%s/%s", dirName, entry->d_name);

        struct stat fileStat;
        if (lstat(path, &fileStat) < 0) {
            perror("Error getting file stats");
            continue;
        }

        int isBmp = 0, isDir = 0, isLnk = 0;

        if (S_ISREG(fileStat.st_mode)) {
            char *ext = strrchr(entry->d_name, '.');
            isBmp = (ext && strcmp(ext, ".bmp") == 0);
        } else if (S_ISDIR(fileStat.st_mode)) {
            isDir = 1;
        } else if (S_ISLNK(fileStat.st_mode)) {
            isLnk = 1;
        }

        getStatistics(entry->d_name, &fileStat, statFile, isBmp, isDir, isLnk);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        write(STDOUT_FILENO, "Usage: ./program <director_intrare>\n", 37);
        return -1;
    }

    DIR *dir = opendir(argv[1]);
    if (dir == NULL) {
        perror("eroare deschidere folder");
        return -1;
    }

    int statFile = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (statFile == -1) {
        perror("eroare obtinere statistica");
        closedir(dir);
        return -1;
    }

    processDirectoryEntries(dir, argv[1], statFile);

    closedir(dir);
    close(statFile);
    return 0;
}

#include <unistd.h>
#include <stdlib.h>
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


void makePictureGray(const char *path) {
    int picture = open(path, O_RDWR);
    if (picture == -1) {
        perror("eroare deschidere fisier bmp");
        return;
    }

    uint8_t header[54];
    if (read(picture, header, 54) != 54) {
        perror("eroare deschidere header");
        close(picture);
        return;
    }

    int32_t width = *(int32_t*)&header[18];
    int32_t height = *(int32_t*)&header[22];
    uint32_t dataOffset = *(uint32_t*)&header[10];

    lseek(picture, dataOffset, SEEK_SET);
    uint8_t RGB[3];
    int padding = (4 - (width * 3) % 4) % 4;

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {

            if (read(picture, RGB, 3) != 3) {
                perror("Error reading pixel data");
                close(picture);
                return;
            }

            //code for gray
            RGB[0] = RGB[1] = RGB[2] = (uint8_t)(0.299 * RGB[2] + 0.587 * RGB[1] + 0.114 * RGB[0]);

            lseek(picture, -3, SEEK_CUR);
            if (write(picture, RGB, 3) != 3) {
                perror("Error writing gray pixel");
                close(picture);
                return;
            }
        }

        lseek(picture, padding, SEEK_CUR);
    }

    close(picture);
}

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

void getStatistics(const char *inputPath, const char *outputPath, int isBmp, int isDir, int isLnk) {
    char buffer[BUFFER_SIZE];
    struct stat fileStat;
    if (lstat(inputPath, &fileStat) < 0) {
        perror("Error getting file stats");
        return;
    }
    memset(buffer, 0, sizeof(buffer));
    int statFile = open(outputPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (statFile == -1) {
        perror("Error opening output file");
        return;
    }
    int stats = 0;

    if (isBmp) {
        getBMPDetails(inputPath, buffer);
        makePictureGray(inputPath);
        stats = strlen(buffer);
    } else if (isDir) {
        stats = sprintf(buffer, "nume director: %s\n", inputPath);
    } else if (isLnk) {
        stats = sprintf(buffer, "nume legatura: %s\n", inputPath);
    } else {
        stats = sprintf(buffer, "nume fisier: %s\n", inputPath);
    }

    stats += sprintf(buffer + stats,
                     "dimensiune: %ld\n"
                     "identificatorul utilizatorului: %d\n"
                     "timpul ultimei modificari: %s"
                     "contorul de legaturi: %ld\n"
                     "drepturi de acces user: %c%c%c\n"
                     "drepturi de acces grup: %c%c%c\n"
                     "drepturi de acces altii: %c%c%c\n",
                     fileStat.st_size,
                     fileStat.st_uid,
                     ctime(&fileStat.st_mtime),
                     fileStat.st_nlink,
                     (fileStat.st_mode & S_IRUSR) ? 'R' : '-',
                     (fileStat.st_mode & S_IWUSR) ? 'W' : '-',
                     (fileStat.st_mode & S_IXUSR) ? 'X' : '-',
                     (fileStat.st_mode & S_IRGRP) ? 'R' : '-',
                     (fileStat.st_mode & S_IWGRP) ? 'W' : '-',
                     '-',
                     (fileStat.st_mode & S_IROTH) ? 'R' : '-',
                     (fileStat.st_mode & S_IWOTH) ? 'W' : '-',
                     '-'
    );

    write(statFile, buffer, stats);
}


void processDirectoryEntries(const char *inputPath, const char *outputPath, struct  dirent *entry) {

    char input[1024];
    char output[1024];
    sprintf(input, "%s/%s", inputPath, entry->d_name);
    sprintf(output, "%s/%s_statistica.txt", outputPath, entry->d_name);

    struct stat fileStat;
    if (lstat(input, &fileStat) < 0) {
        perror("Error getting file stats");
        exit(EXIT_FAILURE);
    }

    int isBmp = S_ISREG(fileStat.st_mode) && strstr(entry->d_name, ".bmp");
    int isDir = S_ISDIR(fileStat.st_mode);
    int isLnk = S_ISLNK(fileStat.st_mode);

    getStatistics(input, output, isBmp, isDir, isLnk);

}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        write(STDOUT_FILENO, "Usage: ./program <director_intrare>\n", 37);
        return -1;
    }

    DIR *dir = opendir(argv[1]);
    if (dir == NULL) {
        perror("eroare deschidere folder");
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        pid_t process = fork();
        if (process == -1)
        {
            perror("eroare creare proces");
            continue;
        }

        if (process == 0)
        {
            processDirectoryEntries(argv[1], argv[2], entry);
            exit(EXIT_SUCCESS);
        }

    }

}

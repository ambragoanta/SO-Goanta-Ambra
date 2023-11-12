#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/stat.h>
#include<unistd.h>
#include <time.h>

int main(int argc, char **argv) {
    if(argc != 2) {
        write(STDOUT_FILENO, "Usage: ./program <fisier_intrare>\n", 35);
        exit(-1);
    }

    int fd_in = open(argv[1], O_RDONLY);
    if(fd_in < 0) {
        perror("eroare deschidere bmp");
        return -1;
    }

    lseek(fd_in, 18, SEEK_SET);
    int32_t inaltime, lungime;
    read(fd_in, &lungime, sizeof(lungime));
    read(fd_in, &inaltime, sizeof(inaltime));

    int fd_out = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
    if(fd_out < 0) {
        perror("eroare deschidere fisier statistica\n");
        close(fd_in);
        return -1;
    }

    struct stat file_stat;
    if(fstat(fd_in, &file_stat) < 0){
        perror("eroare obtinere statistica\n");
        close(fd_in);
        return -1;
    }

    char output[1000];
    sprintf(output, "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\ndrepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c\ndrepturi de acces altii: %c%c%c\n",
            argv[1], inaltime, lungime, file_stat.st_size, file_stat.st_uid, ctime(&file_stat.st_mtime), file_stat.st_nlink,
            (file_stat.st_mode & S_IRUSR) ? 'R' : '-',
            (file_stat.st_mode & S_IWUSR) ? 'W' : '-',
            (file_stat.st_mode & S_IXUSR) ? 'X' : '-',
            (file_stat.st_mode & S_IRGRP) ? 'R' : '-',
            (file_stat.st_mode & S_IWGRP) ? 'W' : '-',
            '-',
            (file_stat.st_mode & S_IROTH) ? 'R' : '-',
            (file_stat.st_mode & S_IWOTH) ? 'W' : '-',
            '-');

    if(write(fd_out, output, strlen(output)) < 0) {
        perror("eroare scriere in fisier de statistica\n");
        close(fd_in);
        return -1;
    }

    close(fd_in);
    close(fd_out);

    return 0;
}

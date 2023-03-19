#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFFER_SIZE 200

void handle_second_process(int in_fifo_fd, int out_fifo_fd) {
    // Read from input FIFO and write to output FIFO with vowel letters capitalized
    char buffer[BUFFER_SIZE];
    int bytes_read;
    while ((bytes_read = read(in_fifo_fd, buffer, BUFFER_SIZE)) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            if (isalpha(buffer[i]) && !strchr("AEIOUYaeiouy", buffer[i])) {
                buffer[i] = toupper(buffer[i]);
            }
        }

        if (write(out_fifo_fd, buffer, bytes_read) == -1) {
            perror("Failed to write to output FIFO");
            exit(EXIT_FAILURE);
        }
    }

    close(in_fifo_fd);
    close(out_fifo_fd);
}

int main() {
    // Open input FIFO for reading
    int in_fifo_fd = open("pipe1", O_RDONLY);
    if (in_fifo_fd == -1) {
        perror("Failed to open pipe1 for reading");
        exit(EXIT_FAILURE);
    }

    // Open output FIFO for writing
    int out_fifo_fd = open("pipe2", O_WRONLY);
    if (out_fifo_fd == -1) {
        perror("Failed to open pipe2 for writing");
        exit(EXIT_FAILURE);
    }

    // Handle second process
    handle_second_process(in_fifo_fd, out_fifo_fd);

    return 0;
}

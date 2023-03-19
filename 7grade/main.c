#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define BUFFER_SIZE 5000

void handle_first_process(char *input_file, char *output_file, int in_fifo_fd, int out_fifo_fd) {
    // Open file for reading
    int file_fd = open(input_file, O_RDONLY);
    if (file_fd == -1) {
        perror("Failed to open file for reading");
        exit(EXIT_FAILURE);
    }

    // Open file for writing
    int output_file_fd = open(output_file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (output_file_fd == -1) {
        perror("Failed to open file for writing");
        exit(EXIT_FAILURE);
    }

    // Read from file and write to input FIFO
    char buffer[BUFFER_SIZE];
    int bytes_read;
    while ((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
        if (write(in_fifo_fd, buffer, bytes_read) == -1) {
            perror("Failed to write to input FIFO");
            exit(EXIT_FAILURE);
        }
    }
    close(in_fifo_fd);

    // Read from output FIFO and write to file
    while ((bytes_read = read(out_fifo_fd, buffer, BUFFER_SIZE)) > 0) {
        if (write(output_file_fd, buffer, bytes_read) == -1) {
            perror("Failed to write to output file");
            exit(EXIT_FAILURE);
        }
    }

    // Close file and output FIFO
    close(file_fd);
    close(out_fifo_fd);
    close(output_file_fd);
}

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

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input_file output_file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Create input FIFO
    mkfifo("pipe1", S_IRUSR | S_IWUSR);
    mkfifo("pipe2", S_IRUSR | S_IWUSR);

    // Fork first child process
    pid_t first_child_pid = fork();
    if (first_child_pid == -1) {
        perror("Failed to fork first child process");
        exit(EXIT_FAILURE);
    } else if (first_child_pid == 0) {
        // Child process - handle first process
        int pipe1_fd = open("pipe1", O_WRONLY);
        if (pipe1_fd == -1) {
            perror("Failed to open pipe1 for reading");
            exit(EXIT_FAILURE);
        }
        int pipe2_fd = open("pipe2", O_RDONLY);
        if (pipe2_fd == -1) {
            perror("Failed to open pipe2 for writing");
            exit(EXIT_FAILURE);
        }
        handle_first_process(argv[1], argv[2], pipe1_fd, pipe2_fd);
        exit(EXIT_SUCCESS);
    }

    // Fork second child process
    pid_t second_child_pid = fork();
    if (second_child_pid == -1) {
        perror("Failed to fork second child process");
        exit(EXIT_FAILURE);
    } else if (second_child_pid == 0) {
        // Child process - handle second process
        int pipe1_fd = open("pipe1", O_RDONLY);
        if (pipe1_fd == -1) {
            perror("Failed to open pipe1 for reading");
            exit(EXIT_FAILURE);
        }
        int pipe2_fd = open("pipe2", O_WRONLY);
        if (pipe2_fd == -1) {
            perror("Failed to open pipe2 for writing");
            exit(EXIT_FAILURE);
        }
        handle_second_process(pipe1_fd, pipe2_fd);
        exit(EXIT_SUCCESS);
    }

    // Wait for child processes to exit
    int status;
    waitpid(first_child_pid, &status, 0);
    waitpid(second_child_pid, &status, 0);

    // Remove named pipes
    unlink("pipe1");
    unlink("pipe2");
    return 0;
}

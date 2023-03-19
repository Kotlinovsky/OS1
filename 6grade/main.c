#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdio.h>

#define BUFFER_SIZE 5000

void handle_first_process(char *input_file, char *output_file, int pipe1_fd[], int pipe2_fd[]) {
    close(pipe1_fd[0]);
    close(pipe2_fd[1]);

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

    // Read from file and write to pipe1
    char buffer[BUFFER_SIZE];
    int bytes_read;
    while ((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
        if (write(pipe1_fd[1], buffer, bytes_read) == -1) {
            perror("Failed to write to pipe1");
            exit(EXIT_FAILURE);
        }
    }
    close(pipe1_fd[1]);

    // Read from pipe2
    while ((bytes_read = read(pipe2_fd[0], buffer, BUFFER_SIZE)) > 0) {
        if (write(output_file_fd, buffer, bytes_read) == -1) {
            perror("Failed to write to output file");
            exit(EXIT_FAILURE);
        }
    }

    // Close file and pipe write end
    close(file_fd);
    close(pipe2_fd[1]);
    close(output_file_fd);
}

void handle_second_process(int pipe1_fd[], int pipe2_fd[]) {
    close(pipe1_fd[1]);
    close(pipe2_fd[0]);

    // Read from pipe1 and write to pipe2 with vowel letters capitalized
    char buffer[BUFFER_SIZE];
    int bytes_read;
    while ((bytes_read = read(pipe1_fd[0], buffer, BUFFER_SIZE)) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            if (isalpha(buffer[i]) && !strchr("AEIOUYaeiouy", buffer[i])) {
                buffer[i] = toupper(buffer[i]);
            }
        }

        if (write(pipe2_fd[1], buffer, bytes_read) == -1) {
            perror("Failed to write to pipe2");
            exit(EXIT_FAILURE);
        }
    }

    close(pipe1_fd[0]);
    close(pipe2_fd[1]);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input_file output_file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Create pipes
    int pipe1_fd[2], pipe2_fd[2];
    if (pipe(pipe1_fd) == -1 || pipe(pipe2_fd) == -1) {
        perror("Failed to create pipes");
        exit(EXIT_FAILURE);
    }

    // Fork first child process
    pid_t first_child_pid = fork();
    if (first_child_pid == -1) {
        perror("Failed to fork first child process");
        exit(EXIT_FAILURE);
    } else if (first_child_pid == 0) {
        // Child process - handle first process
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
        handle_second_process(pipe1_fd, pipe2_fd);
        exit(EXIT_SUCCESS);
    }

    // Parent process - close unused pipe ends
    close(pipe1_fd[0]);
    close(pipe1_fd[1]);
    close(pipe2_fd[0]);
    close(pipe2_fd[1]);

    // Wait for child processes to exit
    int status;
    waitpid(first_child_pid, &status, 0);
    waitpid(second_child_pid, &status, 0);
    return 0;
}

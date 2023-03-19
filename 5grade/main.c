#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define BUFFER_SIZE 5000

void handle_first_process(char *file_path, int write_fd) {
    // Open file for reading
    int file_fd = open(file_path, O_RDONLY);
    if (file_fd == -1) {
        perror("Failed to open file for reading");
        exit(EXIT_FAILURE);
    }

    // Read from file and write to pipe
    char buffer[BUFFER_SIZE];
    int bytes_read;
    while ((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
        if (write(write_fd, buffer, bytes_read) == -1) {
            perror("Failed to write to pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Close file and pipe write end
    close(file_fd);
    close(write_fd);
}

void handle_second_process(int read_fd, int write_fd) {
    // Read from pipe1 and write to pipe2 with vowel letters capitalized
    char buffer[BUFFER_SIZE];
    int bytes_read;
    while ((bytes_read = read(read_fd, buffer, BUFFER_SIZE)) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            if (isalpha(buffer[i]) && !strchr("AEIOUYaeiouy", buffer[i])) {
                buffer[i] = toupper(buffer[i]);
            }
        }

        if (write(write_fd, buffer, bytes_read) == -1) {
            perror("Failed to write to pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Close pipe1 and pipe2 write ends
    close(read_fd);
    close(write_fd);
}

void handle_third_process(char *file_path, int read_fd) {
    // Open file for writing
    int file_fd = open(file_path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (file_fd == -1) {
        perror("Failed to open file for writing");
        exit(EXIT_FAILURE);
    }

    // Read from pipe and write to file
    char buffer[BUFFER_SIZE];
    int bytes_read;
    while ((bytes_read = read(read_fd, buffer, BUFFER_SIZE)) > 0) {
        if (write(file_fd, buffer, bytes_read) == -1) {
            perror("Failed to write to file");
            exit(EXIT_FAILURE);
        }
    }

    // Close file and pipe read end
    close(file_fd);
    close(read_fd);
}

int main(int argc, char *argv[]) {
    // Check for correct number of arguments
    if (argc != 3) {
        printf("Usage: %s input_file output_file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Create named pipes
    const char *pipe1_name = "pipe1";
    const char *pipe2_name = "pipe2";
    mkfifo(pipe1_name, S_IRUSR | S_IWUSR);
    mkfifo(pipe2_name, S_IRUSR | S_IWUSR);

    // Fork first child process
    pid_t first_child_pid = fork();
    if (first_child_pid == -1) {
        perror("Failed to fork first child process");
        exit(EXIT_FAILURE);
    } else if (first_child_pid == 0) {
        // First child process reads from input file and writes to pipe1
        int pipe1_fd = open(pipe1_name, O_WRONLY);
        if (pipe1_fd == -1) {
            perror("Failed to open pipe1 for writing");
            exit(EXIT_FAILURE);
        }
        handle_first_process(argv[1], pipe1_fd);
        exit(EXIT_SUCCESS);
    }

    // Fork second child process
    pid_t second_child_pid = fork();
    if (second_child_pid == -1) {
        perror("Failed to fork second child process");
        exit(EXIT_FAILURE);
    } else if (second_child_pid == 0) {
        // Second child process reads from pipe1 and writes to pipe2 with vowel letters capitalized
        int pipe1_fd = open(pipe1_name, O_RDONLY);
        if (pipe1_fd == -1) {
            perror("Failed to open pipe1 for reading");
            exit(EXIT_FAILURE);
        }
        int pipe2_fd = open(pipe2_name, O_WRONLY);
        if (pipe2_fd == -1) {
            perror("Failed to open pipe2 for writing");
            exit(EXIT_FAILURE);
        }
        handle_second_process(pipe1_fd, pipe2_fd);
        exit(EXIT_SUCCESS);
    }

    // Fork third child process
    pid_t third_child_pid = fork();
    if (third_child_pid == -1) {
        perror("Failed to fork third child process");
        exit(EXIT_FAILURE);
    } else if (third_child_pid == 0) {
        // Third child process reads from pipe2 and writes to output file
        int pipe2_fd = open(pipe2_name, O_RDONLY);
        if (pipe2_fd == -1) {
            perror("Failed to open pipe2 for reading");
            exit(EXIT_FAILURE);
        }
        handle_third_process(argv[2], pipe2_fd);
        exit(EXIT_SUCCESS);
    }

    // Parent process waits for all child processes to finish
    int status;
    waitpid(first_child_pid, &status, 0);
    waitpid(second_child_pid, &status, 0);
    waitpid(third_child_pid, &status, 0);

    // Remove named pipes
    unlink(pipe1_name);
    unlink(pipe2_name);
    return 0;
}

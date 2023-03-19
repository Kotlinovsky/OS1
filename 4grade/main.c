#include <sys/wait.h>
#include <unistdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#define BUFFER_SIZE 5000

void handle_first_process(char *file_path, int pipe_fd[]) {
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
        if (write(pipe_fd[1], buffer, bytes_read) == -1) {
            perror("Failed to write to pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Close file and pipe write end
    close(file_fd);
    close(pipe_fd[1]);
}

void handle_second_process(int pipe1_fd[], int pipe2_fd[]) {
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
            perror("Failed to write to pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Close pipe1 and pipe2 write ends
    close(pipe1_fd[0]);
    close(pipe2_fd[1]);
}

void handle_third_process(char *file_path, int pipe_fd[]) {
    // Open file for writing
    int file_fd = open(file_path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (file_fd == -1) {
        perror("Failed to open file for writing");
        exit(EXIT_FAILURE);
    }

    // Read from pipe and write to file
    char buffer[BUFFER_SIZE];
    int bytes_read;
    while ((bytes_read = read(pipe_fd[0], buffer, BUFFER_SIZE)) > 0) {
        if (write(file_fd, buffer, bytes_read) == -1) {
            perror("Failed to write to file");
            exit(EXIT_FAILURE);
        }
    }

    // Close file and pipe read end
    close(file_fd);
    close(pipe_fd[0]);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Create pipes
    int pipe1_fd[2], pipe2_fd[2];
    if (pipe(pipe1_fd) == -1 || pipe(pipe2_fd) == -1) {
        perror("Failed to create pipe");
        exit(EXIT_FAILURE);
    }

    // Fork first child process
    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("Failed to fork first child process");
        exit(EXIT_FAILURE);
    } else if (pid1 == 0) {
        // Child process 1
        close(pipe1_fd[0]); // close pipe1 read end
        close(pipe2_fd[0]);
        close(pipe2_fd[1]);
        handle_first_process(argv[1], pipe1_fd);
        exit(EXIT_SUCCESS);
    }

    // Fork second child process
    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("Failed to fork second child process");
        exit(EXIT_FAILURE);
    } else if (pid2 == 0) {
        // Child process 2
        close(pipe1_fd[1]); // close pipe1 write end
        close(pipe2_fd[0]); // close pipe2 read end
        handle_second_process(pipe1_fd, pipe2_fd);
        exit(EXIT_SUCCESS);
    }

    // Fork third child process
    pid_t pid3 = fork();
    if (pid3 == -1) {
        perror("Failed to fork third child process");
        exit(EXIT_FAILURE);
    } else if (pid3 == 0) {
        // Child process 3
        close(pipe1_fd[0]);
        close(pipe1_fd[1]);
        close(pipe2_fd[1]); // close pipe2 write end
        handle_third_process(argv[2], pipe2_fd);
        exit(EXIT_SUCCESS);
    }

    // Parent process
    close(pipe1_fd[0]);
    close(pipe1_fd[1]);
    close(pipe2_fd[0]);
    close(pipe2_fd[1]);

    // Wait for all child processes to finish
    int status;
    waitpid(pid1, &status, 0);
    waitpid(pid2, &status, 0);
    waitpid(pid3, &status, 0);

    return 0;
}

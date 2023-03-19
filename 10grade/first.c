#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>

#define BUFFER_SIZE 200

void handle_first_process(char *input_file, char *output_file, mqd_t in_mq, mqd_t out_mq) {
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

    // Read from file and send to input message queue
    char buffer[BUFFER_SIZE];
    int bytes_read;
    while ((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
        if (mq_send(in_mq, buffer, bytes_read, 0) == -1) {
            perror("Failed to send message to input message queue");
            exit(EXIT_FAILURE);
        }
    }

    // Close file
    close(file_fd);

    // Receive from output message queue and write to file
    while ((bytes_read = mq_receive(out_mq, buffer, 10000000, NULL)) > 0) {
        if (write(output_file_fd, buffer, bytes_read) == -1) {
            perror("Failed to write to output file");
            exit(EXIT_FAILURE);
        }
    }

    // Close file and message queues
    close(out_mq);
    mq_close(in_mq);
    mq_close(out_mq);
    close(output_file_fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input_file output_file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Open input queue for reading
    mqd_t in_mq = mq_open("/input_mq", O_WRONLY);
    if (in_mq == (mqd_t)-1) {
        perror("Failed to open input queue for reading");
        exit(EXIT_FAILURE);
    }

    mqd_t out_mq = mq_open("/output_mq", O_RDONLY);
    if (out_mq == (mqd_t)-1) {
        perror("Failed to open output queue for writing");
        exit(EXIT_FAILURE);
    }

    handle_first_process(argv[1], argv[2], in_mq, out_mq);
    return 0;
}

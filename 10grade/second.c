#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>

#define BUFFER_SIZE 200
#define MAX_MSG_SIZE BUFFER_SIZE

struct message {
    long type;
    char text[BUFFER_SIZE];
};

void handle_second_process(mqd_t in_queue, mqd_t out_queue) {
    // Read from input queue and write to output queue with vowel letters capitalized
    struct message msg;
    ssize_t bytes_read;
    while ((bytes_read = mq_receive(in_queue, (char*)&msg, 10000000, NULL)) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            if (isalpha(msg.text[i]) && !strchr("AEIOUYaeiouy", msg.text[i])) {
                msg.text[i] = toupper(msg.text[i]);
            }
        }

        if (mq_send(out_queue, (const char*)&msg, bytes_read, 0) == -1) {
            perror("Failed to send message to output queue");
            exit(EXIT_FAILURE);
        }
    }

    mq_close(in_queue);
    mq_close(out_queue);
}

int main() {
    // Create input and output message queues
    mqd_t in_queue = mq_open("/input_mq", O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR, NULL);
    if (in_queue == -1) {
        perror("Failed to create input message queue");
        exit(EXIT_FAILURE);
    }

    // Open output queue for writing
    mqd_t out_queue = mq_open("/output_mq", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR, NULL);
    if (out_queue == -1) {
        perror("Failed to create output message queue");
        mq_close(in_queue);
        mq_unlink("/input_mq");
        exit(EXIT_FAILURE);
    }

    // Handle second process
    handle_second_process(in_queue, out_queue);

    // Remove message queues
    mq_unlink("/input_mq");
    mq_unlink("/output_mq");
    return 0;
}

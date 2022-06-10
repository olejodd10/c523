#include "tcp_monitor.h"

#define PORT 0 // OS picks random port
#define TCP_BUFFER_LEN 1024 // Size of buffer receiving TCP messages

static int noteActive[NUM_NOTES];

// Returns socket file descriptor bound to port. Updates *port to match what it's actually bound to
int initSocket(int* port) {
    // 0 as tcp_protocol_number will also work, see man socket
    int lsockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // Something easier than SOCK_STREAM is probably possible
    struct sockaddr_in sock = {.sin_family = AF_INET, .sin_port = htons(*port), .sin_addr = htonl(INADDR_ANY)};
    socklen_t addrlen = sizeof(sock);
    int err = bind(lsockfd, (struct sockaddr*)&sock, addrlen);
    if (err < 0) {
        printf("Failed while binding\n");
        exit(EXIT_FAILURE);
    }
    err = getsockname(lsockfd, (struct sockaddr*)&sock, &addrlen);
    if (err < 0) {
        printf("Failed while getting socket name\n");
        exit(EXIT_FAILURE);
    }
    *port = ntohs(sock.sin_port);
    return lsockfd;
}

void *monitorTcp(void *arg) {
    char buffer[TCP_BUFFER_LEN];
    int port = PORT; 
    int lsockfd = initSocket(&port);
    struct sockaddr_in sock;
    socklen_t addrlen = sizeof(sock);
    int err = listen(lsockfd, 1); // Listen for only one connection
    if (err < 0) {
        printf("Failed while listening\n");
        exit(EXIT_FAILURE);
    }
    printf("Listening has started on port %d\n", port);
    int rsockfd = accept(lsockfd, (struct sockaddr*)&sock, &addrlen);
    printf("Accepted new connection\n");
    while (1) {
        err = read(rsockfd, buffer, TCP_BUFFER_LEN);
        for (int i = 0; i < err; ++i) {
            Note note = charToNote(buffer[i]);
            if ((0 <= note) && (note < NUM_NOTES)) {
                noteActive[(int)note] = !noteActive[(int)note];
            }
        } 
        // send(rsockfd, hello, strlen(hello), 0);
    }
    close(lsockfd); // Close file descriptor
    shutdown(lsockfd, SHUT_RDWR); // Shutting down, stopping all receptions and transmissions
}

int isActiveTcp(Note note) {
    return noteActive[(int)note];
}

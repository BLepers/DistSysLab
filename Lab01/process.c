/*
 * process.c - Minimal TCP client/server demo on port 9999.
 *
 * Usage:
 *   ./app 0 <anything>    run as server: bind, listen, accept one connection,
 *									recv one message, send one reply, exit.
 *   ./app 1 <anything>    run as client: connect, send one message,
 *									recv one reply, exit.
 *
 * The code has little to do with the actual assignment, it is just here as
 * an example of code that uses the hooks defined in hooks.c
 * In the assignment, all processes have the same role (there is no "server" nor
 * any "client") and the code must work with more than 2 processes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT      9999
#define BACKLOG   1
#define BUF_SIZE  256

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

static void run_server(void) {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) die("socket");

    /* Allow quick restart without waiting out TIME_WAIT */
    int yes = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(PORT);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        die("bind");

    if (listen(listen_fd, BACKLOG) < 0)
        die("listen");

    printf("[server] listening on port %d...\n", PORT);

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
    if (conn_fd < 0) die("accept");

    printf("[server] accepted connection from %s:%d\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    char buf[BUF_SIZE];
    ssize_t n = recv(conn_fd, buf, sizeof(buf) - 1, 0);
    if (n < 0) die("recv");
    buf[n] = '\0';
    printf("[server] received: \"%s\"\n", buf);

    const char *reply = "hello from server";
    if (send(conn_fd, reply, strlen(reply), 0) < 0)
        die("send");
    printf("[server] sent reply: \"%s\"\n", reply);

    close(conn_fd);
    close(listen_fd);
}

static void run_client(void) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) die("socket");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0)
        die("inet_pton");

    if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        die("connect");

    printf("[client] connected to 127.0.0.1:%d\n", PORT);

    const char *msg = "hello from client";
    if (send(sock_fd, msg, strlen(msg), 0) < 0)
        die("send");
    printf("[client] sent: \"%s\"\n", msg);

    char buf[BUF_SIZE];
    ssize_t n = recv(sock_fd, buf, sizeof(buf) - 1, 0);
    if (n < 0) die("recv");
    buf[n] = '\0';
    printf("[client] received reply: \"%s\"\n", buf);

    close(sock_fd);
}

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s <id> <file>\n", prog);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (atoi(argv[1]) == 0) {
        run_server();
    } else {
		  sleep(1);
        run_client();
    }

    return EXIT_SUCCESS;
}

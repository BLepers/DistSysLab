/*
 * hook.c - LD_PRELOAD shim that intercepts socket-related functions
 *
 * How it works:
 *   - This shared library is loaded before libc via LD_PRELOAD.
 *   - Each function below has the exact same signature as its libc counterpart,
 *     so the dynamic linker resolves calls to OUR version first.
 *   - We use dlsym(RTLD_NEXT, ...) to find the *real* libc function at runtime,
 *     cache it in a function pointer, print a trace message, then call through.
 *
 * Build:
 *   gcc -shared -fPIC -o hook.so hook.c -ldl
 *
 * Use:
 *   LD_PRELOAD=./hook.so ./your_program
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

/* ---- Function pointer types matching the real libc prototypes ---- */
typedef int     (*accept_t)(int, struct sockaddr *, socklen_t *);
typedef int     (*connect_t)(int, const struct sockaddr *, socklen_t);
typedef ssize_t (*send_t)(int, const void *, size_t, int);
typedef ssize_t (*recv_t)(int, void *, size_t, int);

/* ---- Cached pointers to the real functions, resolved lazily ---- */
static accept_t  real_accept  = NULL;
static connect_t real_connect = NULL;
static send_t    real_send    = NULL;
static recv_t    real_recv    = NULL;

/* Helper macro: resolve a symbol once via dlsym(RTLD_NEXT, ...) */
#define RESOLVE(fptr, name)                                            \
    do {                                                               \
        if (!(fptr)) {                                                 \
            (fptr) = (void *)dlsym(RTLD_NEXT, (name));                 \
            if (!(fptr)) {                                             \
                fprintf(stderr, "hook: dlsym failed for %s: %s\n",     \
                        (name), dlerror());                            \
                exit(EXIT_FAILURE);                                    \
            }                                                          \
        }                                                               \
    } while (0)

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    RESOLVE(real_accept, "accept");
    fprintf(stderr, "[hook] accept() called (sockfd=%d)\n", sockfd);
    return real_accept(sockfd, addr, addrlen);
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    RESOLVE(real_connect, "connect");
    fprintf(stderr, "[hook] connect() called (sockfd=%d)\n", sockfd);
    return real_connect(sockfd, addr, addrlen);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
    RESOLVE(real_send, "send");
    fprintf(stderr, "[hook] send() called (sockfd=%d, len=%zu, flags=%d)\n",
            sockfd, len, flags);
    return real_send(sockfd, buf, len, flags);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    RESOLVE(real_recv, "recv");
    fprintf(stderr, "[hook] recv() called (sockfd=%d, len=%zu, flags=%d)\n",
            sockfd, len, flags);
    return real_recv(sockfd, buf, len, flags);
}

#include <sys/epoll.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>


#include "log.h"

static int non_block(int fd) {
    if(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK) == -1) {
        un_log_errno(LOG_ERR, "failed to set socket non blocking");
        return -1;
    }
    return 0;
}

static char* majic = "unit-ipc";
#define IPC_HEADER_SIZE ((int)(sizeof(majic) + 8))

static int sockfd = -1;
static int epollfd = -1;
static int nfds = 0;
struct epoll_event ev;
struct epoll_event events[10];

int main(int argc, char** argv) {
    int uffer = 17;
    un_log(LOG_DEBUG, "oh sheeet");
    if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        un_log_errno(LOG_ERR, "socket failed");
        un_log_errno(LOG_ERR, "portent: %d", uffer);
        return -1;
    }
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    char* sock_dir = NULL;
    if((sock_dir = getenv("XDG_RUNTIME_DIR")) == NULL) {
        sock_dir = "/tmp";
    }    
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/%s", sock_dir, "unitd.sock");
    un_log(LOG_INFO, "sock path: %s", addr.sun_path);
    unlink(addr.sun_path);
    if(bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        un_log_errno(LOG_ERR, "bind failed");
        return -1;
    }
    if(listen(sockfd, 5) == -1) {
        un_log_errno(LOG_ERR, "listen failed");
        return -1;
    }
    char* mag = "unit-ipc";
    char* msg = "Hello lad";
    printf("%s%d%d%s\n", mag, 0, sizeof(msg), msg);
    epollfd = epoll_create(8);
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev) == -1) {
        un_log_errno(LOG_ERR, "epoll failed");
        return -1;
    }
    socklen_t addrlen = sizeof(addr);
    for(;;) {
        nfds = epoll_wait(epollfd, events, 10, -1);
        if(nfds == -1) {
            un_log_errno(LOG_ERR, "epoll wait fails");
            return -1;
        }

        for(int i = 0; i < nfds; i++) {
            if(events[i].data.fd == sockfd) {
                int cfd = accept(sockfd, (struct sockaddr*)&addr, &addrlen);
                if(cfd == -1) {
                    un_log_errno(LOG_ERR, "accept failed");
                    return -1;
                }
                un_log(LOG_INFO, "accepting client: %d", cfd);
                if(non_block(cfd) < 0) {
                    return -1;
                }
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = cfd;
                if(epoll_ctl(epollfd, EPOLL_CTL_ADD, cfd, &ev) == -1) {
                    un_log_errno(LOG_ERR, "unable to add client to epoll");
                }
            } else if(events[i].events & EPOLLIN) {
                char buf[2048];
                size_t nb = read(events[i].data.fd, buf, sizeof(buf));
                buf[nb - 1] = '\0';
                if(memcmp(buf, mag, sizeof(mag) != 0)) {
                    un_log(LOG_ERR, "client: %d failed magic", events[i].data.fd);
                    break;
                }
                char* buf2 = buf + sizeof(majic) + sizeof(int);
                int msg_size = *((int*)buf2);
                un_log(LOG_DEBUG, "sz: %d %.*s", msg_size, msg_size, buf + IPC_HEADER_SIZE);                
            }
        }
    }
    return 0;
}
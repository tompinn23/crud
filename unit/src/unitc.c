#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"

static char* majic = "unit-ipc";
#define IPC_HEADER_SIZE ((int)(sizeof(majic) + 8))

void send_msg(int fd, const char* str) {
    char buf[4096];
    memcpy(buf, majic, sizeof(majic));
    *((int*)(buf + sizeof(majic))) = 0;
    *((int*)(buf + sizeof(majic) + sizeof(int))) = strlen(str);
    memcpy(buf + IPC_HEADER_SIZE, str, strlen(str)); //FECKING BUFFER OVERFLOW HYPE
    write(STDOUT_FILENO, buf, strlen(str) + IPC_HEADER_SIZE);
    write(fd, buf, strlen(str) + IPC_HEADER_SIZE);
}

int main(int argc, char** argv) {
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    
    char* sock_dir = NULL;
    if((sock_dir = getenv("XDG_RUNTIME_DIR")) == NULL) {
        sock_dir = "/tmp";
    }    
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/%s", sock_dir, "unitd.sock");
    un_log(LOG_INFO, "sock path: %s", addr.sun_path);
    connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    send_msg(sockfd, "{\"cmd\": \"alacritty\", \"args\": []}");
}
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <linux/limits.h>

#define IPADDRESS "192.168.0.110"
#define PORT 12345


int to_client(int client, const char *path)
{
    int ret_val = 0;
    ssize_t read_bytes = 0;
    int file;
    static uint8_t image_num = 0;
    uint32_t image_size = 0;
    char file_name[PATH_MAX];

    if (sprintf(file_name, "%simage_%u.jpg", path, image_num) < 0) {
        printf("sprintf() error, %d\n", errno);
        return errno;
    }

    ++image_num;
    char * buffer = (char *)&image_size;

    for(int i = 0; i < sizeof(uint32_t); i += read_bytes) {
        read_bytes = read(client, (buffer + i), sizeof(uint32_t) - i);
        if (read_bytes == -1) {
            printf("read() image_size error, %d\n", errno);
            return errno;
        }
    }

    image_size = ntohl(image_size);
    if (image_size <= 0) {
        printf("image_size <= 0\n");
        return 0;
    }

    file = open(file_name, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if (file == -1) {
        printf("open() file error, %d\n", errno);
        return errno;
    }
    if (ftruncate(file, image_size) == -1) {
        printf("ftruncate() file error, %d\n", errno);
        ret_val = errno;
        goto close;
    }

    char *buffer_mmap = mmap(NULL, image_size, PROT_WRITE, MAP_SHARED, file, 0);
    if (buffer_mmap == (void *)-1) {
        printf("mmap() buffer_mmap error, %d\n", errno);
        ret_val = errno;
        goto close;
    }

    for (int i = 0, read_bytes = 0; i < image_size; i += read_bytes) {
        printf("Reading, %d bytes left...\n", image_size - i);
        read_bytes = read(client, buffer_mmap + i, image_size - i);
        if (read_bytes == -1) {
            printf("read() image error, %d\n", errno);
            ret_val = errno;
            goto munmap;
        }
    }
munmap:
    munmap(buffer_mmap, image_size);
close:
    close(file);

    return ret_val;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("No path to file\n");
        return 1;
    }

    if (access(argv[1], W_OK) != 0) {
        printf("Invalid path to file\n");
        return errno;
    }

    int ret_val = 0;
    const char *path = argv[1];

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1) {
        printf("socket() error, %d\n", errno);
        return errno;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_aton(IPADDRESS, &addr.sin_addr);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        printf("bind() sock error, %d\n", errno);
        ret_val = errno;
        goto out;
    }
    if (listen(sock, 1) == -1) {
        printf("listen() sock error, %d\n", errno);
        ret_val = errno;
        goto out;
    }

    while (1) {
        printf("Waiting for a new client...\n");

        int client_sock = accept(sock, NULL, NULL);
        if (client_sock == -1) {
            printf("accept() sock error, %d\n", errno);
            ret_val = errno;
            goto out;
        }

        printf("A new client found\n");

        if (to_client(client_sock, path) != 0)
            printf("Client error\n");
        close(client_sock);
        printf("Client finished communication\n");
    }

out:
    close(sock);

    return ret_val;
}

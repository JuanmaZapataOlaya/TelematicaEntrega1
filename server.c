#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "logger.h"

#define BUF_SIZE 8192

char *log_filename;
char *root_folder;

const char* get_mime_type(const char* path) {
    const char* ext = strrchr(path, '.');
    if (!ext) return "text/plain";
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".mp4") == 0) return "video/mp4";
    if (strcmp(ext, ".txt") == 0) return "text/plain";
    return "application/octet-stream";
}

void send_headers(int client_fd, int status, const char *type, size_t length, int accept_range) {
    char header[BUF_SIZE];
    snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n"
        "%s\r\n",
        status,
        (status == 200) ? "OK" :
        (status == 206) ? "Partial Content" :
        (status == 400) ? "Bad Request" :
        (status == 404) ? "Not Found" : "OK",
        type, length,
        accept_range ? "Accept-Ranges: bytes" : "");
    write(client_fd, header, strlen(header));
}

void not_found(int client_fd) {
    const char *body = "<html><body><h1>404 Not Found</h1></body></html>";
    send_headers(client_fd, 404, "text/html", strlen(body), 0);
    write(client_fd, body, strlen(body));
}

void bad_request(int client_fd) {
    const char *body = "<html><body><h1>400 Bad Request</h1></body></html>";
    send_headers(client_fd, 400, "text/html", strlen(body), 0);
    write(client_fd, body, strlen(body));
}

void post_success(int client_fd) {
    const char *body = "<html><body><h2>¡Formulario recibido exitosamente!</h2><a href=\"caso3.html\">Volver</a></body></html>";
    send_headers(client_fd, 200, "text/html", strlen(body), 0);
    write(client_fd, body, strlen(body));
}

void *handle_client(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    char buffer[BUF_SIZE];
    int bytes_read = read(client_fd, buffer, BUF_SIZE - 1);
    if (bytes_read <= 0) {
        close(client_fd);
        return NULL;
    }
    buffer[bytes_read] = '\0';

    char method[8], path[256];
    sscanf(buffer, "%s %s", method, path);

    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    getpeername(client_fd, (struct sockaddr *)&addr, &len);
    char *client_ip = inet_ntoa(addr.sin_addr);

    if (strcmp(method, "GET") != 0 && strcmp(method, "HEAD") != 0 && strcmp(method, "POST") != 0) {
        log_request(log_filename, client_ip, method, path, 400);
        bad_request(client_fd);
        close(client_fd);
        return NULL;
    }

    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s%s", root_folder, path);
    if (full_path[strlen(full_path) - 1] == '/')
        strcat(full_path, "index.html");

    if (strcmp(method, "POST") == 0) {
        const char *body_start = strstr(buffer, "\r\n\r\n");
        if (body_start) {
            body_start += 4;
            FILE *f = fopen("postdata.txt", "a");
            if (f) {
                fprintf(f, "[POST] %s - %s\n", path, body_start);
                fclose(f);
            }
        }
    }

    int fd = open(full_path, O_RDONLY);
    if (fd < 0) {
        log_request(log_filename, client_ip, method, path, 404);
        not_found(client_fd);
        close(client_fd);
        return NULL;
    }

    struct stat st;
    fstat(fd, &st);
    char *body = malloc(st.st_size);
    read(fd, body, st.st_size);
    close(fd);

    const char *mime = get_mime_type(full_path);
    int is_video = strstr(mime, "video") != NULL;

    if (strcmp(method, "HEAD") == 0) {
        send_headers(client_fd, 200, mime, st.st_size, is_video);
        log_request(log_filename, client_ip, method, path, 200);
    } else if (strcmp(method, "POST") == 0) {
        post_success(client_fd);
        log_request(log_filename, client_ip, method, path, 200);
    } else if (is_video) {
        char *range_header = strstr(buffer, "Range: bytes=");
        if (range_header) {
            int start = 0, end = st.st_size - 1;
            sscanf(range_header, "Range: bytes=%d-%d", &start, &end);
            if (end < start || end >= st.st_size) end = st.st_size - 1;
            int content_length = end - start + 1;

            char header[BUF_SIZE];
            snprintf(header, sizeof(header),
                     "HTTP/1.1 206 Partial Content\r\n"
                     "Content-Type: %s\r\n"
                     "Content-Length: %d\r\n"
                     "Content-Range: bytes %d-%d/%ld\r\n"
                     "Accept-Ranges: bytes\r\n"
                     "Connection: close\r\n\r\n",
                     mime, content_length, start, end, st.st_size);
            write(client_fd, header, strlen(header));
            write(client_fd, body + start, content_length);
            log_request(log_filename, client_ip, method, path, 206);
        } else {
            send_headers(client_fd, 200, mime, st.st_size, 1);
            write(client_fd, body, st.st_size);
            log_request(log_filename, client_ip, method, path, 200);
        }
    } else {
        send_headers(client_fd, 200, mime, st.st_size, 0);
        write(client_fd, body, st.st_size);
        log_request(log_filename, client_ip, method, path, 200);
    }

    free(body);
    close(client_fd);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <puerto> <log.txt> <www>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    log_filename = argv[2];
    root_folder = argv[3];

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, 10);
    printf("Servidor escuchando en puerto %d...\n", port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int *client_fd = malloc(sizeof(int));
        *client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, client_fd);
        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}

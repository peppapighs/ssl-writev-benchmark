#pragma once

#include "Config.h"

#include <arpa/inet.h>
#include <iostream>
#include <signal.h>
#include <sys/socket.h>
#include <vector>

#include <openssl/err.h>
#include <openssl/ssl.h>

namespace Server {
const size_t BUFFER_SIZE = 1 << 14;

int CreateSocket(int port) {
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }

    if (listen(s, 1) < 0) {
        perror("Unable to listen");
        exit(EXIT_FAILURE);
    }

    return s;
}

void SaveKeylog(const SSL *ssl, const char *line) {
    std::cout << line << std::endl;
}

SSL_CTX *CreateContext() {
    const SSL_METHOD *method = TLS_server_method();
    SSL_CTX *ctx = SSL_CTX_new(method);

    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void ConfigureContext(SSL_CTX *ctx) {
    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    SSL_CTX_set_keylog_callback(ctx, SaveKeylog);
}

void Run() {
    signal(SIGPIPE, SIG_IGN);

    SSL_CTX *ctx = CreateContext();
    ConfigureContext(ctx);
    int sock = CreateSocket(PORT);

    std::cerr << "Server started on port " << PORT << std::endl;

    struct sockaddr_in addr;
    unsigned int len = sizeof(addr);

    int client = accept(sock, (struct sockaddr *)&addr, &len);
    if (client < 0) {
        perror("Unable to accept");
        exit(EXIT_FAILURE);
    }

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client);

    if (SSL_accept(ssl) <= 0)
        ERR_print_errors_fp(stderr);

    char buf[BUFFER_SIZE];
    while (1) {
        int len = SSL_read(ssl, buf, sizeof(buf));
        if (len <= 0) {
            std::cerr << "SSL_read failed: " << len << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(client);

    close(sock);
    SSL_CTX_free(ctx);
}
} // namespace Server
#pragma once

#include "Clock.h"
#include "Config.h"
#include "Util.h"

#include <iostream>
#include <netdb.h>
#include <random>
#include <sys/uio.h>
#include <unistd.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

namespace Client {
const int ERROR_STATUS = -1;
const size_t BUFFER_SIZE = 1 << 14;

int CreateConnection(const char *hostname, const char *port) {
    struct hostent *host;
    if ((host = gethostbyname(hostname)) == nullptr) {
        perror(hostname);
        exit(EXIT_FAILURE);
    }

    struct addrinfo hints = {0}, *addrs;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    const int status = getaddrinfo(hostname, port, &hints, &addrs);
    if (status != 0) {
        fprintf(stderr, "%s: %s\n", hostname, gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    int sfd, err;
    for (struct addrinfo *addr = addrs; addr != nullptr; addr = addr->ai_next) {
        sfd = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
        if (sfd == ERROR_STATUS) {
            err = errno;
            continue;
        }

        if (connect(sfd, addr->ai_addr, addr->ai_addrlen) == 0) {
            break;
        }

        err = errno;
        sfd = ERROR_STATUS;
        close(sfd);
    }

    freeaddrinfo(addrs);

    if (sfd == ERROR_STATUS) {
        fprintf(stderr, "%s: %s\n", hostname, strerror(err));
        exit(EXIT_FAILURE);
    }
    return sfd;
}

SSL_CTX *CreateContext() {
    const SSL_METHOD *method = TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);

    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void ConfigureContext(SSL_CTX *ctx) {
    if (SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    if (SSL_CTX_set_max_proto_version(ctx, TLS1_2_VERSION) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

void Run() {
    SSL_CTX *ctx = CreateContext();
    ConfigureContext(ctx);

    SSL *ssl = SSL_new(ctx);
    const int sfd = CreateConnection("localhost", std::to_string(PORT).c_str());
    SSL_set_fd(ssl, sfd);

    const int status = SSL_connect(ssl);
    if (status != 1) {
        SSL_get_error(ssl, status);
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    std::cerr << "Connected with " << SSL_get_cipher(ssl) << " encryption"
              << std::endl;

    char buf[BUFFER_SIZE];

    std::mt19937 gen(RANDOM_SEED);
    std::default_random_engine engine(gen());
    std::uniform_int_distribution<int> dist(MIN_PACKET_SIZE, MAX_PACKET_SIZE);

    size_t totalLen = 0;
    struct iovec iov[NUM_IOV] = {0};

    while (1) {
        if (!SSL_read(ssl, buf, BUFFER_SIZE))
            continue;
        if (strcmp(buf, "start") == 0)
            break;
    }

    size_t written;
    std::vector<int64_t> times;
    for (int i = 0; i < NUM_PACKETS; i += NUM_IOV) {
        for (int j = 0; j < NUM_IOV; j++) {
            iov[j].iov_len = dist(engine);
            iov[j].iov_base = malloc(iov[j].iov_len);
            totalLen += iov[j].iov_len;
        }

        Clock clock;
#if NUM_IOV == 1
        SSL_write(ssl, iov[0].iov_base, iov[0].iov_len);
#else
        SSL_writev(ssl, iov, NUM_IOV, &written);
#endif

        if (SSL_read(ssl, buf, BUFFER_SIZE) < 0) {
            std::cerr << "Error reading from socket" << std::endl;
            exit(EXIT_FAILURE);
        }
        auto elapsed = clock.GetElapsedNanoseconds();
        times.emplace_back(elapsed / NUM_IOV);

        for (int j = 0; j < NUM_IOV; j++)
            free(iov[j].iov_base);

#if FLUSH_CACHE
        ClearCache();
#endif
    }
    SaveStats(STAT_NAME, times);

    std::cout << "Number of packets: " << NUM_PACKETS << std::endl;
    std::cout << "Number of packets per call: " << NUM_IOV << std::endl;
    std::cout << "Total number of bytes sent: " << totalLen << std::endl;

    PrintStats({STAT_NAME});

    SSL_free(ssl);
    close(sfd);
    SSL_CTX_free(ctx);
}
} // namespace Client
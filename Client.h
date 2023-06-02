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

void BenchWriteSpeed() {
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

    for (auto &name : testNames) {
        auto &testCase = testCases.at(name);
        struct iovec *iov = new struct iovec[testCase.size()];

        std::vector<int64_t> writeTimes;
        writeTimes.reserve(NUM_ITER);
        for (size_t i = 0; i < NUM_ITER; i++) {
            for (size_t j = 0; j < testCase.size(); j++) {
                iov[j].iov_base = malloc(testCase[j]);
                iov[j].iov_len = testCase[j];
            }

            Clock clock;
            for (size_t j = 0; j < testCase.size(); j++)
                SSL_write(ssl, iov[j].iov_base, iov[j].iov_len);
            writeTimes.push_back(clock.GetElapsedNanoseconds());

            for (size_t j = 0; j < testCase.size(); j++)
                free(iov[j].iov_base);
        }
        SaveStats("SSL_write()  " + name, writeTimes);

        std::vector<int64_t> writevTimes;
        writevTimes.reserve(NUM_ITER);
        size_t written;
        for (size_t i = 0; i < NUM_ITER; i++) {
            for (size_t j = 0; j < testCase.size(); j++) {
                iov[j].iov_base = malloc(testCase[j]);
                iov[j].iov_len = testCase[j];
            }

            Clock clock;
            SSL_writev(ssl, iov, testCase.size(), &written);
            writeTimes.push_back(clock.GetElapsedNanoseconds());

            for (size_t j = 0; j < testCase.size(); j++)
                free(iov[j].iov_base);
        }
        SaveStats("SSL_writev() " + name, writeTimes);

        free(iov);
    }

    std::vector<std::string> toPrint;
    for (auto &name : testNames) {
        toPrint.push_back("SSL_write()  " + name);
        toPrint.push_back("SSL_writev() " + name);
    }
    PrintStats(toPrint);

    SSL_free(ssl);
    close(sfd);
    SSL_CTX_free(ctx);
}
} // namespace Client
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
#if FLUSH_RECORD
char buf[1];
#endif

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

#if RANDOMIZE_DATA
    std::mt19937 gen;
    std::default_random_engine rng(gen());
    std::uniform_int_distribution<> dist(0, 255);
#endif

    size_t written;
    for (auto &name : testNames) {
        auto &testCase = testCases.at(name);
        struct iovec *iov = new struct iovec[testCase.size()];

        {
            std::vector<int64_t> writeTimes;
            writeTimes.reserve(NUM_ITER);
            for (size_t i = 0; i < NUM_ITER; i++) {
                for (size_t j = 0; j < testCase.size(); j++) {
                    iov[j].iov_base = malloc(testCase[j]);
                    iov[j].iov_len = testCase[j];

#if RANDOMIZE_DATA
                    for (size_t k = 0; k < testCase[j]; k++)
                        ((char *)iov[j].iov_base)[k] = dist(rng);
#endif
                }

                Clock clock;
                for (size_t j = 0; j < testCase.size(); j++)
                    SSL_write_ex(ssl, iov[j].iov_base, iov[j].iov_len,
                                 &written);
                writeTimes.push_back(clock.GetElapsedNanoseconds());

                for (size_t j = 0; j < testCase.size(); j++)
                    free(iov[j].iov_base);

#if FLUSH_RECORD
                for (size_t j = 0; j < testCase.size(); j++)
                    SSL_read(ssl, buf, 1);
#endif
            }
            SaveStats("SSL_write() " + name, writeTimes);
        }

#if BENCH_WRITE_MEMCPY
        {
            std::vector<int64_t> writeCopyTimes;
            writeCopyTimes.reserve(NUM_ITER);
            for (size_t i = 0; i < NUM_ITER; i++) {
                size_t len = 0, offset = 0;
                for (size_t j = 0; j < testCase.size(); j++) {
                    iov[j].iov_base = malloc(testCase[j]);
                    iov[j].iov_len = testCase[j];
                    len += testCase[j];

#if RANDOMIZE_DATA
                    for (size_t k = 0; k < testCase[j]; k++)
                        ((char *)iov[j].iov_base)[k] = dist(rng);
#endif
                }

                Clock clock;
                void *buf = malloc(len);
                for (size_t j = 0; j < testCase.size(); j++) {
                    memcpy((unsigned char *)buf + offset, iov[j].iov_base,
                           iov[j].iov_len);
                    offset += iov[j].iov_len;
                }
                SSL_write_ex(ssl, buf, len, &written);
                writeCopyTimes.push_back(clock.GetElapsedNanoseconds());

                for (size_t j = 0; j < testCase.size(); j++)
                    free(iov[j].iov_base);
                free(buf);

#if FLUSH_RECORD
                SSL_read(ssl, buf, 1);
#endif
            }
            SaveStats("SSL_write() with memcpy " + name, writeCopyTimes);
        }
#endif

        {
            std::vector<int64_t> writevTimes;
            writevTimes.reserve(NUM_ITER);
            for (size_t i = 0; i < NUM_ITER; i++) {
                for (size_t j = 0; j < testCase.size(); j++) {
                    iov[j].iov_base = malloc(testCase[j]);
                    iov[j].iov_len = testCase[j];

#if RANDOMIZE_DATA
                    for (size_t k = 0; k < testCase[j]; k++)
                        ((char *)iov[j].iov_base)[k] = dist(rng);
#endif
                }

                Clock clock;
                SSL_writev(ssl, iov, testCase.size(), &written);
                writevTimes.push_back(clock.GetElapsedNanoseconds());

                for (size_t j = 0; j < testCase.size(); j++)
                    free(iov[j].iov_base);

#if FLUSH_RECORD
                SSL_read(ssl, buf, 1);
#endif
            }
            SaveStats("SSL_writev() " + name, writevTimes);
        }

        free(iov);
    }

    std::vector<std::string> toPrint;
    for (auto &name : testNames) {
        toPrint.push_back("SSL_write() " + name);
#if BENCH_WRITE_MEMCPY
        toPrint.push_back("SSL_write() with memcpy " + name);
#endif
        toPrint.push_back("SSL_writev() " + name);
    }
    PrintStats(toPrint);

#ifdef STATS_FILE
    SaveStatsToCSV(toPrint, STATS_FILE);
#endif

    SSL_free(ssl);
    close(sfd);
    SSL_CTX_free(ctx);
}

void BenchSingleWrite() {
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

        for (size_t j = 0; j < testCase.size(); j++) {
            iov[j].iov_base = calloc(testCase[j], sizeof(char));
            iov[j].iov_len = testCase[j];
        }

        for (size_t j = 0; j < testCase.size(); j++)
            SSL_write(ssl, iov[j].iov_base, iov[j].iov_len);

        for (size_t j = 0; j < testCase.size(); j++)
            free(iov[j].iov_base);

        size_t written;
        for (size_t j = 0; j < testCase.size(); j++) {
            iov[j].iov_base = calloc(testCase[j], sizeof(char));
            iov[j].iov_len = testCase[j];
        }

        SSL_writev(ssl, iov, testCase.size(), &written);

        for (size_t j = 0; j < testCase.size(); j++)
            free(iov[j].iov_base);

        free(iov);
    }

    SSL_free(ssl);
    close(sfd);
    SSL_CTX_free(ctx);
}
} // namespace Client
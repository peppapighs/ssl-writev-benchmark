#include "Client.h"

int main() {
#ifndef BENCH_SINGLE_WRITE
    Client::BenchWriteSpeed();
#else
    Client::BenchSingleWrite();
#endif
}
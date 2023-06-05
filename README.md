# ssl-writev-benchmark

Benchmark of SSL_writev() implementation

## Prerequisites

- [OpenSSL with `SSL_writev()` implementation](https://github.com/peppapighs/openssl/tree/writev)
- [Boost](https://www.boost.org/)
- [fmt](https://github.com/fmtlib/fmt)
- C++20 Compiler
- CMake

## Usage

1. Build the project. Make sure that OpenSSL is installed in `/usr/local/lib64/`.

   ```bash
   ./make.sh
   ```

1. Run SSL server.

   ```bash
   build/Server
   ```

1. In another terminal, run the benchmark.

   ```bash
   build/Client
   ```

## Result

### Environment

- OS: Ubuntu 22.04.2 LTS x86 | 64
- Kernel: 5.15.0-72-generic
- CPU: Intel Xeon Gold 5218 (64) @ 3.900GHz
- GPU: NVIDIA Quadro T1000 Mobile
- Memory: 128603MiB
- Compiler: g++ (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0
- Boost: 1.76.0

### Execution Time Comparison (nanoseconds)

The results belows are the execution time of `SSL_write()` and `SSL_writev()` API calls in nanoseconds. For each test case, the benchmark is run with 200000 iterations and the first 10000 iterations are discarded to avoid the effect of cache warmup. The goal of this benchmark is to show that `SSL_writev()` is more efficient at sending multiple buffers at once than calling `SSL_write()` multiple times.

The benchmark uses TLSv1.2, ECDHE-RSA-AES256-GCM-SHA384 encryption and the maximum record size of 16384 bytes. The test case name is in the format of `SSL_write() [buffer_sizes]` and `SSL_writev() [buffer_sizes]`, where `buffer_sizes` are the sizes of the buffers to be sent. For example, `SSL_write() [50 + 300 + 100]` means that the benchmark sends 3 buffers with sizes 50, 300, and 100 bytes respectively using `SSL_write()`.

The statistics provided are the minimum, 50th percentile, 90th percentile, 99th percentile, and 99.9th percentile of the execution time in nanoseconds.

| Value                             | min  | 50_pct | 90_pct | 99_pct | 999_pct |
| --------------------------------- | ---- | ------ | ------ | ------ | ------- |
| SSL_write() [50]                  | 1284 | 1407   | 1543   | 2004   | 12032   |
| SSL_writev() [50]                 | 1263 | 1404   | 1681   | 3388   | 11978   |
| SSL_write() [300]                 | 1419 | 1616   | 3722   | 6646   | 13914   |
| SSL_writev() [300]                | 1397 | 1569   | 3529   | 7326   | 13969   |
| SSL_write() [5000]                | 2634 | 2926   | 5777   | 8732   | 20585   |
| SSL_writev() [5000]               | 2614 | 2929   | 5795   | 10116  | 20079   |
| SSL_write() [100 + 50]            | 2612 | 2851   | 3087   | 7011   | 18766   |
| SSL_writev() [100 + 50]           | 1309 | 2682   | 3011   | 6532   | 14963   |
| SSL_write() [50 + 300]            | 2670 | 3035   | 4298   | 7244   | 18829   |
| SSL_writev() [50 + 300]           | 1382 | 2614   | 3864   | 5832   | 15655   |
| SSL_write() [2000 + 1000]         | 3404 | 3722   | 6111   | 8060   | 22736   |
| SSL_writev() [2000 + 1000]        | 2063 | 3500   | 4998   | 7608   | 21436   |
| SSL_write() [100 + 50 + 100]      | 3890 | 4267   | 5439   | 8222   | 20467   |
| SSL_writev() [100 + 50 + 100]     | 1343 | 2947   | 4560   | 8051   | 17957   |
| SSL_write() [50 + 300 + 100]      | 4009 | 4440   | 6205   | 8403   | 20472   |
| SSL_writev() [50 + 300 + 100]     | 1404 | 3057   | 5428   | 8062   | 17791   |
| SSL_write() [1000 + 3000 + 2000]  | 5547 | 6123   | 8981   | 14903  | 32261   |
| SSL_writev() [1000 + 3000 + 2000] | 2920 | 5813   | 8494   | 12356  | 25373   |

The results show that `SSL_writev()` performs no worse than `SSL_write()` in all test cases. In cases with more than one buffer, `SSL_writev()` performs better than `SSL_write()`.

### Record Size Comparison

Using the same encryption and record size as above, a single TLS record takes 29 + `payload_size` bytes. Therefore, using `SSL_writev()` can save 29 \* (`num_buffers` - 1) bytes of overhead. For example, if the benchmark sends 3 buffers with sizes 50, 300, and 100 bytes respectively, the total overhead is 29 \* (3 - 1) = 58 bytes.

This result is observed through running `build/Client_SingleWrite` (sending a single iteration of each test case) and using Wireshark to capture the packets.

The decrease in overhead can allow for a higher throughput in the network.

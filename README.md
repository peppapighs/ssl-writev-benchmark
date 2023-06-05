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

The benchmark uses TLSv1.2, ECDHE-RSA-AES256-GCM-SHA384 encryption and the maximum record size of 16384 bytes. The test case name is in the format of `SSL_write() [buffer_sizes]` and `SSL_writev() [buffer_sizes]`, where `buffer_sizes` are the sizes of the buffers to be sent. For example, `SSL_write() [50, 300, 100]` means that the benchmark sends 3 buffers with sizes 50, 300, and 100 bytes respectively using `SSL_write()` and `SSL_writev() [50] * 20` means that the benchmark sends 20 buffers with size 50 bytes using `SSL_writev()`.

The statistics provided are the minimum, 50th percentile, 90th percentile, 99th percentile, and 99.9th percentile of the execution time in nanoseconds.

| Value                           | min   | 50_pct | 90_pct | 99_pct | 999_pct |
| ------------------------------- | ----- | ------ | ------ | ------ | ------- |
| SSL_write() [50]                | 1285  | 1402   | 1513   | 1945   | 12202   |
| SSL_writev() [50]               | 1278  | 1408   | 1704   | 2638   | 12135   |
| SSL_write() [300]               | 1351  | 1575   | 2335   | 4731   | 13365   |
| SSL_writev() [300]              | 1341  | 1541   | 2009   | 4472   | 12338   |
| SSL_write() [5000]              | 2648  | 3007   | 6050   | 11703  | 20301   |
| SSL_writev() [5000]             | 2637  | 3015   | 6063   | 11847  | 20859   |
| SSL_write() [100, 50]           | 2638  | 3004   | 3622   | 7618   | 18980   |
| SSL_writev() [100, 50]          | 1319  | 2700   | 3415   | 7010   | 17335   |
| SSL_write() [50, 300]           | 2670  | 3091   | 4313   | 7529   | 18841   |
| SSL_writev() [50, 300]          | 1375  | 2722   | 3973   | 6639   | 16871   |
| SSL_write() [2000, 1000]        | 3380  | 3813   | 6376   | 9026   | 23574   |
| SSL_writev() [2000, 1000]       | 2076  | 3539   | 5091   | 8623   | 21829   |
| SSL_write() [100, 50, 100]      | 3951  | 4304   | 5355   | 9655   | 20165   |
| SSL_writev() [100, 50, 100]     | 1348  | 2716   | 4556   | 8339   | 16691   |
| SSL_write() [50, 300, 100]      | 4049  | 4531   | 6411   | 8910   | 20441   |
| SSL_writev() [50, 300, 100]     | 1409  | 2777   | 5486   | 8447   | 17860   |
| SSL_write() [1000, 3000, 2000]  | 5551  | 6147   | 9199   | 14693  | 33447   |
| SSL_writev() [1000, 3000, 2000] | 2917  | 5908   | 8976   | 13428  | 26460   |
| SSL_write() [50] \* 20          | 26217 | 28720  | 39371  | 66174  | 96101   |
| SSL_writev() [50] \* 20         | 1590  | 4592   | 32289  | 53831  | 87398   |

The results show that `SSL_writev()` performs no worse than `SSL_write()` in all test cases. In cases with more than one buffer, `SSL_writev()` performs better than `SSL_write()`.

### Record Size Comparison

Using the same encryption and record size as above, a single TLS record takes 29 + `payload_size` bytes. Therefore, using `SSL_writev()` can save 29 \* (`num_buffers` - 1) bytes of overhead. For example, if the benchmark sends 3 buffers with sizes 50, 300, and 100 bytes respectively, the total overhead is 29 \* (3 - 1) = 58 bytes.

This result is observed through running `build/Client_SingleWrite` (sending a single iteration of each test case) and using Wireshark to capture the packets.

The decrease in overhead can allow for a higher throughput in the network.

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

The results belows are the execution time of `SSL_write()` and `SSL_writev()` API calls in nanoseconds. For each test case, the benchmark is run with 200000 iterations and the first 10000 iterations are discarded to avoid the effect of cache warmup. The goal of this benchmark is to show that `SSL_writev()` is more efficient at sending multiple buffers at once than calling `SSL_write()` multiple times. The results also include `SSL_write()` with `memcpy`, where the buffers are copied into a single buffer before calling `SSL_write()`. This method will serve as a baseline for the performance of `SSL_writev()`.

The benchmark uses TLSv1.2, ECDHE-RSA-AES256-GCM-SHA384 encryption and the maximum record size of 16384 bytes. The test case name is in the format of `SSL_write() [buffer_sizes]` and `SSL_writev() [buffer_sizes]`, where `buffer_sizes` are the sizes of the buffers to be sent. For example, `SSL_write() [50, 300, 100]` means that the benchmark sends 3 buffers with sizes 50, 300, and 100 bytes respectively using `SSL_write()` and `SSL_writev() [50] * 20` means that the benchmark sends 20 buffers with size 50 bytes using `SSL_writev()`.

The statistics provided are the minimum, 50th percentile, 90th percentile, 99th percentile, and 99.9th percentile of the execution time in nanoseconds.

| Value                                      | min   | 50_pct | 90_pct | 99_pct | 999_pct |
| ------------------------------------------ | ----- | ------ | ------ | ------ | ------- |
| SSL_write() [50]                           | 1248  | 1357   | 1455   | 1884   | 12025   |
| SSL_write() with memcpy [50]               | 1264  | 1391   | 1496   | 1914   | 12020   |
| SSL_writev() [50]                          | 1247  | 1371   | 1473   | 5170   | 12013   |
| SSL_write() [300]                          | 1342  | 1489   | 1568   | 5627   | 12806   |
| SSL_write() with memcpy [300]              | 1367  | 1525   | 1602   | 4739   | 12307   |
| SSL_writev() [300]                         | 1340  | 1490   | 1561   | 4704   | 12264   |
| SSL_write() [5000]                         | 2758  | 3157   | 7706   | 15370  | 23741   |
| SSL_write() with memcpy [5000]             | 2800  | 3190   | 7596   | 15325  | 23181   |
| SSL_writev() [5000]                        | 2648  | 3121   | 8550   | 15521  | 23948   |
| SSL_write() [100, 50]                      | 2594  | 3036   | 3531   | 8738   | 18838   |
| SSL_write() with memcpy [100, 50]          | 1319  | 1484   | 1629   | 4338   | 13857   |
| SSL_writev() [100, 50]                     | 1304  | 1444   | 1542   | 4434   | 13644   |
| SSL_write() [50, 300]                      | 2634  | 2943   | 3127   | 8878   | 18734   |
| SSL_write() with memcpy [50, 300]          | 1400  | 1555   | 1837   | 6064   | 12386   |
| SSL_writev() [50, 300]                     | 1389  | 1574   | 1882   | 7226   | 13914   |
| SSL_write() [2000, 1000]                   | 3417  | 3855   | 7884   | 14442  | 23226   |
| SSL_write() with memcpy [2000, 1000]       | 2199  | 2499   | 6394   | 12595  | 21469   |
| SSL_writev() [2000, 1000]                  | 2110  | 2385   | 6193   | 11472  | 18691   |
| SSL_write() [100, 50, 100]                 | 3895  | 4313   | 4984   | 10886  | 20370   |
| SSL_write() with memcpy [100, 50, 100]     | 1364  | 1523   | 1738   | 5493   | 13971   |
| SSL_writev() [100, 50, 100]                | 1350  | 1493   | 1583   | 5044   | 12310   |
| SSL_write() [50, 300, 100]                 | 3959  | 4374   | 5060   | 11654  | 20417   |
| SSL_write() with memcpy [50, 300, 100]     | 1421  | 1593   | 1870   | 7849   | 14908   |
| SSL_writev() [50, 300, 100]                | 1398  | 1554   | 1816   | 7682   | 17181   |
| SSL_write() [1000, 3000, 2000]             | 5534  | 6375   | 13266  | 20364  | 29773   |
| SSL_write() with memcpy [1000, 3000, 2000] | 3146  | 3609   | 10071  | 16255  | 25009   |
| SSL_writev() [1000, 3000, 2000]            | 2953  | 3395   | 9606   | 15551  | 23541   |
| SSL_write() [50] \* 20                     | 25586 | 27896  | 33228  | 44418  | 57014   |
| SSL_write() with memcpy [50] \* 20         | 1630  | 1814   | 2172   | 9312   | 17471   |
| SSL_writev() [50] \* 20                    | 1581  | 1768   | 2111   | 9241   | 17366   |

The results show that `SSL_writev()` performs no worse than `SSL_write()` and `SSL_write()` with `memcpy` in all test cases. In cases with more than one buffer, `SSL_writev()` performs better than `SSL_write()`.

### Record Size Comparison

Using the same encryption and record size as above, a single TLS record takes 29 + `payload_size` bytes. Therefore, using `SSL_writev()` can save 29 \* (`num_buffers` - 1) bytes of overhead. For example, if the benchmark sends 3 buffers with sizes 50, 300, and 100 bytes respectively, the total overhead is 29 \* (3 - 1) = 58 bytes.

This result is observed through running `build/Client_SingleWrite` (sending a single iteration of each test case) and using Wireshark to capture the packets.

The decrease in overhead can allow for a higher goodput in the network.

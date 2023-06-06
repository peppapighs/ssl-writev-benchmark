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

- OS: Ubuntu 22.04.2 LTS x86_64
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
| SSL_write() [50]                           | 1259  | 1384   | 1480   | 1878   | 12119   |
| SSL_write() with memcpy [50]               | 1262  | 1383   | 1471   | 1925   | 12123   |
| SSL_writev() [50]                          | 1250  | 1377   | 1444   | 1549   | 12100   |
| SSL_write() [300]                          | 1351  | 1497   | 1587   | 5147   | 12142   |
| SSL_write() with memcpy [300]              | 1368  | 1519   | 1634   | 4799   | 12343   |
| SSL_writev() [300]                         | 1423  | 1525   | 1670   | 4983   | 9799    |
| SSL_write() [5000]                         | 2781  | 3202   | 8976   | 15749  | 23716   |
| SSL_write() with memcpy [5000]             | 2811  | 3110   | 7192   | 14709  | 21307   |
| SSL_writev() [5000]                        | 2671  | 2968   | 7194   | 14724  | 22226   |
| SSL_write() [100, 50]                      | 2580  | 2759   | 2924   | 5609   | 14178   |
| SSL_write() with memcpy [100, 50]          | 1309  | 1405   | 1488   | 3162   | 9470    |
| SSL_writev() [100, 50]                     | 1317  | 1418   | 1502   | 2393   | 10063   |
| SSL_write() [50, 300]                      | 2663  | 2915   | 3072   | 8541   | 18512   |
| SSL_write() with memcpy [50, 300]          | 1398  | 1529   | 1823   | 5284   | 13457   |
| SSL_writev() [50, 300]                     | 1378  | 1521   | 1629   | 5232   | 12816   |
| SSL_write() [2000, 1000]                   | 3403  | 3824   | 7850   | 13636  | 22733   |
| SSL_write() with memcpy [2000, 1000]       | 2186  | 2442   | 6270   | 10938  | 18814   |
| SSL_writev() [2000, 1000]                  | 2090  | 2365   | 6226   | 12601  | 20958   |
| SSL_write() [100, 50, 100]                 | 3856  | 4275   | 5153   | 10477  | 21076   |
| SSL_write() with memcpy [100, 50, 100]     | 1353  | 1520   | 1680   | 5732   | 12395   |
| SSL_writev() [100, 50, 100]                | 1342  | 1486   | 1722   | 5837   | 12951   |
| SSL_write() [50, 300, 100]                 | 3969  | 4355   | 5255   | 11623  | 21024   |
| SSL_write() with memcpy [50, 300, 100]     | 1413  | 1544   | 1675   | 6261   | 12682   |
| SSL_writev() [50, 300, 100]                | 1408  | 1517   | 1623   | 5186   | 12472   |
| SSL_write() [1000, 3000, 2000]             | 5581  | 6191   | 12834  | 19052  | 27172   |
| SSL_write() with memcpy [1000, 3000, 2000] | 3113  | 3487   | 9081   | 15186  | 22287   |
| SSL_writev() [1000, 3000, 2000]            | 2943  | 3374   | 9852   | 15203  | 22705   |
| SSL_write() [50] \* 20                     | 25516 | 28335  | 36216  | 47039  | 59300   |
| SSL_write() with memcpy [50] \* 20         | 1612  | 1772   | 2132   | 8971   | 15426   |
| SSL_writev() [50] \* 20                    | 1590  | 1769   | 2242   | 9173   | 17273   |

The results show that `SSL_writev()` performs no worse than `SSL_write()` and `SSL_write()` with `memcpy` in all test cases. In cases with more than one buffer, `SSL_writev()` performs better than `SSL_write()`.

### Record Size Comparison

Using the same encryption and record size as above, a single TLS record takes 29 + `payload_size` bytes. Therefore, using `SSL_writev()` can save 29 \* (`num_buffers` - 1) bytes of overhead. For example, if the benchmark sends 3 buffers with sizes 50, 300, and 100 bytes respectively, the total overhead is 29 \* (3 - 1) = 58 bytes.

This result is observed through running `build/Client_SingleWrite` (sending a single iteration of each test case) and using Wireshark to capture the packets.

The decrease in overhead can allow for a higher goodput in the network.

#pragma once

#include <map>
#include <string>
#include <vector>

/*
 * This file contains the configuration parameters for the application.
 */

// Port to listen on.
#define PORT 4433

// Number of iterations for each test case.
#define NUM_ITER 200000

// Randomize the data sent to the server.
#define RANDOMIZE_DATA 1

// Include benchmark result of combining buffer and calling SSL_write().
#define BENCH_WRITE_MEMCPY 1

// Flush the record after each write (take a lot longer to run if enabled).
#define FLUSH_RECORD 0

// Test Cases
const std::vector<std::string> testNames = {
    "[50]",           "[300]",          "[5000]",
    "[100, 50]",      "[50, 300]",      "[2000, 1000]",
    "[100, 50, 100]", "[50, 300, 100]", "[1000, 3000, 2000]",
    "[50] * 20",
};
const std::map<std::string, std::vector<size_t>> testCases = {
    {"[50]", {50}},
    {"[300]", {300}},
    {"[5000]", {5000}},
    {"[100, 50]", {100, 50}},
    {"[50, 300]", {50, 300}},
    {"[2000, 1000]", {2000, 1000}},
    {"[100, 50, 100]", {100, 50, 100}},
    {"[50, 300, 100]", {50, 300, 100}},
    {"[1000, 3000, 2000]", {1000, 3000, 2000}},
    {"[50] * 20", {50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
                   50, 50, 50, 50, 50, 50, 50, 50, 50, 50}},
};
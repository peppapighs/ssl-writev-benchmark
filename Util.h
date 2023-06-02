#pragma once

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include <fmt/core.h>

#include <array>
#include <iostream>
#include <map>
#include <vector>

using namespace boost::accumulators;

constexpr size_t numDiscard = 10000;
constexpr size_t colWidth = 12;

std::array<std::string, 5> statNames = {"min", "50_pct", "90_pct", "99_pct",
                                        "999_pct"};

std::vector<std::string> names;
std::vector<std::vector<double>> results;

int64_t GetPercentile(std::vector<int64_t> &data, double Percentile) {
    size_t rank = Percentile * data.size();
    std::nth_element(data.begin(), data.begin() + rank, data.end());
    return data[rank];
}

void SaveStats(const std::string &name, std::vector<int64_t> &times) {
    times = std::vector<int64_t>(times.begin() + numDiscard, times.end());

    accumulator_set<double, stats<tag::count, tag::mean, tag::variance,
                                  tag::min, tag::max, tag::median>>
        stats;
    std::map<std::string, std::function<double()>> statFuncs = {
        {"count", [&stats]() { return count(stats); }},
        {"min", [&stats]() { return min(stats); }},
        {"50_pct", [&times]() { return GetPercentile(times, 0.5); }},
        {"90_pct", [&times]() { return GetPercentile(times, 0.9); }},
        {"99_pct", [&times]() { return GetPercentile(times, 0.99); }},
        {"999_pct", [&times]() { return GetPercentile(times, 0.999); }}};

    for (auto time : times)
        stats(time);

    names.emplace_back(name);
    results.emplace_back(statNames.size());

    auto &result = results.back();
    for (size_t i = 0; i < statNames.size(); i++)
        result[i] = statFuncs[statNames[i]]();
}

void PrintStats(const std::vector<std::string> &toPrint) {
    std::cout << fmt::format("| {:<{}} |", "Value", colWidth);
    for (auto &name : statNames)
        std::cout << fmt::format(" {:<{}} |", name, colWidth);
    std::cout << std::endl;

    std::cout << fmt::format("| {} |", std::string(colWidth, '-'));
    for (auto &name : statNames)
        std::cout << fmt::format(" {} |", std::string(colWidth, '-'));
    std::cout << std::endl;

    for (size_t i = 0; i < toPrint.size(); i++) {
        std::cout << fmt::format("| {:<{}} |", toPrint[i], colWidth);
        auto ind = std::distance(
            names.begin(), std::find(names.begin(), names.end(), toPrint[i]));
        for (size_t j = 0; j < statNames.size(); j++)
            std::cout << fmt::format(" {:<{}} |",
                                     std::round(results[ind][j] * 1000) / 1000,
                                     colWidth);
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

inline void ClearCache() {
    constexpr size_t size = 240 * 1024 * 1024;
    static volatile char c[size];

    for (int i = 0; i < 2; i++)
        for (int j = 0; j < size; j++)
            c[j] = i * j;
}
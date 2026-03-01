#pragma once
#include <cstdint>
#include <unordered_map>

namespace montecarlo {

    struct Result {
        std::unordered_map<int, int> counter;
        double fairValue, CI_lower, CI_upper, 
               delta, gamma, vega, avg_S_T;
        Result(double FV, double CIl, double CIu, double D, double G, double V, double avg_S_T, std::unordered_map<int, int> ctr);
    };

    enum class OptionType {Call, Put};

    Result Montecarlo(double S_0, double volatility, double K, double T, double r, const OptionType OT, const int64_t N);

    inline double simulation(double S_0, double exponential) {
        return S_0 * exponential;
    }

    inline double getPayoff(double S_T, double K, double phi) {
        return std::max(phi * (S_T - K), 0.0);
    }   

    double getConfidenceIntervals(double C, int64_t N, double SD);
}
#include "montecarlo.hpp"
#include "xoshiro256.hpp"
#include <cmath>
#include <numbers>
#include <random>
#include <omp.h>
#include <utility>

namespace montecarlo {

    Result::Result(double FV, double CIl, double CIu, double D, double G, double V, double ST, std::unordered_map<int, int> ctr) 
        : fairValue(FV), CI_lower(CIl), CI_upper(CIu), delta(D), gamma(G), vega(V), avg_S_T(ST), counter(std::move(ctr)) {}

    double getConfidenceIntervals(const double discount, const int64_t N, const double SD) {
        return 1.96 * discount * (SD / std::sqrt(N));
    }

    Result Montecarlo(const double S_0, const double volatility, const double K, const double T, const double r, const OptionType OT, int64_t N) {
        if(N % 2 != 0){
            N += 1;
        }

        std::unordered_map<int, int> global_counter;

        double sum_payoff = 0.0, sum_P_S0_up = 0.0,
               sum_P_S0_down = 0.0, sum_P_vol_up = 0.0,
               sum_P_vol_down = 0.0, sum_S_T = 0.0, 
               sum_payoff_sq = 0.0;

        const double sqrtT = std::sqrt(T);
        const double epsilon_S_0 = S_0 * 1e-4;
        const double S_0_up = S_0 + epsilon_S_0;
        const double S_0_down = S_0 - epsilon_S_0;
        
        const double epsilon_sigma = 1e-4;
        const double volatility_up = volatility + epsilon_sigma;
        const double volatility_down = volatility - epsilon_sigma;

        const double drift = (r - ( volatility * volatility / 2.0 )) * T;
        const double vol_sqrt_T = volatility * sqrtT;

        const double drift_up = (r - (volatility_up * volatility_up / 2.0)) * T;
        const double vol_sqrt_T_up = volatility_up * sqrtT;

        const double drift_down = (r - (volatility_down * volatility_down / 2.0)) * T;
        const double vol_sqrt_T_down = volatility_down * sqrtT;
        
        std::random_device rd;
        uint64_t master_seed = (static_cast<uint64_t>(rd()) << 32) | rd();
        
        const double phi = (OT == OptionType::Call) ? 1.0 : -1.0;

        #pragma omp parallel
        {
            int thread_id = omp_get_thread_num();
            std::unordered_map<int, int> local_map;
            uint64_t sm_state = master_seed + thread_id; 
            
            xoshiro256::xoshiro256starstar_state rng;
            rng.s[0] = xoshiro256::splitmix64(sm_state);
            rng.s[1] = xoshiro256::splitmix64(sm_state);
            rng.s[2] = xoshiro256::splitmix64(sm_state);
            rng.s[3] = xoshiro256::splitmix64(sm_state);

            #pragma omp for reduction(+:sum_payoff, sum_P_S0_up, sum_P_S0_down, sum_P_vol_up, sum_P_vol_down, sum_S_T, sum_payoff_sq) schedule(static)
            for (int64_t i = 0; i < N ; i += 2) {
                double u1 = 1.0 - xoshiro256::next_double(rng);
                double u2 = xoshiro256::next_double(rng);

                double R = std::sqrt(-2.0 * std::log(u1));
                double theta = 2.0 * std::numbers::pi * u2;
                
                double Z1 = R * std::cos(theta);
                double Z2 = R * std::sin(theta);

                double exp1 = std::exp(drift + vol_sqrt_T * Z1);
                double exp2 = std::exp(drift + vol_sqrt_T * Z2);
                
                double S_T1 = simulation(S_0, exp1);
                double S_T2 = simulation(S_0, exp2);

                double payoff1 = getPayoff(S_T1, K, phi);
                double payoff2 = getPayoff(S_T2, K, phi);
                
                sum_S_T += S_T1 + S_T2;
                sum_payoff += payoff1 + payoff2;
                sum_payoff_sq += payoff1 * payoff1 + payoff2 * payoff2;

                sum_P_S0_up += getPayoff(simulation(S_0_up, exp1), K, phi) + 
                               getPayoff(simulation(S_0_up, exp2), K, phi);

                sum_P_S0_down += getPayoff(simulation(S_0_down, exp1), K, phi) +
                                 getPayoff(simulation(S_0_down, exp2), K, phi);

                sum_P_vol_up += getPayoff(simulation(S_0, std::exp(drift_up + vol_sqrt_T_up * Z1)), K, phi) +
                                getPayoff(simulation(S_0, std::exp(drift_up + vol_sqrt_T_up * Z2)), K, phi);

                sum_P_vol_down += getPayoff(simulation(S_0, std::exp(drift_down + vol_sqrt_T_down * Z1)), K, phi) +
                                  getPayoff(simulation(S_0, std::exp(drift_down + vol_sqrt_T_down * Z2)), K, phi);

                int bin1 = static_cast<int>(std::round(S_T1 * 100)); 
                int bin2 = static_cast<int>(std::round(S_T2 * 100));

                local_map[bin1]++;
                local_map[bin2]++;
            }

            #pragma omp critical
            {
                for(const auto& pair : local_map){
                    global_counter[pair.first] += pair.second;
                }
            }
        }
        
        const double avg_S_T = sum_S_T / N;
        const double avg_payoff = sum_payoff / N;
        const double avg_P_S0_up = sum_P_S0_up / N;
        const double avg_P_S0_down = sum_P_S0_down / N;
        const double avg_P_vol_up = sum_P_vol_up / N;
        const double avg_P_vol_down = sum_P_vol_down / N;

        const double var = (sum_payoff_sq - (sum_payoff * sum_payoff / N)) / (N - 1);
        
        const double discount = std::exp(-r * T);
        const double fairValue = discount * avg_payoff;
        const double confidenceIntervals = getConfidenceIntervals(discount, N, std::sqrt(var));

        const double delta = discount * (avg_P_S0_up - avg_P_S0_down) / (2.0 * epsilon_S_0);
        const double gamma = discount * (avg_P_S0_up - (2.0 * avg_payoff) + avg_P_S0_down) / (epsilon_S_0 * epsilon_S_0);
        const double vega  = discount * (avg_P_vol_up - avg_P_vol_down) / (2.0 * epsilon_sigma);
        
        return Result(fairValue, fairValue - confidenceIntervals, fairValue + confidenceIntervals, delta, gamma, vega, avg_S_T, global_counter);
    }
}
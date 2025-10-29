#pragma once

#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>

// Forward declaration of BenchmarkResult structure
template <typename T>
struct BenchmarkResult {
    std::string algorithm_name;
    std::string dataset_name;
    std::map<std::string, std::string> parameters;
    
    double mean_approximation_factor;
    double recall_at_n;
    double queries_per_second;
    double mean_query_time_ms;
    double brute_force_time_ms;
    double speedup_vs_brute_force;
    
    size_t dataset_size;
    size_t query_count;
    
    BenchmarkResult() : mean_approximation_factor(0), recall_at_n(0),
                       queries_per_second(0), mean_query_time_ms(0),
                       brute_force_time_ms(0), speedup_vs_brute_force(0),
                       dataset_size(0), query_count(0) {}
};

template <typename T>
class ResultsVisualizer {
private:
    std::vector<BenchmarkResult<T>> results;
    std::string output_directory;
    
    // Helper methods for plotting
    void write_gnuplot_script(const std::string& script_file, const std::string& data_file,
                             const std::string& output_file, const std::string& title,
                             const std::string& xlabel, const std::string& ylabel,
                             const std::vector<std::string>& algorithms) const;
    
    void write_python_script(const std::string& script_file, const std::string& data_file,
                            const std::string& output_file, const std::string& title,
                            const std::string& xlabel, const std::string& ylabel,
                            const std::vector<std::string>& algorithms) const;
    
    std::vector<std::string> get_unique_algorithms() const;
    
public:
    ResultsVisualizer(const std::string& output_dir = "visualization/plots") 
        : output_directory(output_dir) {}
    
    // Set results data
    void set_results(const std::vector<BenchmarkResult<T>>& benchmark_results) {
        results = benchmark_results;
    }
    
    // Parameter impact analysis
    void plot_parameter_impact(const std::string& algorithm_name, 
                              const std::string& parameter_name,
                              const std::string& metric_name) const;
    
    // Algorithm comparison plots
    void plot_algorithm_comparison_bar(const std::string& metric_name) const;
    void plot_algorithm_comparison_scatter(const std::string& x_metric, 
                                          const std::string& y_metric) const;
    
    // Performance vs parameter plots
    void plot_performance_vs_parameter(const std::string& parameter_name,
                                      const std::string& performance_metric) const;
    
    // Memory usage analysis
    void plot_memory_usage_comparison() const;
    
    // Time analysis
    void plot_time_comparison() const;
    void plot_speedup_analysis() const;
    
    // Recall vs approximation factor trade-off
    void plot_recall_vs_approximation_factor() const;
    
    // Generate comprehensive report
    void generate_full_report() const;
    
    // Export data for external plotting tools
    void export_for_matplotlib(const std::string& filename) const;
    void export_for_gnuplot(const std::string& filename) const;
    
    // Create HTML report with embedded plots
    void create_html_report(const std::string& filename) const;
};
#include "visualizer.h"
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <cstdlib>

template <typename T>
std::vector<std::string> ResultsVisualizer<T>::get_unique_algorithms() const {
    std::vector<std::string> algorithms;
    for (const auto& result : results) {
        if (std::find(algorithms.begin(), algorithms.end(), result.algorithm_name) == algorithms.end()) {
            algorithms.push_back(result.algorithm_name);
        }
    }
    return algorithms;
}

template <typename T>
void ResultsVisualizer<T>::write_python_script(const std::string& script_file, 
                                              const std::string& data_file,
                                              const std::string& output_file, 
                                              const std::string& title,
                                              const std::string& xlabel, 
                                              const std::string& ylabel,
                                              const std::vector<std::string>& algorithms) const {
    std::ofstream script(script_file);
    if (!script.is_open()) {
        std::cerr << "Error: Cannot create Python script file " << script_file << std::endl;
        return;
    }
    
    script << "#!/usr/bin/env python3\n";
    script << "import pandas as pd\n";
    script << "import matplotlib.pyplot as plt\n";
    script << "import seaborn as sns\n";
    script << "import numpy as np\n\n";
    
    script << "# Set style\n";
    script << "plt.style.use('default')\n";
    script << "sns.set_palette('husl')\n";
    script << "plt.figure(figsize=(12, 8))\n\n";
    
    script << "# Load data\n";
    script << "data = pd.read_csv('" << data_file << "')\n\n";
    
    script << "# Create plot\n";
    script << "plt.figure(figsize=(12, 8))\n";
    script << "for algorithm in data['Algorithm'].unique():\n";
    script << "    subset = data[data['Algorithm'] == algorithm]\n";
    script << "    plt.plot(subset['" << xlabel << "'], subset['" << ylabel << "'], \n";
    script << "             marker='o', linewidth=2, markersize=8, label=algorithm)\n\n";
    
    script << "plt.xlabel('" << xlabel << "', fontsize=14)\n";
    script << "plt.ylabel('" << ylabel << "', fontsize=14)\n";
    script << "plt.title('" << title << "', fontsize=16, fontweight='bold')\n";
    script << "plt.legend(fontsize=12)\n";
    script << "plt.grid(True, alpha=0.3)\n";
    script << "plt.tight_layout()\n";
    script << "plt.savefig('" << output_file << "', dpi=300, bbox_inches='tight')\n";
    script << "plt.close()\n";
    script << "print('Plot saved to " << output_file << "')\n";
    
    script.close();
}

template <typename T>
void ResultsVisualizer<T>::plot_parameter_impact(const std::string& algorithm_name,
                                                const std::string& parameter_name,
                                                const std::string& metric_name) const {
    // Create output directory
    std::filesystem::create_directories(output_directory);
    
    std::string data_file = output_directory + "/param_impact_" + algorithm_name + "_" + parameter_name + ".csv";
    std::string plot_file = output_directory + "/param_impact_" + algorithm_name + "_" + parameter_name + ".png";
    std::string script_file = output_directory + "/plot_param_impact.py";
    
    // Filter results for specific algorithm
    std::vector<BenchmarkResult<T>> filtered_results;
    for (const auto& result : results) {
        if (result.algorithm_name == algorithm_name) {
            filtered_results.push_back(result);
        }
    }
    
    if (filtered_results.empty()) {
        std::cerr << "No results found for algorithm: " << algorithm_name << std::endl;
        return;
    }
    
    // Write data file
    std::ofstream data(data_file);
    data << "Parameter," << metric_name << "\n";
    
    for (const auto& result : filtered_results) {
        auto param_it = result.parameters.find(parameter_name);
        if (param_it != result.parameters.end()) {
            data << param_it->second << ",";
            
            if (metric_name == "AF") {
                data << result.average_approximation_factor;
            } else if (metric_name == "Recall") {
                data << result.recall_at_n;
            } else if (metric_name == "QPS") {
                data << result.queries_per_second;
            } else if (metric_name == "Time") {
                data << result.average_approximate_time_ms;
            }
            data << "\n";
        }
    }
    data.close();
    
    // Create Python plotting script
    std::ofstream script(script_file);
    script << "#!/usr/bin/env python3\n";
    script << "import pandas as pd\n";
    script << "import matplotlib.pyplot as plt\n";
    script << "import seaborn as sns\n\n";
    
    script << "# Load data\n";
    script << "data = pd.read_csv('" << data_file << "')\n";
    script << "data = data.sort_values('Parameter')\n\n";
    
    script << "# Create plot\n";
    script << "plt.figure(figsize=(10, 6))\n";
    script << "plt.plot(data['Parameter'], data['" << metric_name << "'], \n";
    script << "         marker='o', linewidth=2, markersize=8, color='steelblue')\n";
    script << "plt.xlabel('" << parameter_name << "', fontsize=12)\n";
    script << "plt.ylabel('" << metric_name << "', fontsize=12)\n";
    script << "plt.title('" << algorithm_name << " - " << metric_name << " vs " << parameter_name << "', fontsize=14, fontweight='bold')\n";
    script << "plt.grid(True, alpha=0.3)\n";
    script << "plt.tight_layout()\n";
    script << "plt.savefig('" << plot_file << "', dpi=300, bbox_inches='tight')\n";
    script << "plt.close()\n";
    script << "print('Parameter impact plot saved to " << plot_file << "')\n";
    
    script.close();
    
    // Execute Python script
    std::string command = "python3 " + script_file;
    int result_code = std::system(command.c_str());
    if (result_code != 0) {
        std::cerr << "Warning: Failed to execute Python plotting script" << std::endl;
    }
}

template <typename T>
void ResultsVisualizer<T>::plot_algorithm_comparison_bar(const std::string& metric_name) const {
    std::filesystem::create_directories(output_directory);
    
    std::string data_file = output_directory + "/algorithm_comparison_" + metric_name + ".csv";
    std::string plot_file = output_directory + "/algorithm_comparison_" + metric_name + ".png";
    std::string script_file = output_directory + "/plot_algorithm_comparison.py";
    
    // Group results by algorithm and compute averages
    std::map<std::string, std::vector<T>> algorithm_metrics;
    
    for (const auto& result : results) {
        T metric_value = 0;
        if (metric_name == "AF") {
            metric_value = result.average_approximation_factor;
        } else if (metric_name == "Recall") {
            metric_value = result.recall_at_n;
        } else if (metric_name == "QPS") {
            metric_value = result.queries_per_second;
        } else if (metric_name == "Time") {
            metric_value = result.average_approximate_time_ms;
        }
        
        algorithm_metrics[result.algorithm_name].push_back(metric_value);
    }
    
    // Write data file
    std::ofstream data(data_file);
    data << "Algorithm," << metric_name << ",StdDev\n";
    
    for (const auto& pair : algorithm_metrics) {
        const auto& values = pair.second;
        T mean = std::accumulate(values.begin(), values.end(), T(0)) / values.size();
        
        T variance = 0;
        for (T value : values) {
            variance += (value - mean) * (value - mean);
        }
        T stddev = std::sqrt(variance / values.size());
        
        data << pair.first << "," << mean << "," << stddev << "\n";
    }
    data.close();
    
    // Create Python plotting script
    std::ofstream script(script_file);
    script << "#!/usr/bin/env python3\n";
    script << "import pandas as pd\n";
    script << "import matplotlib.pyplot as plt\n";
    script << "import seaborn as sns\n\n";
    
    script << "# Load data\n";
    script << "data = pd.read_csv('" << data_file << "')\n\n";
    
    script << "# Create bar plot\n";
    script << "plt.figure(figsize=(10, 6))\n";
    script << "bars = plt.bar(data['Algorithm'], data['" << metric_name << "'], \n";
    script << "               yerr=data['StdDev'], capsize=5, alpha=0.8)\n";
    script << "plt.xlabel('Algorithm', fontsize=12)\n";
    script << "plt.ylabel('" << metric_name << "', fontsize=12)\n";
    script << "plt.title('Algorithm Comparison - " << metric_name << "', fontsize=14, fontweight='bold')\n";
    script << "plt.xticks(rotation=45)\n";
    script << "plt.grid(True, alpha=0.3, axis='y')\n";
    script << "plt.tight_layout()\n";
    script << "plt.savefig('" << plot_file << "', dpi=300, bbox_inches='tight')\n";
    script << "plt.close()\n";
    script << "print('Algorithm comparison plot saved to " << plot_file << "')\n";
    
    script.close();
    
    // Execute Python script
    std::string command = "python3 " + script_file;
    int result_code = std::system(command.c_str());
    if (result_code != 0) {
        std::cerr << "Warning: Failed to execute Python plotting script" << std::endl;
    }
}

template <typename T>
void ResultsVisualizer<T>::plot_recall_vs_approximation_factor() const {
    std::filesystem::create_directories(output_directory);
    
    std::string data_file = output_directory + "/recall_vs_af.csv";
    std::string plot_file = output_directory + "/recall_vs_af.png";
    std::string script_file = output_directory + "/plot_recall_vs_af.py";
    
    // Write data file
    std::ofstream data(data_file);
    data << "Algorithm,Recall,AF\n";
    
    for (const auto& result : results) {
        data << result.algorithm_name << "," 
             << result.recall_at_n << "," 
             << result.average_approximation_factor << "\n";
    }
    data.close();
    
    // Create Python plotting script
    std::ofstream script(script_file);
    script << "#!/usr/bin/env python3\n";
    script << "import pandas as pd\n";
    script << "import matplotlib.pyplot as plt\n";
    script << "import seaborn as sns\n\n";
    
    script << "# Load data\n";
    script << "data = pd.read_csv('" << data_file << "')\n\n";
    
    script << "# Create scatter plot\n";
    script << "plt.figure(figsize=(10, 8))\n";
    script << "for algorithm in data['Algorithm'].unique():\n";
    script << "    subset = data[data['Algorithm'] == algorithm]\n";
    script << "    plt.scatter(subset['AF'], subset['Recall'], \n";
    script << "               s=100, alpha=0.7, label=algorithm)\n\n";
    
    script << "plt.xlabel('Approximation Factor', fontsize=12)\n";
    script << "plt.ylabel('Recall@N', fontsize=12)\n";
    script << "plt.title('Recall vs Approximation Factor Trade-off', fontsize=14, fontweight='bold')\n";
    script << "plt.legend(fontsize=10)\n";
    script << "plt.grid(True, alpha=0.3)\n";
    script << "plt.tight_layout()\n";
    script << "plt.savefig('" << plot_file << "', dpi=300, bbox_inches='tight')\n";
    script << "plt.close()\n";
    script << "print('Recall vs AF plot saved to " << plot_file << "')\n";
    
    script.close();
    
    // Execute Python script
    std::string command = "python3 " + script_file;
    int result_code = std::system(command.c_str());
    if (result_code != 0) {
        std::cerr << "Warning: Failed to execute Python plotting script" << std::endl;
    }
}

template <typename T>
void ResultsVisualizer<T>::generate_full_report() const {
    std::cout << "Generating comprehensive visualization report..." << std::endl;
    
    // Generate all standard plots
    plot_algorithm_comparison_bar("AF");
    plot_algorithm_comparison_bar("Recall");
    plot_algorithm_comparison_bar("QPS");
    plot_algorithm_comparison_bar("Time");
    plot_recall_vs_approximation_factor();
    
    // Generate parameter impact plots for each algorithm
    auto algorithms = get_unique_algorithms();
    for (const auto& algorithm : algorithms) {
        if (algorithm == "LSH") {
            plot_parameter_impact("LSH", "k", "AF");
            plot_parameter_impact("LSH", "L", "Recall");
            plot_parameter_impact("LSH", "w", "QPS");
        } else if (algorithm == "Hypercube") {
            plot_parameter_impact("Hypercube", "k", "AF");
            plot_parameter_impact("Hypercube", "probes", "Recall");
            plot_parameter_impact("Hypercube", "M", "QPS");
        } else if (algorithm == "IVFFlat") {
            plot_parameter_impact("IVFFlat", "n_clusters", "AF");
            plot_parameter_impact("IVFFlat", "n_probe", "Recall");
        } else if (algorithm == "IVFPQ") {
            plot_parameter_impact("IVFPQ", "n_clusters", "AF");
            plot_parameter_impact("IVFPQ", "n_probe", "Recall");
            plot_parameter_impact("IVFPQ", "m", "QPS");
        }
    }
    
    // Create HTML report
    create_html_report(output_directory + "/benchmark_report.html");
    
    std::cout << "Visualization report generated in: " << output_directory << std::endl;
}

template <typename T>
void ResultsVisualizer<T>::create_html_report(const std::string& filename) const {
    std::ofstream html(filename);
    if (!html.is_open()) {
        std::cerr << "Error: Cannot create HTML report file " << filename << std::endl;
        return;
    }
    
    html << "<!DOCTYPE html>\n";
    html << "<html>\n<head>\n";
    html << "<title>Vector Similarity Search - Benchmark Report</title>\n";
    html << "<style>\n";
    html << "body { font-family: Arial, sans-serif; margin: 40px; }\n";
    html << "h1 { color: #333; border-bottom: 2px solid #ddd; }\n";
    html << "h2 { color: #666; margin-top: 30px; }\n";
    html << ".summary { background: #f5f5f5; padding: 15px; border-radius: 5px; }\n";
    html << ".plot { text-align: center; margin: 20px 0; }\n";
    html << "img { max-width: 100%; height: auto; border: 1px solid #ddd; }\n";
    html << "table { border-collapse: collapse; width: 100%; }\n";
    html << "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n";
    html << "th { background-color: #f2f2f2; }\n";
    html << "</style>\n</head>\n<body>\n";
    
    html << "<h1>Vector Similarity Search Algorithms - Benchmark Report</h1>\n";
    
    html << "<div class='summary'>\n";
    html << "<h2>Summary</h2>\n";
    html << "<p>This report presents a comprehensive comparison of vector similarity search algorithms:</p>\n";
    html << "<ul>\n";
    html << "<li><strong>LSH</strong> - Locality Sensitive Hashing</li>\n";
    html << "<li><strong>Hypercube</strong> - Random Projection to Hypercube</li>\n";
    html << "<li><strong>IVFFlat</strong> - Inverted File with Flat quantization</li>\n";
    html << "<li><strong>IVFPQ</strong> - Inverted File with Product Quantization</li>\n";
    html << "</ul>\n";
    html << "</div>\n";
    
    html << "<h2>Algorithm Comparison</h2>\n";
    
    std::vector<std::string> metrics = {"AF", "Recall", "QPS", "Time"};
    std::vector<std::string> metric_names = {"Approximation Factor", "Recall@N", "Queries Per Second", "Query Time (ms)"};
    
    for (size_t i = 0; i < metrics.size(); ++i) {
        html << "<div class='plot'>\n";
        html << "<h3>" << metric_names[i] << "</h3>\n";
        html << "<img src='algorithm_comparison_" << metrics[i] << ".png' alt='" << metric_names[i] << " Comparison'>\n";
        html << "</div>\n";
    }
    
    html << "<h2>Recall vs Approximation Factor Trade-off</h2>\n";
    html << "<div class='plot'>\n";
    html << "<img src='recall_vs_af.png' alt='Recall vs AF Trade-off'>\n";
    html << "</div>\n";
    
    html << "<h2>Parameter Impact Analysis</h2>\n";
    html << "<p>The following plots show how different parameters affect algorithm performance:</p>\n";
    
    // Add parameter impact plots
    auto algorithms = get_unique_algorithms();
    for (const auto& algorithm : algorithms) {
        html << "<h3>" << algorithm << "</h3>\n";
        
        if (algorithm == "LSH") {
            html << "<div class='plot'>\n";
            html << "<img src='param_impact_LSH_k.png' alt='LSH k parameter impact'>\n";
            html << "<img src='param_impact_LSH_L.png' alt='LSH L parameter impact'>\n";
            html << "<img src='param_impact_LSH_w.png' alt='LSH w parameter impact'>\n";
            html << "</div>\n";
        }
        // Add similar blocks for other algorithms...
    }
    
    html << "<h2>Raw Data</h2>\n";
    html << "<table>\n";
    html << "<tr><th>Algorithm</th><th>AF</th><th>Recall@N</th><th>QPS</th><th>Time (ms)</th><th>Memory (MB)</th></tr>\n";
    
    for (const auto& result : results) {
        html << "<tr>\n";
        html << "<td>" << result.algorithm_name << "</td>\n";
        html << "<td>" << std::fixed << std::setprecision(3) << result.average_approximation_factor << "</td>\n";
        html << "<td>" << std::fixed << std::setprecision(3) << result.recall_at_n << "</td>\n";
        html << "<td>" << std::fixed << std::setprecision(1) << result.queries_per_second << "</td>\n";
        html << "<td>" << std::fixed << std::setprecision(2) << result.average_approximate_time_ms << "</td>\n";
        html << "<td>" << std::fixed << std::setprecision(1) << (result.memory_usage_bytes / (1024.0 * 1024.0)) << "</td>\n";
        html << "</tr>\n";
    }
    
    html << "</table>\n";
    
    html << "</body>\n</html>\n";
    html.close();
    
    std::cout << "HTML report created: " << filename << std::endl;
}

// Explicit template instantiations
template class ResultsVisualizer<float>;
template class ResultsVisualizer<double>;
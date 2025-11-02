#!/usr/bin/env python3
"""
Parse all result files from run_expiramental_study.sh and generate a comprehensive report
matching the format in report.md
"""

import os
import re
import sys
from pathlib import Path
from typing import Dict, List, Tuple
import json

RESULTS_DIR = Path("results/report_tests")
OUTPUT_MD = Path("results/REPORT_TESTS_SUMMARY.md")
OUTPUT_JSON = Path("results/REPORT_TESTS_SUMMARY.json")


def parse_result_file(filepath: Path) -> Dict:
    """Parse a single result file and extract metrics."""
    if not filepath.exists():
        return None
    
    try:
        with open(filepath, 'r') as f:
            content = f.read()
        
        # Extract final statistics
        recall_match = re.search(r'Recall@N:\s+([\d.]+)', content)
        af_match = re.search(r'Average AF:\s+([\d.]+)', content)
        qps_match = re.search(r'QPS:\s+([\d.]+)', content)
        approx_time_match = re.search(r'tApproximateAverage:\s+([\d.e+-]+)', content)
        true_time_match = re.search(r'tTrueAverage:\s+([\d.e+-]+)', content)
        
        result = {
            'recall': float(recall_match.group(1)) if recall_match else None,
            'af': float(af_match.group(1)) if af_match else None,
            'qps': float(qps_match.group(1)) if qps_match else None,
            'approx_time': float(approx_time_match.group(1)) if approx_time_match else None,
            'true_time': float(true_time_match.group(1)) if true_time_match else None,
        }
        
        return result
    except Exception as e:
        print(f"Warning: Failed to parse {filepath}: {e}", file=sys.stderr)
        return None


def format_float(val, decimals=5):
    """Format float with specified decimals, or return empty if None."""
    if val is None:
        return ""
    return f"{val:.{decimals}f}"


# Reuse all table generators from analyze_report_tests.py by importing its logic
# For simplicity and to avoid duplication, we inline the same logic here.
# If you prefer, you can refactor to import shared functions.

from analyze_report_tests import (
    generate_lsh_mnist_tables,
    generate_lsh_sift_tables,
    generate_hypercube_mnist_tables,
    generate_hypercube_sift_tables,
    generate_ivfflat_mnist_tables,
    generate_ivfflat_sift_tables,
    generate_ivfpq_mnist_tables,
)


def main():
    """Main entry point."""
    if not RESULTS_DIR.exists():
        print(f"Error: Results directory not found: {RESULTS_DIR}", file=sys.stderr)
        sys.exit(1)
    
    print("Parsing test results and generating report...")
    
    # Generate markdown report
    md_content = "# Report Test Results Summary\n\n"
    md_content += f"Generated from: `{RESULTS_DIR}`\n\n"
    
    md_content += "## LSH\n"
    md_content += generate_lsh_mnist_tables(RESULTS_DIR)
    md_content += "\n"
    md_content += generate_lsh_sift_tables(RESULTS_DIR)
    md_content += "\n\n"
    
    md_content += "## HYPERCUBE\n"
    md_content += generate_hypercube_mnist_tables(RESULTS_DIR)
    md_content += "\n"
    md_content += generate_hypercube_sift_tables(RESULTS_DIR)
    md_content += "\n\n"
    
    md_content += "## IVFFlat\n"
    md_content += generate_ivfflat_mnist_tables(RESULTS_DIR)
    md_content += "\n"
    md_content += generate_ivfflat_sift_tables(RESULTS_DIR)
    md_content += "\n\n"
    
    md_content += "## IVFPQ\n"
    md_content += generate_ivfpq_mnist_tables(RESULTS_DIR)
    md_content += "\n"
    
    # Write markdown
    OUTPUT_MD.parent.mkdir(parents=True, exist_ok=True)
    with open(OUTPUT_MD, 'w') as f:
        f.write(md_content)
    
    print(f"\n✓ Markdown report: {OUTPUT_MD}")
    print(f"  Total size: {OUTPUT_MD.stat().st_size:,} bytes")
    
    # Collect all data for JSON export
    all_results = {}
    for algo in ['lsh', 'hypercube', 'ivfflat', 'ivfpq']:
        algo_dir = RESULTS_DIR / algo
        if not algo_dir.exists():
            continue
        all_results[algo] = {}
        for result_file in algo_dir.glob('*.txt'):
            data = parse_result_file(result_file)
            if data:
                all_results[algo][result_file.stem] = data
    
    with open(OUTPUT_JSON, 'w') as f:
        json.dump(all_results, f, indent=2)
    
    print(f"✓ JSON data: {OUTPUT_JSON}")
    print(f"  Total tests: {sum(len(v) for v in all_results.values())}")
    print("\nDone!")


if __name__ == "__main__":
    main()

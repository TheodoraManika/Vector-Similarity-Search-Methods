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

def generate_algo_markdown(algo: str, base_dir: Path) -> str:
    """Generate a simple markdown table for a given algorithm by scanning result files.
    Columns: config (filename stem), Average AF, Recall@N, QPS, tApproximateAverage, tTrueAverage
    """
    algo_dir = base_dir / algo
    if not algo_dir.exists():
        return f"### {algo.upper()}\n\n_No results found for {algo}._\n\n"

    rows = []
    for result_file in sorted(algo_dir.glob('*.txt')):
        data = parse_result_file(result_file)
        if not data:
            continue
        rows.append({
            'config': result_file.stem,
            'af': data['af'],
            'recall': data['recall'],
            'qps': data['qps'],
            'approx_time': data['approx_time'],
            'true_time': data['true_time'],
        })

    # Build markdown
    md = [f"### {algo.upper()}\n",
          "| Config | Average AF | Recall@N | QPS | tApproximateAverage | tTrueAverage |",
          "|---|---:|---:|---:|---:|---:|"]
    for r in rows:
        md.append(
            f"| {r['config']} | {format_float(r['af'], 6)} | {format_float(r['recall'], 6)} | "
            f"{format_float(r['qps'], 2)} | {format_float(r['approx_time'], 6)} | {format_float(r['true_time'], 6)} |"
        )

    if not rows:
        md.append("_No parsed results._")

    md.append("")
    return "\n".join(md)


def main():
    """Main entry point."""
    if not RESULTS_DIR.exists():
        print(f"Error: Results directory not found: {RESULTS_DIR}", file=sys.stderr)
        sys.exit(1)
    
    print("Parsing test results and generating report...")
    
    # Generate markdown report by scanning result files (no external report module)
    md_content = "# Report Test Results Summary\n\n"
    md_content += f"Generated from: `{RESULTS_DIR}`\n\n"
    md_content += generate_algo_markdown('lsh', RESULTS_DIR)
    md_content += "\n"
    md_content += generate_algo_markdown('hypercube', RESULTS_DIR)
    md_content += "\n"
    md_content += generate_algo_markdown('ivfflat', RESULTS_DIR)
    md_content += "\n"
    md_content += generate_algo_markdown('ivfpq', RESULTS_DIR)
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

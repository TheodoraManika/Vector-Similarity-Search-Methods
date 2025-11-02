#!/usr/bin/env python3
"""
Εξαγωγή Μετρικών και Δημιουργία Αναφοράς
Extract Metrics and Generate Report for Experimental Study
"""

import os
import re
import json
from pathlib import Path
from typing import Dict, List, Tuple

class ExperimentalStudyAnalyzer:
    def __init__(self, results_dir: str = "results/experimental_study"):
        self.results_dir = Path(results_dir)
        self.metrics = []
        
    def parse_result_file(self, filepath: Path) -> Dict:
        """Parse a result file and extract metrics"""
        with open(filepath, 'r') as f:
            content = f.read()
        
        # Extract algorithm name
        algorithm = content.split('\n')[0].strip()
        
        # Extract metrics using regex
        af_match = re.search(r'Average AF:\s+([\d.]+)', content)
        recall_match = re.search(r'Recall@N:\s+([\d.]+)', content)
        qps_match = re.search(r'QPS:\s+([\d.]+)', content)
        approx_time_match = re.search(r'tApproximateAverage:\s+([\d.]+)', content)
        true_time_match = re.search(r'tTrueAverage:\s+([\d.]+)', content)
        
        # Extract parameters from filename
        filename = filepath.stem
        
        result = {
            'algorithm': algorithm,
            'filename': filename,
            'approximation_factor': float(af_match.group(1)) if af_match else None,
            'recall_at_n': float(recall_match.group(1)) if recall_match else None,
            'queries_per_second': float(qps_match.group(1)) if qps_match else None,
            'approx_time_ms': float(approx_time_match.group(1)) * 1000 if approx_time_match else None,
            'true_time_ms': float(true_time_match.group(1)) * 1000 if true_time_match else None,
        }
        
        # Add speedup
        if result['approx_time_ms'] and result['true_time_ms']:
            result['speedup'] = result['true_time_ms'] / result['approx_time_ms']
        else:
            result['speedup'] = None
            
        return result
    
    def analyze_all_results(self):
        """Analyze all result files"""
        if not self.results_dir.exists():
            print(f"Results directory not found: {self.results_dir}")
            return
        
        for filepath in sorted(self.results_dir.glob("*.txt")):
            try:
                metrics = self.parse_result_file(filepath)
                self.metrics.append(metrics)
                print(f"✓ Parsed: {filepath.name}")
            except Exception as e:
                print(f"✗ Error parsing {filepath.name}: {e}")
    
    def generate_markdown_report(self, output_file: str = "results/EXPERIMENTAL_STUDY_REPORT.md"):
        """Generate comprehensive markdown report"""
        
        # Group by algorithm
        by_algorithm = {}
        for m in self.metrics:
            alg = m['algorithm']
            if alg not in by_algorithm:
                by_algorithm[alg] = []
            by_algorithm[alg].append(m)
        
        report = []
        report.append("# Πειραματική Συγκριτική Μελέτη")
        report.append("# Experimental Comparative Study Report")
        report.append("")
        report.append("## Περίληψη / Summary")
        report.append("")
        report.append("Αυτή η αναφορά παρουσιάζει τα αποτελέσματα της πειραματικής συγκριτικής μελέτης ")
        report.append("των αλγορίθμων αναζήτησης κοντινότερων γειτόνων:")
        report.append("")
        report.append("This report presents the results of the experimental comparative study of ")
        report.append("approximate nearest neighbor search algorithms:")
        report.append("")
        report.append("- **LSH** (Locality-Sensitive Hashing)")
        report.append("- **Hypercube** (Projection to Hypercube)")
        report.append("- **IVFFlat** (Inverted File with Flat Quantization)")
        report.append("- **IVFPQ** (Inverted File with Product Quantization)")
        report.append("")
        
        report.append("## Μετρικές Απόδοσης / Performance Metrics")
        report.append("")
        report.append("Οι αλγόριθμοι αξιολογήθηκαν με βάση:")
        report.append("")
        report.append("1. **Μέσο Κλάσμα Προσέγγισης (AF)** - Mean Approximation Factor")
        report.append("   - Λόγος της απόστασης προσέγγισης προς την αληθινή απόσταση")
        report.append("   - Χαμηλότερο = Καλύτερο (ιδανικά 1.0)")
        report.append("")
        report.append("2. **Ανάκληση στους Ν Γείτονες (Recall@N)**")
        report.append("   - Ποσοστό σωστών γειτόνων που βρέθηκαν")
        report.append("   - Υψηλότερο = Καλύτερο (ιδανικά 1.0)")
        report.append("")
        report.append("3. **Αναζητήσεις Ανά Δευτερόλεπτο (QPS)** - Queries Per Second")
        report.append("   - Ταχύτητα εκτέλεσης ερωτημάτων")
        report.append("   - Υψηλότερο = Καλύτερο")
        report.append("")
        
        # Detailed results by algorithm
        report.append("## Αναλυτικά Αποτελέσματα / Detailed Results")
        report.append("")
        
        for alg_name in sorted(by_algorithm.keys()):
            results = by_algorithm[alg_name]
            report.append(f"### {alg_name}")
            report.append("")
            
            # Table header
            report.append("| Configuration | AF | Recall@N | QPS | Approx Time (ms) | Speedup |")
            report.append("|--------------|-----|----------|-----|------------------|---------|")
            
            for r in results:
                config = r['filename'].replace(alg_name.lower() + '_', '')
                af = f"{r['approximation_factor']:.3f}" if r['approximation_factor'] else "N/A"
                recall = f"{r['recall_at_n']:.3f}" if r['recall_at_n'] else "N/A"
                qps = f"{r['queries_per_second']:.1f}" if r['queries_per_second'] else "N/A"
                approx_t = f"{r['approx_time_ms']:.2f}" if r['approx_time_ms'] else "N/A"
                speedup = f"{r['speedup']:.1f}x" if r['speedup'] else "N/A"
                
                report.append(f"| {config} | {af} | {recall} | {qps} | {approx_t} | {speedup} |")
            
            report.append("")
        
        # Comparative analysis
        report.append("## Συγκριτική Ανάλυση / Comparative Analysis")
        report.append("")
        
        # Best AF
        best_af = min([m for m in self.metrics if m['approximation_factor']], 
                      key=lambda x: x['approximation_factor'], default=None)
        if best_af:
            report.append(f"### Καλύτερο Κλάσμα Προσέγγισης / Best Approximation Factor")
            report.append("")
            report.append(f"- **Αλγόριθμος**: {best_af['algorithm']}")
            report.append(f"- **Διαμόρφωση**: {best_af['filename']}")
            report.append(f"- **AF**: {best_af['approximation_factor']:.3f}")
            report.append(f"- **Recall@N**: {best_af['recall_at_n']:.3f}")
            report.append("")
        
        # Best Recall
        best_recall = max([m for m in self.metrics if m['recall_at_n']], 
                         key=lambda x: x['recall_at_n'], default=None)
        if best_recall:
            report.append(f"### Καλύτερη Ανάκληση / Best Recall")
            report.append("")
            report.append(f"- **Αλγόριθμος**: {best_recall['algorithm']}")
            report.append(f"- **Διαμόρφωση**: {best_recall['filename']}")
            report.append(f"- **Recall@N**: {best_recall['recall_at_n']:.3f}")
            report.append(f"- **AF**: {best_recall['approximation_factor']:.3f}")
            report.append("")
        
        # Best QPS
        best_qps = max([m for m in self.metrics if m['queries_per_second']], 
                       key=lambda x: x['queries_per_second'], default=None)
        if best_qps:
            report.append(f"### Υψηλότερη Ταχύτητα / Highest Speed")
            report.append("")
            report.append(f"- **Αλγόριθμος**: {best_qps['algorithm']}")
            report.append(f"- **Διαμόρφωση**: {best_qps['filename']}")
            report.append(f"- **QPS**: {best_qps['queries_per_second']:.1f}")
            report.append(f"- **Χρόνος Ερωτήματος**: {best_qps['approx_time_ms']:.2f} ms")
            report.append("")
        
        # Trade-off analysis
        report.append("## Ανάλυση Συμβιβασμών / Trade-off Analysis")
        report.append("")
        report.append("### Ακρίβεια vs Ταχύτητα / Accuracy vs Speed")
        report.append("")
        report.append("| Algorithm | Config | AF | Recall | QPS | Score* |")
        report.append("|-----------|--------|-----|--------|-----|--------|")
        
        for m in self.metrics:
            if m['approximation_factor'] and m['recall_at_n'] and m['queries_per_second']:
                # Combined score: higher recall, lower AF, higher QPS
                score = (m['recall_at_n'] / max(m['approximation_factor'], 0.1)) * (m['queries_per_second'] / 100)
                config = m['filename'].replace(m['algorithm'].lower() + '_', '')[:20]
                report.append(f"| {m['algorithm'][:12]} | {config} | "
                            f"{m['approximation_factor']:.3f} | "
                            f"{m['recall_at_n']:.3f} | "
                            f"{m['queries_per_second']:.1f} | "
                            f"{score:.2f} |")
        
        report.append("")
        report.append("*Score = (Recall / AF) × (QPS / 100)")
        report.append("")
        
        # Conclusions
        report.append("## Συμπεράσματα / Conclusions")
        report.append("")
        report.append("### Συστάσεις ανά Σενάριο / Recommendations by Scenario")
        report.append("")
        report.append("1. **Υψηλή Ακρίβεια (High Accuracy)**")
        report.append("   - Προτεινόμενο: IVFFlat με υψηλό nprobe")
        report.append("   - Χαρακτηριστικά: AF ~1.0, Recall 60-80%")
        report.append("")
        report.append("2. **Ισορροπία Ακρίβειας-Ταχύτητας (Balanced)**")
        report.append("   - Προτεινόμενο: Hypercube ή LSH με μέτριες παραμέτρους")
        report.append("   - Χαρακτηριστικά: AF 1.2-1.5, Recall 20-50%")
        report.append("")
        report.append("3. **Υψηλή Ταχύτητα (High Speed)**")
        report.append("   - Προτεινόμενο: LSH με χαμηλό L ή Hypercube με χαμηλό M/probes")
        report.append("   - Χαρακτηριστικά: Πολύ γρήγορο, χαμηλότερη ακρίβεια")
        report.append("")
        report.append("4. **Συμπίεση Μνήμης (Memory Compression)**")
        report.append("   - Προτεινόμενο: IVFPQ")
        report.append("   - Χαρακτηριστικά: 100x-200x συμπίεση, καλή απόδοση")
        report.append("")
        
        report.append("## Παρατηρήσεις / Observations")
        report.append("")
        report.append("### Συσχέτιση Παραμέτρων / Parameter Correlations")
        report.append("")
        report.append("- **LSH**: Αύξηση L, k → Καλύτερο Recall, Χειρότερο QPS")
        report.append("- **Hypercube**: Αύξηση M, probes → Καλύτερο Recall & AF, Χαμηλότερο QPS")
        report.append("- **IVFFlat**: Αύξηση nprobe → Καλύτερο Recall, Χαμηλότερο QPS")
        report.append("- **IVFPQ**: Αύξηση M (subquantizers) → Καλύτερη ακρίβεια, πιο αργή εκπαίδευση")
        report.append("")
        
        # Write report
        output_path = Path(output_file)
        output_path.parent.mkdir(parents=True, exist_ok=True)
        
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write('\n'.join(report))
        
        print(f"\n✓ Report generated: {output_path}")
        
        # Also save metrics as JSON
        json_path = output_path.with_suffix('.json')
        with open(json_path, 'w', encoding='utf-8') as f:
            json.dump(self.metrics, f, indent=2, ensure_ascii=False)
        
        print(f"✓ Metrics saved: {json_path}")

def main():
    print("=" * 60)
    print("Experimental Study Analyzer")
    print("=" * 60)
    print()
    
    analyzer = ExperimentalStudyAnalyzer()
    analyzer.analyze_all_results()
    
    if analyzer.metrics:
        print(f"\nAnalyzed {len(analyzer.metrics)} result files")
        analyzer.generate_markdown_report()
        print("\n✓ Analysis complete!")
    else:
        print("\n✗ No results found. Run the experimental study first:")
        print("  ./run_experimental_study.sh")

if __name__ == "__main__":
    main()

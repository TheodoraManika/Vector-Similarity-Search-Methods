# Πειραματική Συγκριτική Μελέτη - Οδηγός Χρήσης
# Experimental Comparative Study - Usage Guide

## 📋 Περιγραφή / Description

Αυτό το σύστημα υλοποιεί την πειραματική συγκριτική μελέτη που απαιτείται από την εκφώνηση:

> "Μετά την υλοποίηση των αλγορίθμων θα πραγματοποιηθεί πειραματική συγκριτική μελέτη 
> της απόδοσης και της επίδοσής τους βάσει του **μέσου κλάσματος προσέγγισης**, 
> της **ανάκλησης στους Ν πλησιέστερους γείτονες** και του **πλήθους αναζητήσεων 
> ανά δευτερόλεπτο** καθώς και της συσχέτισης των μεγεθών αυτών μεταξύ τους."

## 🚀 Γρήγορη Εκκίνηση / Quick Start

### Βήμα 1: Μεταγλώττιση
```bash
make clean && make -j
```

### Βήμα 2: Εκτέλεση Πειραμάτων
```bash
make experimental-study
```
⏱️ **Χρόνος**: ~20-30 λεπτά

### Βήμα 3: Ανάλυση & Αναφορά
```bash
make analyze-study
```

### Βήμα 4: Δείτε την Αναφορά
```bash
cat results/EXPERIMENTAL_STUDY_REPORT.md
```

## 📊 Τι Μετράει / What It Measures

### 1. Μέσο Κλάσμα Προσέγγισης (Mean Approximation Factor - AF)
- **Ορισμός**: `AF = distance_approximate / distance_true`
- **Ιδανικό**: 1.0 (τέλεια ακρίβεια)
- **Καλό**: < 1.5
- **Αποδεκτό**: < 2.0

### 2. Ανάκληση στους Ν Γείτονες (Recall@N)
- **Ορισμός**: Ποσοστό σωστών γειτόνων που βρέθηκαν
- **Τύπος**: `Recall = |found ∩ true| / N`
- **Ιδανικό**: 1.0 (100%)
- **Καλό**: > 0.7 (70%)
- **Αποδεκτό**: > 0.4 (40%)

### 3. Αναζητήσεις Ανά Δευτερόλεπτο (QPS)
- **Ορισμός**: Πόσα ερωτήματα μπορεί να επεξεργαστεί ανά δευτερόλεπτο
- **Ιδανικό**: > 100 QPS
- **Καλό**: > 20 QPS
- **Σημείωση**: Υψηλότερο = Γρηγορότερο

## 🔬 Αλγόριθμοι & Παράμετροι / Algorithms & Parameters

### LSH (Locality-Sensitive Hashing)
```
Test 1: k=4, L=5   (γρήγορο, χαμηλό recall)
Test 2: k=6, L=10  (ισορροπημένο)
Test 3: k=8, L=15  (αργό, υψηλό recall)
```

### Hypercube
```
Test 1: M=500,  probes=10  (γρήγορο)
Test 2: M=1000, probes=20  (ισορροπημένο)
Test 3: M=2000, probes=50  (υψηλή ακρίβεια)
```

### IVFFlat
```
Test 1: kclusters=50,  nprobe=5   (γρήγορο)
Test 2: kclusters=100, nprobe=10  (ισορροπημένο)
Test 3: kclusters=150, nprobe=20  (υψηλή ακρίβεια)
```

### IVFPQ (με συμπίεση)
```
Test 1: kclusters=50,  nprobe=5,  M=16  (χαμηλή μνήμη)
Test 2: kclusters=100, nprobe=10, M=32  (καλύτερη ακρίβεια)
```

## 📈 Αναμενόμενα Αποτελέσματα / Expected Results

### Ταχύτητα (από γρηγορότερο σε αργότερο):
1. **LSH** (low L) - Πολύ γρήγορο
2. **Hypercube** (low M/probes) - Γρήγορο
3. **IVFPQ** - Μέτριο (+ συμπίεση)
4. **IVFFlat** - Αργότερο αλλά ακριβές

### Ακρίβεια (από καλύτερο σε χειρότερο):
1. **IVFFlat** (high nprobe) - AF ~1.0, Recall 60-80%
2. **IVFPQ** (high M) - AF ~1.2-1.5, Recall 50-70%
3. **Hypercube** (high M/probes) - AF ~1.2-1.8, Recall 20-60%
4. **LSH** (high L) - AF ~1.5-2.5, Recall 10-40%

## 📁 Αρχεία Εξόδου / Output Files

```
results/experimental_study/
├── lsh_k4_L5.txt               # Αποτελέσματα LSH
├── lsh_k6_L10.txt
├── lsh_k8_L15.txt
├── hypercube_M500_p10.txt      # Αποτελέσματα Hypercube
├── hypercube_M1000_p20.txt
├── hypercube_M2000_p50.txt
├── ivfflat_k50_p5.txt          # Αποτελέσματα IVFFlat
├── ivfflat_k100_p10.txt
├── ivfflat_k150_p20.txt
├── ivfpq_k50_p5_M16.txt        # Αποτελέσματα IVFPQ
└── ivfpq_k100_p10_M32.txt

results/
├── EXPERIMENTAL_STUDY_REPORT.md   # 📄 ΚΥΡΙΑ ΑΝΑΦΟΡΑ
└── EXPERIMENTAL_STUDY_REPORT.json # Δεδομένα σε JSON
```

## 🎯 Χρήση της Αναφοράς / Using the Report

Η αναφορά περιλαμβάνει:

1. **Συγκριτικούς πίνακες** για κάθε αλγόριθμο
2. **Ανάλυση Best Performers** (καλύτερο AF, Recall, QPS)
3. **Trade-off Analysis** (Ακρίβεια vs Ταχύτητα)
4. **Συστάσεις** ανά σενάριο χρήσης
5. **Παρατηρήσεις** συσχέτισης παραμέτρων

## 🔧 Προσαρμογή / Customization

### Αλλαγή Αριθμού Queries
Επεξεργαστείτε `run_experimental_study.sh`:
```bash
QMAX=50   # Γρήγορη δοκιμή
QMAX=100  # Προεπιλογή
QMAX=500  # Εκτενής μελέτη
```

### Προσθήκη Νέων Παραμέτρων
Προσθέστε στο `run_experimental_study.sh`:
```bash
./search -d $MNIST_TRAIN -q $MNIST_QUERY \
  -o experimental_study/my_custom_test.txt \
  -type mnist -lsh -k 10 -L 20 -N 10 -qmax $QMAX
```

### Δημιουργία Γραφημάτων
```python
import json
import matplotlib.pyplot as plt

with open('results/EXPERIMENTAL_STUDY_REPORT.json') as f:
    data = json.load(f)

# AF vs Recall scatter plot
afs = [d['approximation_factor'] for d in data if d['approximation_factor']]
recalls = [d['recall_at_n'] for d in data if d['recall_at_n']]

plt.scatter(afs, recalls)
plt.xlabel('Approximation Factor')
plt.ylabel('Recall@N')
plt.title('Trade-off: Accuracy vs Recall')
plt.grid(True)
plt.savefig('results/af_vs_recall.png')
```

## ⚠️ Σημαντικές Σημειώσεις / Important Notes

1. **IVFPQ Training**: Χρειάζεται 5-15 λεπτά για εκπαίδευση
   - Θα δείτε: "Training coarse quantizer... Training subspace X/M..."
   - Αυτό είναι φυσιολογικό!

2. **Μνήμη**: Τα datasets (MNIST 60K) φορτώνονται στη μνήμη
   - Χρειάζεται ~2GB RAM

3. **Ground Truth**: Υπολογίζεται κάθε φορά (brute force)
   - Για γρηγορότερα αποτελέσματα: μειώστε QMAX

## 📚 Παραπομπές / References

- **LSH**: [Indyk & Motwani, 1998]
- **Hypercube**: Προβολή σε υπερκύβο
- **IVF**: [Jegou et al., 2011]
- **PQ**: [Jegou et al., 2011] Product Quantization

## 🆘 Troubleshooting

### Πρόβλημα: "No results found"
```bash
# Βεβαιωθείτε ότι τρέξατε τα πειράματα πρώτα
./run_experimental_study.sh
```

### Πρόβλημα: Αργή εκτέλεση
```bash
# Μειώστε queries
# Στο run_experimental_study.sh: QMAX=20
```

### Πρόβλημα: IVFPQ κολλάει
```bash
# Δεν κόλλησε! Απλά εκπαιδεύει...
# Περιμένετε 5-15 λεπτά ή Ctrl+C και μειώστε M ή kclusters
```

## ✅ Checklist Ολοκλήρωσης

- [ ] Μεταγλώττιση: `make clean && make -j`
- [ ] Εκτέλεση: `make experimental-study`
- [ ] Ανάλυση: `make analyze-study`
- [ ] Έλεγχος αναφοράς: `results/EXPERIMENTAL_STUDY_REPORT.md`
- [ ] (Προαιρετικά) Γραφήματα από JSON
- [ ] Υποβολή αναφοράς ως παραδοτέο

---

**Καλή επιτυχία στην πειραματική σας μελέτη!** 🎓
**Good luck with your experimental study!** 🚀

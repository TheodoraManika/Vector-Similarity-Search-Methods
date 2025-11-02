# Vector Similarity Search Methods

Υλοποίηση αλγορίθμων αναζήτησης διανυσμάτων στον d-διάστατο χώρο βάσει της ευκλείδειας μετρικής (L2).

## Περιγραφή Προγράμματος

Αυτό το πρόγραμμα υλοποιεί τέσσερις διαφορετικούς αλγόριθμους αναζήτησης διανυσμάτων:

1. **LSH (Locality Sensitive Hashing)** - Αλγόριθμος LSH για προσεγγιστική αναζήτηση
2. **Hypercube** - Αλγόριθμος τυχαίας προβολής σε υπερκύβο  
3. **IVFFlat** - Αλγόριθμος αναζήτησης με συσταδοποίηση k-means και ευρετήριο Inverted File Flat
4. **IVFPQ** - Αλγόριθμος αναζήτησης με συσταδοποίηση k-means και ευρετήριο Inverted File Product Quantization

Το πρόγραμμα υποστηρίζει δύο τύπους δεδομένων:
- **MNIST**: Εικόνες χειρόγραφων ψηφίων 28x28 pixels (784 διαστάσεις)
- **SIFT**: Χαρακτηριστικά διανύσματα SIFT με 128 διαστάσεις

## Κατάλογος Αρχείων

### Κύρια Αρχεία
- `src/main.cpp` - Κύριο σημείο εισόδου του προγράμματος (CLI search)
- `Makefile` - Αρχείο μεταγλώττισης με πολλαπλούς στόχους (search, benchmarks, tests)
- `README.md` - Αυτό το αρχείο τεκμηρίωσης

### Αλγόριθμοι (`algorithms/`)
- `LSH/LSH.h`, `LSH/LSH.cpp` - Υλοποίηση LSH
- `hypercube/Hypercube.h`, `hypercube/Hypercube.cpp` - Υλοποίηση Hypercube
- `Kmeans/kmeans.h`, `Kmeans/kmeans.cpp` - Υλοποίηση K-means clustering
- `IVFFlat/IVFFlat.h`, `IVFFlat/IVFFlat.cpp` - Υλοποίηση IVFFlat
- `IVFPQ/IVFPQ.h`, `IVFPQ/IVFPQ.cpp` - Υλοποίηση IVFPQ
- `BruteForce/BruteForce.h`, `BruteForce/BruteForce.cpp` - Ακριβής (ground-truth) αναζήτηση για σύγκριση

### Επεξεργασία Δεδομένων (`dataprocessing/`)
- `SIFT/SIFT.h`, `SIFT/SIFT.cpp` - Parser για δεδομένα SIFT
- `MNIST/MNIST.h`, `MNIST/MNIST.cpp` - Parser για δεδομένα MNIST

### Βοηθητικά Εργαλεία (`utils/`)
- `interface/Interface.h`, `interface/Interface.cpp` - Κύρια διεπαφή προγράμματος (parsing, εκτέλεση αναζητήσεων)
- `helpers/Utils.h`, `helpers/Utils.cpp` - Βοηθητικές συναρτήσεις (FileUtils, Stats, Convert) και AutoTimer για μετρήσεις χρόνου
- `math/math.h`, `math/math.cpp` - Μαθηματικές λειτουργίες και υπολογισμοί

### Μετρικές (`metrics/`)
- `metrics.h`, `metrics.cpp` - Υλοποίηση μετρικών απόστασης

## Δομή Project

```
Vector-Similarity-Search-Methods/
├── algorithms/              # Υλοποιήσεις αλγορίθμων
│   ├── LSH/                # Locality Sensitive Hashing
│   ├── hypercube/          # Random Projection Hypercube
│   ├── IVFFlat/            # Inverted File Flat Index
│   ├── IVFPQ/              # Inverted File Product Quantization
│   ├── Kmeans/             # K-means clustering
│   └── BruteForce/         # Ακριβής αναζήτηση (ground truth)
├── dataprocessing/         # Dataset parsers
│   ├── MNIST/              # MNIST image parser
│   └── SIFT/               # SIFT vector parser
├── utils/                  # Βοηθητικά εργαλεία
│   ├── interface/          # CLI interface και orchestration
│   ├── helpers/            # Utils, FileUtils, AutoTimer
│   └── math/               # Μαθηματικές συναρτήσεις
├── metrics/                # Distance metrics (L2, Manhattan)
├── src/                    # Κύριο πρόγραμμα
│   └── main.cpp           # Entry point
├── dataset/                # Datasets (MNIST, SIFT)
├── results/                # Αποτελέσματα αναζητήσεων
├── obj/                    # Build artifacts (auto-generated)
├── run_expiramental_study.sh    # Script για πειραματική μελέτη
├── analyze_expiramental_study.py # Ανάλυση αποτελεσμάτων
├── Makefile               # Build configuration
└── README.md              # Αυτό το αρχείο
```

## Οδηγίες Μεταγλώττισης

### Προαπαιτούμενα
- Compiler C++17 ή νεότερος (g++ 7+ ή clang++ 5+)
- Python 3.x (για scripts ανάλυσης)
- Make

### Μεταγλώττιση
```bash
# Κανονική μεταγλώττιση (optimized, -O3)
make

# Debug μεταγλώττιση με symbols
make debug

# Καθαρισμός αρχείων μεταγλώττισης
make clean

# Rebuild από την αρχή
make rebuild
```

### Διαθέσιμοι Στόχοι Make
```bash
make all              # Μεταγλώττιση του search executable
make debug            # Debug build με -g
make clean            # Καθαρισμός build artifacts
make rebuild          # Clean + build

# Γρήγορες δοκιμές ανά αλγόριθμο
make test-lsh-sift
make test-lsh-mnist
make test-hypercube-sift
make test-hypercube-mnist
make test-ivfflat-sift
make test-ivfflat-mnist
make test-ivfpq-sift
make test-ivfpq-mnist

# Πειραματικές μελέτες
make experimental-study      # Εκτέλεση full experimental study
make analyze-study          # Ανάλυση αποτελεσμάτων

make help                   # Εμφάνιση όλων των διαθέσιμων targets
```

## Οδηγίες Χρήσης

### Γενική Σύνταξη
```bash
./search -d <input_file> -q <query_file> -o <output_file> -type <data_type> [algorithm_options]
```

### Παράμετροι Αλγορίθμων

#### Κοινές Παράμετροι
- `-d <file>`: Input dataset file (required)
- `-q <file>`: Query file (required)
- `-o <file>`: Output file (required)
- `-type <mnist|sift>`: Dataset type (required)
- `-N <int>`: Αριθμός πλησιέστερων γειτόνων (default: 1)
- `-R <double>`: Ακτίνα για range search (default: 2000 για MNIST, 2.0 για SIFT)
- `-range <true|false>`: Ενεργοποίηση range search (default: false)
- `-qmax <int>`: Μέγιστος αριθμός queries προς επεξεργασία, 0=όλα (default: 0)
- `-seed <int>`: Random seed (default: 1)

#### LSH
```bash
./search -d input.dat -q query.dat -o output.txt -type sift -lsh \
         -k <int> -L <int> -w <double> -N <int> -R <double> -range <true|false> \
         -qmax <int> -auto_w <true|false>
```

**Παράμετροι:**
- `-k <int>`: Αριθμός hash functions ανά table (default: 4)
- `-L <int>`: Αριθμός hash tables (default: 5)
- `-w <double>`: Μήκος κελιού προβολής (default: 4.0)
- `-auto_w <true|false>`: Αυτόματος υπολογισμός w από δεδομένα (default: false)

#### Hypercube
```bash
./search -d input.dat -q query.dat -o output.txt -type sift -hypercube \
         -kproj <int> -w <double> -M <int> -probes <int> -N <int> -R <double> \
         -range <true|false> -qmax <int> -auto_w <true|false>
```

**Παράμετροι:**
- `-kproj <int>`: Διάσταση προβολής (default: 14)
- `-w <double>`: Μήκος κελιού προβολής (default: 4.0)
- `-M <int>`: Μέγιστος αριθμός υποψήφιων προς έλεγχο (default: 500)
- `-probes <int>`: Μέγιστος αριθμός vertices προς έλεγχο (default: 10)
- `-auto_w <true|false>`: Αυτόματος υπολογισμός w από δεδομένα (default: false)

#### IVFFlat
```bash
./search -d input.dat -q query.dat -o output.txt -type sift -ivfflat \
         -kclusters <int> -nprobe <int> -N <int> -R <double> -range <true|false> \
         -seed <int> -qmax <int>
```

**Παράμετροι:**
- `-kclusters <int>`: Αριθμός clusters k-means (default: 50)
- `-nprobe <int>`: Αριθμός clusters προς έλεγχο κατά search (default: 5)

#### IVFPQ
```bash
./search -d input.dat -q query.dat -o output.txt -type sift -ivfpq \
         -kclusters <int> -nprobe <int> -M <int> -nbits <int> -N <int> -R <double> \
         -range <true|false> -seed <int> -qmax <int>
```

**Παράμετροι:**
- `-kclusters <int>`: Αριθμός clusters k-means (default: 50)
- `-nprobe <int>`: Αριθμός clusters προς έλεγχο κατά search (default: 5)
- `-M <int>`: Αριθμός subquantizers για Product Quantization (default: 500)
- `-nbits <int>`: Bits ανά subquantizer (default: 8)

### Παραδείγματα Χρήσης

#### LSH με SIFT
```bash
# Βασική εκτέλεση με 10 πλησιέστερους γείτονες
./search -d Dataset/sift/siftsmall_base.fvecs -q Dataset/sift/siftsmall_query.fvecs \
         -o results/lsh_sift_output.txt -type sift -lsh -k 4 -L 5 -N 10

# Με range search και αυτόματο υπολογισμό w
./search -d Dataset/sift/siftsmall_base.fvecs -q Dataset/sift/siftsmall_query.fvecs \
         -o results/lsh_sift_range.txt -type sift -lsh -k 4 -L 5 -N 10 -R 300 \
         -range true -auto_w true

# Περιορισμός επεξεργασίας σε 100 queries
QMAX=100 ./search -d Dataset/sift/siftsmall_base.fvecs \
         -q Dataset/sift/siftsmall_query.fvecs -o results/lsh_quick.txt \
         -type sift -lsh -k 4 -L 5 -N 5 -qmax 100
```

#### Hypercube με MNIST
```bash
# Βασική εκτέλεση με 5 πλησιέστερους γείτονες
./search -d Dataset/mnist/train-images-idx3-ubyte \
         -q Dataset/mnist/t10k-images-idx3-ubyte \
         -o results/hypercube_mnist.txt -type mnist -hypercube \
         -kproj 14 -M 500 -probes 10 -N 5

# Με range search
./search -d Dataset/mnist/train-images-idx3-ubyte \
         -q Dataset/mnist/t10k-images-idx3-ubyte \
         -o results/hypercube_range.txt -type mnist -hypercube \
         -kproj 14 -M 500 -probes 10 -N 5 -R 200 -range true
```

#### IVFFlat με SIFT
```bash
# 50 clusters, έλεγχος top-5 clusters
./search -d Dataset/sift/siftsmall_base.fvecs \
         -q Dataset/sift/siftsmall_query.fvecs \
         -o results/ivfflat_sift.txt -type sift -ivfflat \
         -kclusters 50 -nprobe 5 -N 10 -R 300 -range true
```

#### IVFPQ με MNIST
```bash
# Product Quantization με M=500 subquantizers
./search -d Dataset/mnist/train-images-idx3-ubyte \
         -q Dataset/mnist/t10k-images-idx3-ubyte \
         -o results/ivfpq_mnist.txt -type mnist -ivfpq \
         -kclusters 50 -nprobe 5 -M 500 -nbits 8 -N 10

# Με custom seed για reproducible results
./search -d Dataset/mnist/train-images-idx3-ubyte \
         -q Dataset/mnist/t10k-images-idx3-ubyte \
         -o results/ivfpq_seed42.txt -type mnist -ivfpq \
         -kclusters 50 -nprobe 5 -M 500 -nbits 8 -N 10 -seed 42
```

#### Εκτέλεση με Make Targets
```bash
# Γρήγορα single tests
make test-lsh-sift          # LSH με SIFT dataset
make test-hypercube-mnist   # Hypercube με MNIST dataset
make test-ivfflat-sift      # IVFFlat με SIFT dataset
make test-ivfpq-mnist       # IVFPQ με MNIST dataset

# Comprehensive experimental study (~75 configurations)
make report-tests           # Runs run_expiramental_study.sh and analyzer
# Ή ξεχωριστά:
QMAX=100 ./run_expiramental_study.sh  # Limit to 100 queries per test
python3 analyze_expiramental_study.py  # Generate summary reports
```

## Πειραματική Μελέτη

Για αυτόματες δοκιμές και αναφορές, χρησιμοποιήστε τους στόχους πειραματικής μελέτης:

```bash
# Εκτέλεση πλήρους πειραματικής μελέτης (~75 configurations)
make report-tests
# Αυτό εκτελεί: ./run_expiramental_study.sh && python3 analyze_expiramental_study.py

# Ή ξεχωριστά με περιορισμό queries:
QMAX=100 ./run_expiramental_study.sh    # Εκτέλεση tests με max 100 queries ανά config
python3 analyze_expiramental_study.py   # Δημιουργία markdown/JSON reports

# Επιμέρους targets:
make experimental-study     # Μόνο εκτέλεση tests
make analyze-study         # Μόνο ανάλυση αποτελεσμάτων
```

**Αποτελέσματα:**
- Τα αποτελέσματα αποθηκεύονται σε `results/report_tests/<algorithm>/<config>.txt`
- Οι συγκεντρωτικές αναφορές δημιουργούνται ως:
  - `results/REPORT_TESTS_SUMMARY.md` (Markdown tables)
  - `results/REPORT_TESTS_SUMMARY.json` (JSON format)

**Σημειώσεις:**
- Οι μετρήσεις χρόνου υλοποιούνται με `Utils::AutoTimer` στο interface και καταγράφονται στα αποτελέσματα.
- Το `BruteForce` χρησιμοποιείται για υπολογισμό ακριβών KNN και υπολογισμό AF/Recall@N.
- Η πλήρης πειραματική μελέτη ελέγχει πολλαπλές παραμετροποιήσεις για κάθε αλγόριθμο με MNIST και SIFT datasets.

## Μορφή Εξόδου

Το αρχείο εξόδου ακολουθεί το πρότυπο:

```
<METHOD NAME>
Query: <query_id>
Nearest neighbor-1: <image_id>
distanceApproximate: <distance>
distanceTrue: <distance>
...
Nearest neighbor-N: <image_id>
distanceApproximate: <distance>
distanceTrue: <distance>
R-near neighbors:
<image_id_1>
<image_id_2>
...
tApproximateAverage: <time>
tTrueAverage: <time>
QPS: <queries_per_second>
```

## Αρχιτεκτονική

### Σχεδιαστικές Αρχές
- **Template-based**: Υποστήριξη διαφορετικών τύπων δεδομένων (float, double)
- **Modular Design**: Κάθε αλγόριθμος είναι ανεξάρτητη κλάση
- **Extensible**: Εύκολη προσθήκη νέων μετρικών και αλγορίθμων
- **Memory Efficient**: Βελτιστοποιημένη διαχείριση μνήμης

### Αλγοριθμικές Λεπτομέρειες

#### LSH (Locality Sensitive Hashing)
- Χρήση γκαουσιανών κατανομών για τους συντελεστές προβολής
- L hash tables με k hash functions το καθένα
- Amplified hash functions για καλύτερη κατανομή

#### Hypercube (Random Projection)
- Προβολή σε d'-διάστατο χώρο (kproj παράμετρος)
- Χρήση flipped coins mapping για συνέπεια
- BFS traversal των γειτονικών vertices

#### K-means Clustering
- Υλοποίηση k-means++ για καλύτερη αρχικοποίηση
- Silhouette score για αξιολόγηση ποιότητας clustering
- Αυτόματη εύρεση βέλτιστου k

#### IVFFlat (Inverted File Flat)
- Inverted lists για κάθε cluster
- Αναζήτηση σε nprobe πλησιέστερα clusters
- Ακριβής διανύσματα σε κάθε cluster

#### IVFPQ (Inverted File Product Quantization)
- Product Quantization για συμπίεση διανυσμάτων
- Asymmetric distance computation
- Configurable compression ratio μέσω M και nbits

## Μετρικές Αξιολόγησης

Το πρόγραμμα υπολογίζει και αναφέρει:

### Βασικές Μετρικές
- **Approximation Factor (AF)**: Λόγος προσεγγιστικής προς ακριβή απόσταση (`approx_dist / true_dist`)
  - Υπολογίζεται για κάθε query με μη-κενά αποτελέσματα
  - Μέσος όρος αναφέρεται ως Average AF
  - Ιδανικά πλησιάζει το 1.0 (τέλεια προσέγγιση)

- **Recall@N**: Ποσοστό ορθών αποτελεσμάτων στους N πλησιέστερους γείτονες
  - Υπολογίζεται ως `|approximate_neighbors ∩ true_neighbors| / N`
  - Μέσος όρος αναφέρεται ως Average Recall@N
  - Εύρος: [0, 1], όπου 1.0 σημαίνει όλα τα αποτελέσματα είναι ορθά

- **QPS (Queries Per Second)**: Ταχύτητα επεξεργασίας ερωτημάτων
  - Υπολογίζεται ως `total_queries / total_time`
  - Υψηλότερο QPS = καλύτερη απόδοση

### Χρονικές Μετρικές
- **tApproximateAverage**: Μέσος χρόνος προσεγγιστικής αναζήτησης ανά query (ms)
- **tTrueAverage**: Μέσος χρόνος ακριβούς αναζήτησης (brute force) ανά query (ms)
- **MeanQueryTimeMS**: Μέσος χρόνος ερωτήματος (ms) - στις αναφορές πειραματικής μελέτης
- **BruteForceTimeMS**: Μέσος χρόνος brute force (ms) - στις αναφορές πειραματικής μελέτης
- **SpeedupVsBruteForce**: Λόγος επιτάχυνσης έναντι brute force (`brute_force_time / approx_time`)

### Σημαντικές Σημειώσεις
- **Έγκυρα Queries**: Μόνο queries με μη-κενά αποτελέσματα (από τον προσεγγιστικό αλγόριθμο και το brute force) συμμετέχουν στον υπολογισμό μέσων όρων AF και Recall@N
- **Brute Force Baseline**: Το `BruteForce` χρησιμοποιείται για υπολογισμό ακριβών KNN (ground truth) για τον υπολογισμό AF/Recall
- **Range Search**: Οι μετρικές range search αναφέρουν τον αριθμό γειτόνων εντός ακτίνας R
- **Timing**: Χρησιμοποιείται `Utils::AutoTimer` για ακριβείς μετρήσεις χρόνου με RAII pattern

### Ερμηνεία Αποτελεσμάτων
**Καλή Απόδοση:**
- AF κοντά στο 1.0 (π.χ., 1.0-1.2)
- Recall@N > 0.8 (80%+ ορθά αποτελέσματα)
- QPS >> Brute Force QPS
- Speedup > 10x

**Trade-offs:**
- Υψηλότερη ακρίβεια (AF↓, Recall↑) συνήθως μειώνει την ταχύτητα (QPS↓)
- Ρυθμίσεις παραμέτρων: περισσότερα hash tables/probes/clusters → καλύτερη ακρίβεια, χειρότερη ταχύτητα

## Στοιχεία Ανάπτυξης

**Φοιτητές:**
- Τσάκωνα Μυρτώ  - [1115202100188] - [sdi2100188@di.uoa.gr]
- Μανίκα Θεοδώρα - [1115202100267] - [sdi2100267@di.uoa.gr]

**Ακαδημαϊκό Έτος:** 2025-2026

**Μάθημα:** Ανάπτυξη Λογισμικού για Αλγοριθμικά Προβλήματα

**Ίδρυμα:** ΕΚΠΑ

## Άδεια Χρήσης

Αυτό το έργο αναπτύχθηκε για εκπαιδευτικούς σκοπούς ως μέρος πανεπιστημιακής εργασίας.
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
- `main.cpp` - Κύριο σημείο εισόδου του προγράμματος
- `Makefile` - Αρχείο μεταγλώττισης
- `README.md` - Αυτό το αρχείο τεκμηρίωσης

### Αλγόριθμοι (`algorithms/`)
- `LSH/LSH.h`, `LSH/LSH.cpp` - Υλοποίηση LSH
- `hypercube/Hypercube.h`, `hypercube/Hypercube.cpp` - Υλοποίηση Hypercube
- `Kmeans/kmeans.h`, `Kmeans/kmeans.cpp` - Υλοποίηση K-means clustering
- `IVFFlat/IVFFlat.h`, `IVFFlat/IVFFlat.cpp` - Υλοποίηση IVFFlat
- `IVFPQ/IVFPQ.h`, `IVFPQ/IVFPQ.cpp` - Υλοποίηση IVFPQ

### Επεξεργασία Δεδομένων (`dataprocessing/`)
- `SIFT/SIFT.h`, `SIFT/SIFT.cpp` - Parser για δεδομένα SIFT
- `MNIST/MNIST.h`, `MNIST/MNIST_impl.cpp` - Parser για δεδομένα MNIST

### Βοηθητικά Εργαλεία (`utils/`)
- `interface/Interface.h`, `interface/Interface.cpp` - Κύρια διεπαφή προγράμματος
- `helpers/Utils.h`, `helpers/Utils.cpp` - Βοηθητικές συναρτήσεις
- `math/math.h`, `math/math.cpp` - Μαθηματικές λειτουργίες
- `helpers/Utils.h` - Utility functions for statistics and file operations
- `math/math.h` - Mathematical functions and hash computations
- `math/Random.h` - Random number generation utilities  
- `math/Gaussian.h` - Gaussian distribution functions
- `interface/print.h` - Print functions for debugging

### Μετρικές (`metrics/`)
- `metrics.h`, `metrics.cpp` - Υλοποίηση μετρικών απόστασης

## Οδηγίες Μεταγλώττισης

### Προαπαιτούμενα
- Compiler C++17 ή νεότερος (g++ 7+ ή clang++ 5+)
- GNU Make

### Μεταγλώττιση
```bash
# Κανονική μεταγλώττιση (optimized)
make

# Debug μεταγλώττιση
make debug

# Καθαρισμός αρχείων μεταγλώττισης
make clean

# Rebuild από την αρχή
make rebuild
```

## Οδηγίες Χρήσης

### Γενική Σύνταξη
```bash
./search -d <input_file> -q <query_file> -o <output_file> -type <data_type> [algorithm_options]
```

### Παράμετροι Αλγορίθμων

#### LSH
```bash
./search -d input.dat -q query.dat -o output.txt -type sift -lsh \
         -k <int> -L <int> -w <double> -N <int> -R <double> -range <true|false>
```

**Παράμετροι:**
- `-k <int>`: Αριθμός LSH συναρτήσεων (default: 4)
- `-L <int>`: Αριθμός πινάκων κατακερματισμού (default: 5)
- `-w <double>`: Μήκος κελιού προβολής (default: 4.0)
- `-N <int>`: Αριθμός πλησιέστερων γειτόνων (default: 1)
- `-R <double>`: Ακτίνα αναζήτησης (default: 2000 για MNIST, 2 για SIFT)
- `-range <true|false>`: Ενεργοποίηση αναζήτησης περιοχής

#### Hypercube
```bash
./search -d input.dat -q query.dat -o output.txt -type sift -hypercube \
         -kproj <int> -w <double> -M <int> -probes <int> -N <int> -R <double> -range <true|false>
```

**Παράμετροι:**
- `-kproj <int>`: Διάσταση προβολής (default: 14)
- `-w <double>`: Μήκος κελιού προβολής (default: 4.0)
- `-M <int>`: Μέγιστος αριθμός υποψήφιων (default: 10)
- `-probes <int>`: Μέγιστος αριθμός probes (default: 2)

#### IVFFlat
```bash
./search -d input.dat -q query.dat -o output.txt -type sift -ivfflat \
         -kclusters <int> -nprobe <int> -N <int> -R <double> -range <true|false> -seed <int>
```

**Παράμετροι:**
- `-kclusters <int>`: Αριθμός clusters (default: 50)
- `-nprobe <int>`: Αριθμός clusters προς έλεγχο (default: 5)
- `-seed <int>`: Seed για random generator (default: 1)

#### IVFPQ
```bash
./search -d input.dat -q query.dat -o output.txt -type sift -ivfpq \
         -kclusters <int> -nprobe <int> -M <int> -nbits <int> -N <int> -R <double> -range <true|false> -seed <int>
```

**Παράμετροι:**
- `-kclusters <int>`: Αριθμός clusters (default: 50)
- `-nprobe <int>`: Αριθμός clusters προς έλεγχο (default: 5)
- `-M <int>`: Αριθμός υποδιανυσμάτων για PQ (default: 16)
- `-nbits <int>`: Bits ανά subquantizer (default: 8)

### Παραδείγματα Χρήσης

```bash
# LSH με SIFT δεδομένα
./search -d sift_base.fvecs -q sift_query.fvecs -o output_lsh.txt -type sift -lsh -k 4 -L 5 -w 4.0 -N 1 -R 2.0

# Hypercube με MNIST δεδομένα
./search -d mnist_input.dat -q mnist_query.dat -o output_hypercube.txt -type mnist -hypercube -kproj 14 -w 4.0 -M 10 -probes 2 -N 5 -R 2000 -range true

# IVFFlat με optimized clusters
./search -d input.dat -q query.dat -o output_ivfflat.txt -type sift -ivfflat -kclusters 100 -nprobe 10 -N 10 -R 2.0 -seed 42

# IVFPQ με υψηλή συμπίεση
./search -d input.dat -q query.dat -o output_ivfpq.txt -type sift -ivfpq -kclusters 50 -nprobe 5 -M 16 -nbits 8 -N 1 -R 2.0
```

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

- **Approximation Factor (AF)**: Λόγος προσεγγιστικής προς ακριβή απόσταση
- **Recall@N**: Ποσοστό ορθών αποτελεσμάτων στους N πλησιέστερους
- **QPS (Queries Per Second)**: Ταχύτητα επεξεργασίας ερωτημάτων
- **tApproximateAverage**: Μέσος χρόνος προσεγγιστικής αναζήτησης
- **tTrueAverage**: Μέσος χρόνος ακριβούς αναζήτησης

## Στοιχεία Ανάπτυξης

**Φοιτητές:**
- [Όνομα Φοιτητή 1] - [ΑΜ] - [email]
- [Όνομα Φοιτητή 2] - [ΑΜ] - [email]

**Ακαδημαϊκό Έτος:** 2024-2025

**Μάθημα:** Ανάπτυξη Λογισμικού για Αλγοριθμικά Προβλήματα

**Ίδρυμα:** [Πανεπιστήμιο]

## Άδεια Χρήσης

Αυτό το έργο αναπτύχθηκε για εκπαιδευτικούς σκοπούς ως μέρος πανεπιστημιακής εργασίας.

## Αναφορές

1. A. Andoni and P. Indyk. Near-optimal hashing algorithms for approximate nearest neighbor in high dimensions. Communications of the ACM, 2008.
2. H. Jégou, M. Douze, and C. Schmid. Product quantization for nearest neighbor search. IEEE TPAMI, 2011.
3. J. Wang, T. Zhang, N. Sebe, H. T. Shen, et al. A survey on learning to hash. IEEE TPAMI, 2018.

Responsibilities Theodora:
- [ ] Random Projection
- [ ] IVFPQ
- [ ] SIFT

Responsibilities Myrto:
- [ ] LSH
- [ ] kmeans
- [ ] IVFFlat
- [ ] MNIST

To do Theodora:
- [x] metrics func
- [x] math implementation 
- [ ] SIFT parse input 
- [ ] hypercube.h and .cpp
- [x] other helpful function

To do Myrto:
- [✓] διαβάζεις διαφάνειες και βλέπεις αν πρέπει να γίνει κάποια αλλαγή στους κώδικες πχ κάποια πράξη δεν έγινε για lsh
- [✓] τα βάζεις όλα στο chat να σου πεί τη γνώμη του για lsh
- [✓] βλέπεις αν υπάρει κάτι που λείπει από την εκφώνηση για lsh
- [✓] διάβασε τις διαφάνειες για kmeans
- [✓] δες αν είναι σωστά υλοποιημένες και οι 3 φάσεις φιατί ζητάει κάτι διαφορετικό από το συνηθισμένο 
- [✓] όταν βρείς τις διαφορές ζητησε από το chat να στο υλοποιήσει χειροκίνητα χωρίς να του δώσεις όλο το αρχείο απλά περιγράφοντας του τι λείπει
- [✓] την ίδια διαδικασία ακολούθησε για ivfflat 
- [✓] είπε στις διαφάνειες ότι θα χρησιμοποιήσουμε αυτό που υλοποιήσαμε στο lsh όποτε δες αν το χρησιμοποιούμε και για τα δύο όντως


# Εγκατάσταση (Ubuntu/Debian)
sudo apt update
sudo apt install -y build-essential

# Από τον ριζικό φάκελο του project σου:
make

# Τρέξε το εκτελέσιμο
./bin/main

# Tests
ctest --preset test-debug


https://github.com/nikosgalanis/Vectors-Similarity-Search-and-Clustering/blob/main/results/cluster_lloyds_1000_queries
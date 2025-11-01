Looking at your benchmark results, here are the major problems:

🔴 Critical Issues:
1. Hypercube has TERRIBLE approximation factors (23-28x)
SIFT: AF = 23.2 and 28.6 (should be ~1.0-2.0)
MNIST: AF = 28.5 and 26.8 (should be ~1.0-2.0)
Recall = 0% - finding ZERO correct nearest neighbors!
Cause: The Cauchy distribution change we made is likely making things worse, OR the w parameter computation is still wrong for Hypercube.

2. LSH has poor approximation factors (2.0-2.2x)
Both datasets: AF = 2.0-2.2
Recall = 0% - finding ZERO correct nearest neighbors!
Cause: The ID filtering (if (candidate_id != query_id) continue;) is too strict - it's filtering out ALL candidates, even the correct ones.

3. IVFPQ MNIST crashed
Last row: All zeros (0.000000) for MeanApproximationFactor, Recall, QueriesPerSecond, MeanQueryTimeMS
Parameters: M=32, N=10, kclusters=100
Cause: Likely crashed or timed out during execution.

4. IVFFlat has low recall (50-60%)
Should be higher with those parameters
📊 What's Working:
✅ IVFFlat: Best approximation factors (0.93-0.96) but recall could be better
✅ IVFPQ: Decent recall (60-72%) but high AF (3.7-4.6)

🔧 Immediate Fixes Needed:
Fix 1: Disable ID filtering in LSH (like the legacy code)
The legacy LSH (lsh (1).cpp) has this line commented out:

Your current LSH has it enabled, which is filtering out too many candidates.

Fix 2: Revert Hypercube to use Gaussian (not Cauchy)
The Cauchy distribution for Manhattan distance might not be working correctly. The legacy code uses Gaussian with Manhattan distance and it worked.

Fix 3: Check if w is actually being used correctly
The results show w=4 and w=10 in the parameters, but we computed w=672624 for MNIST. Something's wrong - either:

The computed w isn't being used
The CSV is printing the wrong w value
The hash functions aren't using the updated w
Let me check what's actually happening:

The w is being computed. The CSV might just be printing the CLI parameter, not the actual one used. But the real issue is LSH and Hypercube are both finding ZERO correct neighbors (recall=0%).

Root Cause Analysis:
Your Approximation Factors are calculated from WRONG neighbors because recall is 0%. When recall=0%, it means:

LSH/Hypercube are returning neighbors
But NONE of them are in the true top-K
So the AF is the ratio of (distance to wrong neighbor) / (distance to true neighbor)
This explains the high AF values
The Fix Priority:
HIGHEST: Disable ID filtering in LSH → will immediately improve recall
HIGH: Revert Hypercube Cauchy back to Gaussian
MEDIUM: Verify w is actually being passed correctly to hash functions
Would you like me to implement these fixes?
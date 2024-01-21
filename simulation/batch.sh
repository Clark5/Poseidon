#!/bin/bash

# python run.py --cc poseidon --trace trace_multi_hop_congestion_small --bw 100 --topo topo_tworacks --poseidon_m 0.25 --poseidon_min_rate 1.0 > ./results/data/poseidononly_0.25_1.0.txt
# python run.py --cc poseidon --trace trace_multi_hop_congestion_small --bw 100 --topo topo_tworacks --poseidon_m 0.25 --poseidon_min_rate 1.0 --enable_routopia 1 > ./results/data/routopia_0.25_1.0.txt

# python run.py --cc poseidon --trace trace_under_utilization_6 --bw 100 --topo topo_3racks_origin --poseidon_m 0.25 --poseidon_min_rate 1.0 > ./results/data/poseidon_underutilization6_0.25_1.0.txt
# python run.py --cc poseidon --trace trace_under_utilization_6 --bw 100 --topo topo_3racks_origin --poseidon_m 0.25 --poseidon_min_rate 1.0 --enable_routopia 1 > ./results/data/routopia_underutilization6_0.25_1.0.txt

# python run.py --cc poseidon --trace trace_imbalance --bw 100 --topo topo_3racks_origin --poseidon_m 0.25 --poseidon_min_rate 1.0 > ./results/data/poseidon_imbalance_failure_0.25_1.0.txt
# python run.py --cc poseidon --trace trace_imbalance --bw 100 --topo topo_3racks_origin --poseidon_m 0.25 --poseidon_min_rate 1.0 --enable_routopia 1 > ./results/data/routopia_imbalance_failure_0.25_1.0_100deter.txt

python run.py --cc poseidon --trace trace_large_scale_websearch --bw 100 --topo fat --poseidon_m 0.25 --poseidon_min_rate 1.0 > ./results/data/poseidon_websearch_1ultimate.txt
python run.py --cc poseidon --trace trace_large_scale_websearch --bw 100 --topo fat --poseidon_m 0.25 --poseidon_min_rate 1.0 --enable_routopia 1 > ./results/data/routopia_websearch_1ultimate.txt

# python run.py --cc poseidon --trace trace_jupiter --bw 100 --topo topo_jupiter --poseidon_m 0.25 --poseidon_min_rate 1.0 > ./results/data/poseidon_jupiter_0.25_1.0.txt
# python run.py --cc poseidon --trace trace_jupiter --bw 100 --topo topo_jupiter --poseidon_m 0.25 --poseidon_min_rate 1.0 --enable_routopia 1 > ./results/data/routopia_jupiter_0.25_1.0.txt

# python run.py --cc poseidon --trace trace_large_scale_websearch_6 --bw 100 --topo fat --poseidon_m 0.25 --poseidon_min_rate 1.0 > ./results/data/poseidon_websearch6_0.25_1.0.txt
# python run.py --cc poseidon --trace trace_large_scale_websearch_6 --bw 100 --topo fat --poseidon_m 0.25 --poseidon_min_rate 1.0 --enable_routopia 1 > ./results/data/routopia_websearch6_0.25_1.0.txt

# python run.py --cc poseidon --trace trace_under_utilization_ablation --bw 100 --topo topo_3racks_origin --poseidon_m 0.25 --poseidon_min_rate 1.0 --enable_routopia 1 > ./results/data/routopia_underutilization_0.25_1.0_origin_ablation.txt

cd results
python draw_routopia.py
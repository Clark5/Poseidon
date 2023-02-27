#!/bin/bash

python run.py --cc poseidon --trace trace_multi_hop_congestion_small --bw 100 --topo topo_racks --poseidon_m 0.25 --poseidon_min_rate 1.0 > ./results/data/poseidon_0.25_1.0.txt

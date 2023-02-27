# Poseidon NS-3 simulator
This is a ns-3 simulator of paper [Poseidon: Efficient, Robust, and Practical Datacenter CC via Deployable INT](https://weitaowang.site/papers/poseidon.pdf) from NSDI 2023. It also includes the implementation of HPCC, DCQCN, TIMELY, DCTCP, PFC, ECN and Broadcom shared buffer switch. Our code is based on the [simulator for PINT and HPCC](https://github.com/alibaba-edu/High-Precision-Congestion-Control).

It is based on NS-3 version 3.17.

## Quick Start

### Build
```
cd simulator
./waf configure
```

Please note if gcc version > 5, compilation will fail due to some ns3 code style.  If this what you encounter, please use:

`CC='gcc-5' CXX='g++-5' ./waf configure`

And you may need to use Python 2.7 for seamless installation. Note that Python only wraps the simulator, and the simulator is written in C++.

### Experiment config
Please see `mix/config.txt` for example. 

`mix/config_doc.txt` is a explanation of the example (texts in {..} are explanations).

### Run
We highly recommend you to use the `batch.sh` to run the simulation in batch and store the results in `./results/data/`. Besides, there are also two other ways to run the simulation.

The direct command to run is:
`./waf --run 'scratch/third mix/config.txt'`

We provide a `run.py` for automatically *generating config* and *running experiment*. Please `python run.py -h` for usage.
Example usage:
`python run.py --cc poseidon --trace trace_multi_hop_congestion_small --bw 100 --topo topo_racks --poseidon_m 0.25 --poseidon_min_rate 1.0 > ./results/data/poseidon_0.25_1.0.txt`

### Plot results
The `draw.py` will plot the essential results for Poseidon, including the evolving of rate, queue length, and mpd signal along with time. 

### Exmaples
```
sh batch.sh
python ./results/draw.py
```

## Files added/edited based on NS3
The major ones are listed here. There could be some files not listed here that are not important or not related to core logic.

`point-to-point/model/qbb-net-device.cc/h`: the net-device RDMA

`point-to-point/model/pause-header.cc/h`: the header of PFC packet

`point-to-point/model/cn-header.cc/h`: the header of CNP

`point-to-point/model/pint.cc/h`: the PINT encoding/decoding algorithms

`point-to-point/model/qbb-header.cc/h`: the header of ACK

`point-to-point/model/qbb-channel.cc/h`: the channel of qbb-net-device

`point-to-point/model/qbb-remote-channel.cc/h`

`point-to-point/model/rdma-driver.cc/h`: layer of assigning qp and manage multiple NICs

`point-to-point/model/rdma-queue-pair.cc/h`: queue pair

`point-to-point/model/rdma-hw.cc/h`: the core logic of congestion control

`point-to-point/model/switch-node.cc/h`: the node class for switch

`point-to-point/model/switch-mmu.cc/h`: the mmu module of switch

`network/utils/broadcom-egress-queue.cc/h`: the multi-queue implementation of a switch port

`network/utils/custom-header.cc/h`: a customized header class for speeding up header parsing

`network/utils/int-header.cc/h`: the header of INT

`applications/model/rdma-client.cc/h`: the application of generating RDMA traffic

## Notes on other schemes
The HPCC and PINT implementations are the same with their simulator [repo](https://github.com/alibaba-edu/High-Precision-Congestion-Control).

The DCQCN implementation is based on [Mellanox's implementation on CX4 and newer version](https://community.mellanox.com/s/article/dcqcn-parameters), which is slightly different from the DCQCN paper version.

The TIMELY implementation is based on our own understanding of the TIMELY paper. We believe we correctly implemented it. We use the parameters in the TIMELY paper. For parameters whose settings are absent in the TIMELY paper, we get from [this paper (footnote 4)](https://www.microsoft.com/en-us/research/wp-content/uploads/2016/09/ecndelay-conext16.pdf).

The DCTCP implementation is a version that we envision DCTCP will be implemented in hardware. It starts at line rate (not slow start) which we believe is necessary in future high-speed network. It also does not delay ACK, because delayed ACk is for saving software overhead. These settings are consistent with other schemes.

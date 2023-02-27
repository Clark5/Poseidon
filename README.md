# Poseidon Algorithm
This is a ns-3 simulator of paper [Poseidon: Efficient, Robust, and Practical Datacenter CC via Deployable INT](https://weitaowang.site/papers/poseidon.pdf) from NSDI 2023. It also includes the implementation of HPCC, DCQCN, TIMELY, DCTCP, PFC, ECN and Broadcom shared buffer switch. Our code is based on the [simulator for PINT and HPCC](https://github.com/alibaba-edu/High-Precision-Congestion-Control).

## Installation
Please refer to the [README.md](simulation/README.md) under simulation folder for more details.

## Disclaimer
This repo is under active development by [Weitao Wang](weitaowang.site/about), any questions are welcome through the repo issues or emails.

In Poseidon paper, we use a different simulator, which is a Google internal packet-level simulator.
Poseidon paper also constructes a testbed with Tofino2 switches. For implementation details of the testbed, please also contact Weitao (wtwang@rice.edu).

## Citing Poseidon
If you compare with Poseidon or use the Poseidon algorithm in a scientific publication, please cite the following paper:

Wang, Weitao, et al. "Poseidon: Efficient, Robust, and Practical Datacenter CC via Deployable INT", NSDI, 2023.

## Contribution
Community contributions are more than welcome, whether it be to fix bugs or to add new features. Feel free to open GitHub issues about your contribution ideas, and I will review them.

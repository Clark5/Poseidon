import plotters
import os
from collections import deque

if __name__ == "__main__":
    directory = "./data/"
    fig_directory = "./figures/"
    flow_mapping = {"0b000001": 0, 

                    "0b002001": 1, 
                    "0b003001": 2, 

                    "0b000101": 5, 
                    "0b000201": 6,

                    "0b004001": 3,
                    "0b005001": 4,
                    "0b006001": 7,
                    "0b007001": 8,
                    "0b008001": 9,
                    "0b009001": 10,
                    "0b00a001": 11,

                    "0b000301": 12,
                    "0b000401": 13,
                    "0b000501": 14,
                    "0b000601": 15,
                    "0b000701": 16,
                    "0b000801": 17,
                    "0b000901": 18,
                    }
    for file in os.listdir(directory):
        if file.endswith(".txt"):
            filename = file[:-4]
            print(filename)
            x_axis = [[] for i in range(19)]
            y_axis = [[] for i in range(19)]
            x_mpd = [[] for i in range(19)]
            y_mpd = [[] for i in range(19)]
            signals_x = [[] for i in range(9)]
            signals_y = [[] for i in range(9)]
            smooth_x = [[] for i in range(9)]
            smooth_y = [[] for i in range(9)]
            with open("./data/"+filename+".txt", "r") as raw_file:
                q = [deque() for i in range(19)]
                avg = [0 for i in range(19)]
                for line in raw_file:
                    items = line.split()
                    if (len(items) < 4):
                        continue
                    if items[0] == "Rate:":
                        time = int(items[1])
                        srcip = items[2]
                        rate = float(items[3])
                        if srcip in flow_mapping:
                            x_axis[flow_mapping[srcip]].append(time / 1e6 - 2000)
                            y_axis[flow_mapping[srcip]].append(rate)
                    elif items[0] == "Port:":
                        time = int(items[1])
                        port = int(items[2])
                        utilization = float(items[3])
                        signals_x[port].append(time / 1e9)
                        signals_y[port].append(utilization)
                    elif items[0] == "SPort:":
                        time = int(items[1])
                        port = int(items[2])
                        utilization = float(items[3])
                        smooth_x[port].append(time / 1e9)
                        smooth_y[port].append(utilization)
                    elif items[0] == "SMPD:":
                        time = int(items[1])
                        srcip = items[2]
                        mpd = float(items[3])
                        if srcip in flow_mapping:
                            x_mpd[flow_mapping[srcip]].append(time / 1e6 - 2000)
                            y_mpd[flow_mapping[srcip]].append(mpd)
                            q[flow_mapping[srcip]].append(mpd)
                            avg[flow_mapping[srcip]] += mpd
                            if len(q[flow_mapping[srcip]]) == 51:
                                oldest = q[flow_mapping[srcip]].popleft()
                                avg[flow_mapping[srcip]] -= oldest
            signals_y[2] = []
            signals_x[2] = []
            smooth_y[2] = []
            smooth_x[2] = []
            plotters.draw("line", y_axis, "./rate/"+filename+".png", x_axis=x_axis, 
                        xylabels=["Time (ms)", "Flow Rate (Gbps)"],
                        markersize=0, linewidth=1.5,
                        ylim=(-5, 105),
                        figure_size=(7, 5),
                        legends=["Flow 0", "Flow 1", "Flow 2", "Flow 3"]) # ["Flow 0", "Flow 1", "Flow 2", "Flow 3", "Flow 4", "Flow 5", "Flow 6", "Flow 7", "Flow 8"]
            plotters.draw("line", y_mpd, "./mpd/"+filename+".png", x_axis=x_mpd, 
                        xylabels=["Time (ms)", "MPD (%)"],
                        markersize=0, linewidth=1,
                        # ylim=(-5, 105),
                        figure_size=(7, 5),
                        legends=[k for k in range(5)])
            plotters.draw("line", signals_y, "./signal/"+filename+"_signals.png", x_axis=signals_x, 
                        xylabels=["Time (ms)", "Utilization"],
                        markersize=0, linewidth=1,
                        # ylim=(-5, 105),
                        figure_size=(7, 5),
                        legends=[k for k in range(9)]) 
            plotters.draw("line", smooth_y, "./smoothed_signal/"+filename+"_smooth.png", x_axis=smooth_x, 
                        xylabels=["Time (ms)", "Utilization"],
                        markersize=0, linewidth=1,
                        # ylim=(-5, 105),
                        figure_size=(7, 5),
                        legends=[k for k in range(9)])
    
    flow_mapping = {"0b000001": 0, 

                    "0b002001": 1, 
                    "0b003001": 2, 

                    "0b000101": 3, 
                    "0b000201": 4,

                    "0b004001": 5,
                    "0b005001": 6,
                    "0b006001": 7,
                    "0b007001": 8,
                    "0b008001": 9,
                    "0b009001": 10,
                    "0b00a001": 11,

                    "0b000301": 12,
                    "0b000401": 13,
                    "0b000501": 14,
                    "0b000601": 15,
                    "0b000701": 16,
                    "0b000801": 17,
                    "0b000901": 18,
                    }
    file_list = ["multi_trinity_95_05_1RTT_0.02_1.0_20RTT_4.0_10.0",
                 "multi_poseidon_95_05_1RTT_0.25_1.0_20RTT_4.0_10.0",
                 "multi_trinity_95_05_1RTT_0.02_1.0_20RTT_0.0_0.0",
                 "1000_95_05_1RTT_0.02_1.0_20RTT_4.0_1.0",
                 "without_target_95_05_1RTT_0.02_1.0_20RTT_0.0_0.0",
                 "without_MIMD_95_05_1RTT_0.02_1.0_20RTT_0.0_0.0"]
    for file in os.listdir(directory):
        if file[:-4]+".png" in os.listdir("./rate/"):
            if file[:-4] not in file_list:
                continue
        if file.endswith(".txt"):
            filename = file[:-4]
            print(filename)
            x_axis = [[] for i in range(19)]
            y_axis = [[] for i in range(19)]
            x_mpd = [[] for i in range(19)]
            y_mpd = [[] for i in range(19)]
            signals_x = [[] for i in range(9)]
            signals_y = [[] for i in range(9)]
            smooth_x = [[] for i in range(9)]
            smooth_y = [[] for i in range(9)]
            with open("./data/"+filename+".txt", "r") as raw_file:
                q = [deque() for i in range(19)]
                avg = [0 for i in range(19)]
                for line in raw_file:
                    items = line.split()
                    if (len(items) < 4):
                        continue
                    if items[0] == "Rate:":
                        time = int(items[1])
                        if time > 2050000000:
                            break
                        srcip = items[2]
                        rate = float(items[3])
                        if srcip in flow_mapping:
                            x_axis[flow_mapping[srcip]].append(time / 1e6 - 2000)
                            y_axis[flow_mapping[srcip]].append(rate)
                    elif items[0] == "Port:":
                        time = int(items[1])
                        port = int(items[2])
                        utilization = float(items[3])
                        signals_x[port].append(time / 1e9)
                        signals_y[port].append(utilization)
                    elif items[0] == "SPort:":
                        time = int(items[1])
                        port = int(items[2])
                        utilization = float(items[3])
                        smooth_x[port].append(time / 1e9)
                        smooth_y[port].append(utilization)
                    elif items[0] == "SMPD:":
                        time = int(items[1])
                        srcip = items[2]
                        mpd = float(items[3])
                        if srcip in flow_mapping:
                            x_mpd[flow_mapping[srcip]].append(time / 1e6 - 2000)
                            y_mpd[flow_mapping[srcip]].append(mpd)
                            q[flow_mapping[srcip]].append(mpd)
                            avg[flow_mapping[srcip]] += mpd
                            if len(q[flow_mapping[srcip]]) == 51:
                                oldest = q[flow_mapping[srcip]].popleft()
                                avg[flow_mapping[srcip]] -= oldest
            signals_y[2] = []
            signals_x[2] = []
            smooth_y[2] = []
            smooth_x[2] = []
            plotters.draw("line", y_axis, "./rate/"+filename+".png", x_axis=x_axis, 
                        xylabels=["Time (ms)", "Flow Rate (Gbps)"],
                        markersize=0, linewidth=1.5,
                        ylim=(-5, 105),
                        figure_size=(7, 5),
                        legends=["Flow 0", "Flow 1", "Flow 2", "Flow 3", "Flow 4", "Flow 5", "Flow 6", "Flow 7", "Flow 8"],
                        ncol=2)
            plotters.draw("line", y_mpd, "./mpd/"+filename+".png", x_axis=x_mpd, 
                        xylabels=["Time (ms)", "MPD (%)"],
                        markersize=0, linewidth=1,
                        # ylim=(-5, 105),
                        figure_size=(7, 5),
                        legends=[k for k in range(5)])
            plotters.draw("line", signals_y, "./signal/"+filename+"_signals.png", x_axis=signals_x, 
                        xylabels=["Time (ms)", "Utilization"],
                        markersize=0, linewidth=1,
                        # ylim=(-5, 105),
                        figure_size=(7, 5),
                        legends=[k for k in range(9)]) 
            plotters.draw("line", smooth_y, "./smoothed_signal/"+filename+"_smooth.png", x_axis=smooth_x, 
                        xylabels=["Time (ms)", "Utilization"],
                        markersize=0, linewidth=1,
                        # ylim=(-5, 105),
                        figure_size=(7, 5),
                        legends=[k for k in range(9)])
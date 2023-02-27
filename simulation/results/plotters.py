import numpy as np
import matplotlib
matplotlib.use('Agg')
from matplotlib import pyplot as plt
from matplotlib import cm 
from matplotlib import axes
from matplotlib import rcParams
from matplotlib import hatch
from matplotlib.backends.backend_pdf import PdfPages
#from mpl_toolkits.mplot3d import Axes3D

# color palette from seaborn 
color_pastel =      [u'#a1c9f4', u'#ffb482', u'#8de5a1', u'#ff9f9b', u'#d0bbff', u'#debb9b', u'#fab0e4', u'#cfcfcf', u'#fffea3', u'#b9f2f0'] * 10
color_muted =       [u'#4878d0', u'#ee854a', u'#6acc64', u'#d65f5f', u'#956cb4', u'#8c613c', u'#dc7ec0', u'#797979', u'#d5bb67', u'#82c6e2'] * 10
color_deep =        [u'#4c72b0', u'#dd8452', u'#55a868', u'#c44e52', u'#8172b3', u'#937860', u'#da8bc3', u'#8c8c8c', u'#ccb974', u'#64b5cd'] * 10
color_bright =      [u'#023eff', u'#ff7c00', u'#1ac938', u'#e8000b', u'#8b2be2', u'#9f4800', u'#f14cc1', u'#a3a3a3', u'#ffc400', u'#00d7ff'] * 10
color_colorblind =  [u'#0173b2', u'#de8f05', u'#029e73', u'#d55e00', u'#cc78bc', u'#ca9161', u'#fbafe4', u'#949494', u'#ece133', u'#56b4e9'] * 10
color_dark =        [u'#001c7f', u'#b1400d', u'#12711c', u'#8c0800', u'#591e71', u'#592f0d', u'#a23582', u'#3c3c3c', u'#b8850a', u'#006374'] * 10
color_default = color_deep

# lists
marker_list =       ['o', 'v', 'p', '+', 'x', 's', '.', 'P', '1', '*'] * 10
hatch_list =        ['.', '/', 'x', '+', '\\', '*', 'O','o', '-', '|'] * 10
linestyle_list =    ['-'] * 40  # ['-', ':','-.','--', '-', ':','-.','--'] * 10


def draw(   # general settings
            figure_type, dataset, filename, 
            # x axis data (optional)
            x_axis = [], 
            # figure layout
            figure_size = (6,5),
            spines = [False, True, True, False],
            # main body styles
            linewidth = 1,
            markersize = 1,
            # x/y axises styles
            xylabels = ["x label", "y label"], 
            xylabels_fontsizes = [18, 18],
            xscale = "",
            yscale = "",
            xlim = (),
            ylim = (),
            xticks = [],
            yticks = [],
            xticks_labels = [],
            yticks_labels = [],
            xticks_rotation = 0,
            yticks_rotation = 0,
            xtick_labelsize = 18,
            ytick_labelsize = 18,
            # legends styles
            legends = [],
            legend_fontsize = 18,
            legend_location = "best",
            legend_frame_alpha = 0.6, 
            # grid styles
            xgrid = True,
            ygrid = True,
            xgrid_style = 'dashed',
            ygrid_style = 'dashed',
            # special settings for histograms
            bar_total_width = 0.6,
            bar_group_size = 1,
            ncol=1):
    """
    Plot the figures of different types.
    """

    def histogram_plotter():   
        """
        histogram plotter
        """
        # plot bars 
        label_size = len(dataset)
        x_size = len(dataset[0])
        if (label_size > 1):
            for i in range(label_size):
                half_width = bar_total_width/2/label_size
                x_axis = [(j-half_width*(label_size-1)+2*half_width*i) for j in range(x_size)]
                print x_axis
                plt.bar(x_axis, dataset[i], width = 2*half_width, color=color_pastel[i], hatch=hatch_list[i], edgecolor='black', lw=linewidth)
        else:
            for i in range(len(dataset[0])):
                plt.bar(i, dataset[0][i], width = bar_total_width, color=color_pastel[i], hatch=hatch_list[i], edgecolor='black', lw=linewidth)
    
    def accumulated_histogram_plotter():
        """
        accumulated histogram plotter
        """      
        # data processing
        layer_number = len(dataset)
        assert(len(dataset[0]) % bar_group_size == 0)
        x_size = len(dataset[0]) / bar_group_size
        bottom = [0 for i in range(x_size*bar_group_size)]
        # plot bars
        for k in range(layer_number):
            # calculate x axis 
            half_width = bar_total_width/2/bar_group_size
            x_axis = []
            for i in range(x_size):
                for j in range(bar_group_size):
                    x_axis.append(i-half_width*(bar_group_size-1)+2*half_width*j)
            # plot 
            plt.bar(x_axis, dataset[k], bottom=bottom, width = 2*half_width, color=color_pastel[k], hatch=hatch_list[k], edgecolor='black', lw=linewidth) 
            for i in range(x_size*bar_group_size):
                bottom[i] += dataset[k][i]

    def linechart_plotter(x_axis):
        """
        linechart plotter
        """
        # data processing
        if x_axis == []:
            x_axis = [i for i in range(len(dataset[0]))]
        # plot each line
        for i in range(len(dataset)):
            plt.plot(x_axis[i], dataset[i], linewidth=linewidth, linestyle=linestyle_list[i], color=color_default[i], marker=marker_list[i], markersize=markersize)

    def cdf_plotter():
        """
        cdf plotter
        """
        # data processing: sort 
        for i in range(len(dataset)):
            dataset[i].sort()
            data_len = len(dataset[i])
            print(dataset[i][data_len/2], dataset[i][-1])
        # plot each cdf 
        idx = -1
        for row in dataset:
            idx += 1
            xyaxis = [[],[]]
            length = len(row)
            for i in range(length):
                xyaxis[0].append(float(row[i]))
                xyaxis[1].append(float(i+1)/length)
            plt.plot(xyaxis[0],xyaxis[1], linewidth=linewidth, linestyle=linestyle_list[idx%10], color=color_default[idx%10], marker=marker_list[idx%10], markersize=markersize)
    
    def scatter_plotter(x_axis):
        """
        scatter plotter
        """
        # data processing
        if x_axis == []:
            x_axis = [i for i in range(len(dataset[0]))]
        # plot scatter
        for i in range(len(dataset)):
            ax.scatter(x_axis, dataset[i], s=markersize, marker=marker_list[i], linewidths=linewidth)

    def heatmap_plotter():
        """
        heatmap plotter (default: gray)
        """
        # data processing
        vmax=dataset[0][0]
        vmin=dataset[0][0]
        for i in range(len(dataset)):
            for j in range(len(dataset)):
                if dataset[i][j] > vmax:
                    vmax = dataset[i][j]
                if dataset[i][j] < vmin:
                    vmin = dataset[i][j]
        # plot heatmap
        cmap=cm.get_cmap('Greys', 1000)
        map=ax.imshow(dataset, interpolation='nearest', cmap=cmap, aspect='auto', vmin=vmin, vmax=vmax)
        plt.colorbar(mappable=map,cax=None,ax=None,shrink=0.6)

    # global styles setting
    plt.figure(figsize=figure_size)
    plt.rcParams['savefig.dpi'] = 300
    plt.rcParams['figure.dpi'] = 300
    rcParams['xtick.labelsize'] = xtick_labelsize
    rcParams['ytick.labelsize'] = ytick_labelsize
    rcParams["legend.framealpha"] = legend_frame_alpha
    rcParams['hatch.color'] = 'red'
    ax = plt.subplot(1,1,1)
    ax.set_axisbelow(True)
    ax.spines['top'].set_visible(spines[0])
    ax.spines['bottom'].set_visible(spines[1])
    ax.spines['left'].set_visible(spines[2])
    ax.spines['right'].set_visible(spines[3])
    
    # draw the figure
    if (figure_type == "histogram"): # grouped
        histogram_plotter()
        if xticks == []:
            xticks = [i for i in range(len(dataset[0])/bar_group_size)]
    elif (figure_type == "accumulated histogram"): # accumulated + grouped (parameter bar_group_size)
        accumulated_histogram_plotter()
        if xticks == []:
            xticks = [i for i in range(len(dataset[0])/bar_group_size)]
    elif (figure_type == "line"):
        linechart_plotter(x_axis)
    elif (figure_type == "cdf"):
        cdf_plotter()
    elif (figure_type == "scatter"):
        scatter_plotter(x_axis)
    elif (figure_type == "heatmap"):
        heatmap_plotter()
        if xticks == []:
            xticks = [i for i in range(len(dataset[0]))]
        if yticks == []:
            yticks = [i for i in range(len(dataset))]
    else:
        print ("Figure type does not exist.")
        return 1

    # styles setting
    if legends != []:
        plt.legend(legends,loc=legend_location, fontsize=legend_fontsize, ncol=ncol)
    plt.xlabel(xylabels[0], fontsize=xylabels_fontsizes[0])
    plt.ylabel(xylabels[1], fontsize=xylabels_fontsizes[1])
    if xlim != ():
        plt.xlim(xlim[0], xlim[1])
    if ylim != ():
        plt.ylim(ylim[0], ylim[1])
    if xticks != []:
        plt.xticks(xticks, xticks_labels, rotation=xticks_rotation)
    if yticks != []:
        plt.yticks(yticks, yticks_labels, rotation=yticks_rotation)
    if xscale != "":
        plt.xscale(xscale)
    if yscale != "":
        plt.yscale(yscale)
    if xgrid:
        plt.grid(axis='x', linestyle=xgrid_style)
    if ygrid:
        plt.grid(axis='y', linestyle=ygrid_style)

    # save the figure
    plt.tight_layout()
    plt.savefig(filename)
    plt.close()
    

# text = "Inter-Rack Traffic:\n" + str(int(inter_rack_sum))
# plt.text(30, 0.8, text, size=18, ha="center", va="center",bbox=dict(boxstyle="round",ec=(70/float(255),130/float(255),180/float(255)),fc=(135/float(255),206/float(255),250/float(255))))
    
# x = [0, 1, 2, 3, 4]
# y = [100, 97, 94, 89, 85]
# plt.fill_between(x, y, interpolate=True, color=color_default[i+1], alpha=0.3, hatch='/')


if __name__ == "__main__":
    pass


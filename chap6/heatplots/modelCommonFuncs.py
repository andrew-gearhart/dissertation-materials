from math import *
from mpl_toolkits.mplot3d import Axes3D
from matplotlib import cm
from matplotlib.ticker import LinearLocator, FormatStrFormatter
import matplotlib.pyplot as plt
import numpy as np

def clearall():
    """clear all globals"""
    for uniquevar in [var for var in globals().copy() if var[0] != "_" and var != 'clearall']:
        del globals()[uniquevar]

def quickHeatPlot(input):
    Xaxis = input['Xaxis']
    Yaxis = input['Yaxis']
    data = input['data']
    labels = input['labels']
    fig,ax = plt.subplots()
    Xaxis,Yaxis = np.meshgrid(Xaxis,Yaxis)
    graph = ax.pcolor(Xaxis,Yaxis,data)
    ax.ticklabel_format(style='sci', axis='x', scilimits=(0,0))
    ax.text(0.5, 1.08, labels['title'],
         horizontalalignment='center',
         fontsize=14,
         transform = ax.transAxes)
    ax.set_xlabel(labels['Xlabel'])
    ax.set_ylabel(labels['Ylabel'])
    ax.set_autoscaley_on(False)
    ax.set_ylim([Yaxis[0][0],Yaxis[-1][0]])
    ax.set_xlim([Xaxis[0][0],Xaxis[0][-1]])

    ax.scatter(input['newConstEx'],input['newY'],s=100,linewidth='3',facecolor='green')
    ax.scatter(input['newConstTx'],input['newY'],s=100,linewidth='3',facecolor='blue')

    cb = fig.colorbar(graph,ax=ax)

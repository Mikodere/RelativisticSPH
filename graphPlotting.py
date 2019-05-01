"""
Code to plot out animated 1D data from text file

@author: Gabriel Canet Espinosa
"""

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

den = np.loadtxt('densities.txt', delimiter=',')
pre = np.loadtxt('pressures.txt', delimiter=',')
vel = np.loadtxt('velocities.txt', delimiter=',')
pos = np.loadtxt('positions.txt', delimiter=',')
ene = np.loadtxt('energies.txt', delimiter=',')

fig = plt.figure()

def animate(i):
    x4 = pos[i, ::2]
    y4 = pos[i,1::2]

    x1 = den[i, ::2]
    y1 = den[i,1::2]
    plt.subplot(2,2,1)
    plt.subplot(2,2,1).clear()
    plt.plot(y4, y1, 'o', mfc='none', ms=2)

    x2 = pre[i, ::2]
    y2 = pre[i,1::2]
    plt.subplot(2,2,2)
    plt.subplot(2,2,2).clear()
    plt.plot(y4, y2, 'o', mfc='none', ms=2)

    x3 = vel[i, ::2]
    y3 = vel[i,1::2]
    plt.subplot(2,2,3)
    plt.subplot(2,2,3).clear()
    plt.plot(y4, y3, 'o', mfc='none', ms=2)
	
    x5 = ene[i, ::2]
    y5 = ene[i,1::2]
    plt.subplot(2,2,4)
    plt.subplot(2,2,4).clear()
    plt.plot(y4, y5, 'o', mfc='none', ms=2)
    

ani = animation.FuncAnimation(fig, animate, frames=len(den), interval=50)

plt.show()

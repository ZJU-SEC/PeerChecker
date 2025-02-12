from scipy.interpolate import make_interp_spline
import matplotlib.pyplot as plt
import numpy as np
import json

plt.figure()
plt.yscale('log',base=10) 

with open('AvgCount.json', 'r') as f:
    rawData = json.load(f)

rawData = dict(map(lambda kv: (int(kv[0]), kv[1]), rawData.items()))


Y1 = []
Y2 = []


X = np.array(list(rawData.keys()))

for _, value in rawData.items():
    Y1.append(value[1] / value[0])
    Y2.append(value[2] / value[0])

plt.plot(X, Y1, marker='.', color='#1f77b4', alpha=0.75)
plt.plot(X, Y2, marker='.', color='#d62728', alpha=0.75)

plt.legend(["all type dependency", "peer dependency"], loc='lower right')

plt.xlabel('Year')
plt.ylabel(' Average dependency number (log scale)')
plt.savefig('2.AvgCount.pdf', format='pdf', dpi=400)






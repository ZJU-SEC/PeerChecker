from scipy.interpolate import make_interp_spline
import matplotlib.pyplot as plt
import numpy as np
import json

plt.figure()

with open('LegMap.json', 'r') as f:
    rawData = json.load(f)
    
# print(sum(rawData.values()))

plt.yscale('log', base=10) 

rawData = dict(filter(lambda kv: (int(kv[0]) <= 1200), rawData.items()))
rawData = dict(map(lambda kv: (int(kv[0]), kv[1]), rawData.items()))

print(len(rawData))

X = np.array(list(rawData.keys()))
Y = list(rawData.values())

X_Y_Spline = make_interp_spline(X, Y)
X1 = np.linspace(X.min(), X.max(), 2060)
Y1 = X_Y_Spline(X1)
plt.bar(X1, Y1, width=4, facecolor='#1f77b4', edgecolor=None, alpha=0.2)


plt.ylim(bottom=10, top=10000)
plt.xlabel('Day(s)')
plt.ylabel('Version number')

plt.grid(axis='y')


plt.savefig('LegMap.pdf', format='pdf', dpi=400)

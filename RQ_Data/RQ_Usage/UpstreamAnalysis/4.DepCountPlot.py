from scipy.interpolate import make_interp_spline
import matplotlib.pyplot as plt
import numpy as np
import json

plt.figure()
plt.yscale('log',base=10) 

with open('DepCount.json', 'r') as f:
    data = json.load(f)
    
AllDepStatistic = data['AllDepStatistic']
PeerDepStatistic = data['PeerDepStatistic']

AllDepStatistic = dict(filter(lambda kv: (int(kv[0]) <= 100), AllDepStatistic.items()))
AllDepStatistic = dict(map(lambda kv: (int(kv[0]), kv[1]), AllDepStatistic.items()))


X = np.array(list(AllDepStatistic.keys()))
Y = np.array(list(AllDepStatistic.values()))

# 300 represents number of points to make between T.min and T.max
X_Y_Spline = make_interp_spline(X, Y)
X = np.linspace(X.min(), X.max(), 100)
Y = X_Y_Spline(X)

plt.bar(X, Y, width=1.0, facecolor='#1f77b4', edgecolor=None, alpha=0.75)

PeerDepStatistic = dict(filter(lambda kv: (int(kv[0]) <= 100), PeerDepStatistic.items()))
PeerDepStatistic = dict(map(lambda kv: (int(kv[0]), kv[1]), PeerDepStatistic.items()))


X = np.array(list(PeerDepStatistic.keys()))
Y = np.array(list(PeerDepStatistic.values()))

# 300 represents number of points to make between T.min and T.max
X_Y_Spline = make_interp_spline(X, Y)
X = np.linspace(X.min(), X.max(), 100)
Y = X_Y_Spline(X)

# plt.bar(X, Y, width=1.0, facecolor='#1f77b4', edgecolor='#1f77b4')
plt.bar(X, Y, width=1.0, facecolor='#d62728', edgecolor=None, alpha=0.75)
plt.xlabel('Quantity of dependency')
plt.ylabel('Version number (log scale)')

plt.legend(["all dependency", "peer dependency"], loc='upper right')

plt.xlim(left=0, right=100)
plt.savefig('4.DepCount.pdf', format='pdf', dpi=400)






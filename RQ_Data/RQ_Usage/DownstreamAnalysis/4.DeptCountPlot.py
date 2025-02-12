from scipy.interpolate import make_interp_spline
import matplotlib.pyplot as plt
import numpy as np
import json

plt.figure()
plt.yscale('log',base=10) 

with open('DeptCount.json', 'r') as f:
    data = json.load(f)
    
AllDeptStatistic = data['AllDeptStatistic']
PeerDeptStatistic = data['PeerDeptStatistic']

AllDeptStatistic = dict(filter(lambda kv: (int(kv[0]) <= 1000), AllDeptStatistic.items()))
AllDeptStatistic = dict(map(lambda kv: (int(kv[0]), kv[1]), AllDeptStatistic.items()))


X = np.array(list(AllDeptStatistic.keys()))
Y = np.array(list(AllDeptStatistic.values()))

X_Y_Spline = make_interp_spline(X, Y)
X = np.linspace(X.min(), X.max(), 1000)
Y = X_Y_Spline(X)

plt.bar(X, Y, width=1.0, facecolor='#1f77b4', edgecolor=None, alpha=0.75)

PeerDeptStatistic = dict(filter(lambda kv: (int(kv[0]) <= 1000), PeerDeptStatistic.items()))
PeerDeptStatistic = dict(map(lambda kv: (int(kv[0]), kv[1]), PeerDeptStatistic.items()))

X = np.array(list(PeerDeptStatistic.keys()))
Y = np.array(list(PeerDeptStatistic.values()))

X_Y_Spline = make_interp_spline(X, Y)
X = np.linspace(X.min(), X.max(), 1000)
Y = X_Y_Spline(X)

plt.bar(X, Y, width=1.0, facecolor='#d62728', edgecolor=None, alpha=0.75)
plt.xlabel('Quantity of dependent ')
plt.ylabel('Version number (log scale)')

plt.legend(["all dependents", "peer dependents"], loc='upper right')

plt.xlim(left=0, right=1000)
plt.ylim(bottom=10, top=1000000)
plt.savefig('4.DeptCount.pdf')






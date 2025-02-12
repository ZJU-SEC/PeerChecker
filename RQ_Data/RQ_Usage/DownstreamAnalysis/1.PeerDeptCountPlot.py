from scipy.interpolate import make_interp_spline
import matplotlib.pyplot as plt
import numpy as np
import json

plt.figure()

plt.yscale('log',base=10) 

with open('./PeerDeptCount.json', 'r') as f:
    rawData = json.load(f)
    

rawData = dict(filter(lambda kv: (int(kv[0]) <= 500), rawData.items()))
rawData = dict(map(lambda kv: (int(kv[0]), kv[1]), rawData.items()))

X = np.array(list(rawData.keys()))

    
Y1 = list(map(lambda kv: kv[1][0], rawData.items()))
Y2 = list(map(lambda kv: kv[1][1], rawData.items()))

X_Y_Spline = make_interp_spline(X, Y1)
X1 = np.linspace(X.min(), X.max(), 500)
Y1 = X_Y_Spline(X1)

X_Y_Spline = make_interp_spline(X, Y2)
X2 = np.linspace(X.min(), X.max(), 500)
Y2 = X_Y_Spline(X2)

plt.xlim(left=1, right=500)
plt.ylim(bottom=10, top=1000000)

plt.bar(X1, Y1, width=1, facecolor='#1f77b4', edgecolor=None, alpha=0.75)
plt.bar(X2, Y2, width=1, facecolor='#d62728', edgecolor=None, alpha=0.75)
plt.legend(["all type dependent", "peer dependent"], loc='upper right')

# plt.xticks([1, 100, 200, 300, 400, 500])
plt.xlabel('Quantity of dependent')
plt.ylabel('Version number (log scale)')

plt.savefig('1.PeerDeptCount.pdf')
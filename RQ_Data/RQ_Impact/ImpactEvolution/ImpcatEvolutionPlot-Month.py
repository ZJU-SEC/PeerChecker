from scipy.interpolate import make_interp_spline
import matplotlib.pyplot as plt
import numpy as np
import json

plt.figure()

with open('VersionDistribution.json', 'r') as f:
    data = json.load(f)

X = list(data.keys())
Y = list(data.values())
plt.xticks(rotation=30)

plt.plot(X, Y, color='#1f77b4', alpha=0.65)

plt.xlabel('Year')
plt.ylabel('Version number')

plt.savefig('VersionDistribution-Month.pdf', format='pdf', dpi=400)
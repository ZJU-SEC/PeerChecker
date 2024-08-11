from scipy.interpolate import make_interp_spline
import matplotlib.pyplot as plt
import numpy as np
import json

plt.figure(figsize=(8, 6))

with open('VersionDistribution.json', 'r') as f:
    data = json.load(f)

X = list(data.keys())
Y = list(data.values())
plt.xticks(rotation=30)

plt.bar(X, Y, width=0.5, color='#1f77b4', alpha=0.65)

plt.xlabel('Year')
plt.ylabel('Version number')

plt.grid(axis='y')

plt.savefig('VersionDistribution.pdf', format='pdf', dpi=400)
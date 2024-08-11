from scipy.interpolate import make_interp_spline
import matplotlib.pyplot as plt
import numpy as np
import json

plt.figure(figsize=(8, 6))

with open('DeptEvolution.json', 'r') as f:
    data = json.load(f)
    
NoDeptByYear = data['NoDeptByYear']
NoPeerDeptByYear = data['NoPeerDeptByYear']
HasPeerDeptByYear = data['HasPeerDeptByYear']
    
proportion1 = np.array([])
proportion2 = np.array([])
proportion3 = np.array([])

for key in NoDeptByYear.keys():
    total = NoDeptByYear[key] + NoPeerDeptByYear[key] + HasPeerDeptByYear[key]
    proportion1 = np.append(proportion1, float("{:.2f}".format(NoDeptByYear[key] / total)))
    proportion2 = np.append(proportion2, float("{:.2f}".format(NoPeerDeptByYear[key] / total)))
    proportion3 = np.append(proportion3, 1 - float("{:.2f}".format(NoDeptByYear[key] / total)) - float("{:.2f}".format(NoPeerDeptByYear[key] / total)))

X = list(NoDeptByYear.keys())
plt.xticks(rotation=30)

plt.bar(X, proportion1, width=0.5, color='#1f77b4', alpha=0.75)
plt.bar(X, proportion2, width=0.5, bottom=proportion1, color='#2ca02c', alpha=0.75)
plt.bar(X, proportion3, width=0.5, bottom=proportion1 + proportion2, color='#d62728', alpha=0.75)
plt.legend(["do not has any dependent", "has none-peer dependent", "has peer dependent"], bbox_to_anchor=(0.5, 1.1), loc='upper center', ncol=3)

plt.xlabel('Year')
plt.ylabel('Proportion')

plt.savefig('3.DeptEvolution.pdf', format='pdf', dpi=400)
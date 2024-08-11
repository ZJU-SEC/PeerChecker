import matplotlib.pyplot as plt

explode = (0.05, 0.05, 0.05)  # only "explode" the 2nd slice (i.e. 'Hogs')

# labels = 'With peer dependents','With non-peer dependents', 'No dependents'
sizes = [276226, 5977520, 30513484]

plt.figure()
plt.pie(sizes, autopct='%2.2f%%', colors=['#d62728', '#1f77b4', 'gray'], startangle=90, wedgeprops={'width': 1, "alpha": 0.75}, radius=1)
plt.legend(['With peer dependents','With non-peer dependents', 'No dependents'], loc='best')
plt.savefig('DeptsPro.pdf', format='pdf', dpi=400)



# import numpy as np
# import matplotlib.pyplot as plt

# # plt.rcParams['font.family'] = 'Arial'
# labels = ['With peer dependents','With non-peer dependents', 'No dependents']
# sizes = [276226, 5977520, 30513484]

# fig, ax = plt.subplots(figsize=(3, 3), subplot_kw=dict(aspect="equal"))

# wedges, texts = ax.pie(sizes, wedgeprops={'width': 0.5, "alpha": 0.75}, startangle=-40)

# kw = dict(arrowprops=dict(arrowstyle="->"), va="center")
# for p, label in zip(wedges, labels):
#     ang = np.deg2rad((p.theta1 + p.theta2)/2)
#     y = np.sin(ang)
#     x = np.cos(ang)
#     horizontalalignment = "center" if abs(x) < abs(y) else "right" if x < 0 else "left"
#     ax.annotate(label, xy=(0.75*x, 0.75*y), xytext=(1.3*x, 1.3*y),
#                 horizontalalignment=horizontalalignment, **kw)
# plt.tight_layout()
# plt.savefig('DeptsPro.pdf', format='pdf', dpi=400)

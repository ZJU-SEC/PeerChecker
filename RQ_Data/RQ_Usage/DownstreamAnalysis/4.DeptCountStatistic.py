import csv
import json
import numpy as np

with open('DeptCount.json', 'r') as f:
    data = json.load(f)
    
AllDeptStatistic = data['AllDeptStatistic']
PeerDeptStatistic = data['PeerDeptStatistic']

A = []

DeptsLessThanAvg = 0
for key, value in AllDeptStatistic.items():
    if int(key) > 0 and int(key) < 1010:
        DeptsLessThanAvg += value
    
    if int(key) > 0:
        A.extend([int(key) for i in range(value)])
    
print('DeptsLessThanAvg: ', DeptsLessThanAvg)
print('Std: ', np.std(np.array(A)))



B = []

PeerDeptsLessThanAvg = 0
for key, value in PeerDeptStatistic.items():
    if int(key) > 0 and int(key) < 197:
        PeerDeptsLessThanAvg += value
    
    if int(key) > 0:
        B.extend([int(key) for i in range(value)])
    
print('PeerDeptsLessThanAvg: ', PeerDeptsLessThanAvg)
print('Std: ', np.std(np.array(B)))

# print('Has No Depts: ', AllDeptStatistic[0])
# DeptsLess100 = 0
# DeptsLess500 = 0
# DeptsLess1000 = 0
# DeptsTotal = 0
# for key, value in AllDeptStatistic.items():
#     if key <= 100:
#         DeptsLess100 += value
#     if key <= 500:
#         DeptsLess500 += value
#     if key <= 1000:
#         DeptsLess1000 += value
#     DeptsTotal += key * value
    
# print('DeptsLess100: ', DeptsLess100, '{:.4f}'.format(DeptsLess100 / (line_count - 1)))
# print('DeptsLess500: ', DeptsLess500, '{:.4f}'.format(DeptsLess500 / (line_count - 1)))
# print('DeptsLess1000: ', DeptsLess1000, '{:.4f}'.format(DeptsLess1000 / (line_count - 1)))
# print('DeptsTotal: ', DeptsTotal, ' DeptsAvg: ', DeptsTotal / AtLeastOneDetps)

# print('Has No Peer Depts: ', PeerDeptStatistic[0])
# DeptsLess100 = 0
# DeptsLess500 = 0
# DeptsLess1000 = 0
# DeptsTotal = 0
# for key, value in PeerDeptStatistic.items():
#     if key <= 100:
#         DeptsLess100 += value
#     if key <= 500:
#         DeptsLess500 += value
#     if key <= 1000:
#         DeptsLess1000 += value
#     DeptsTotal += key * value

# print('PeerDeptsLess100: ', DeptsLess100, '{:.4f}'.format(DeptsLess100 / (line_count - 1)))
# print('PeerDeptsLess500: ', DeptsLess500, '{:.4f}'.format(DeptsLess500 / (line_count - 1)))
# print('PeerDeptsLess1000: ', DeptsLess1000, '{:.4f}'.format(DeptsLess1000 / (line_count - 1)))
# print('PeerDeptsTotal: ', DeptsTotal, ' PeerDeptsAvg: ', DeptsTotal / AtLeastOneDetps)

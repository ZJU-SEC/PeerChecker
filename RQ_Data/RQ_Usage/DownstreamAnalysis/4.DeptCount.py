import csv
import json
import numpy as np

AtLeastOneDetps = 0
AtLeastOnePeerDetps = 0
AllDeptStatistic = {}
PeerDeptStatistic = {} 

with open('../final/analysis.all.csv') as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    line_count = 0
    for row in csv_reader:
        if line_count == 0:
            # print(f'Colums are {", ".join(row)}')
            line_count += 1
        else:
            deptCount = int(row[5])
            
            if deptCount > 0:
                AtLeastOneDetps += 1
            
            if deptCount not in AllDeptStatistic:
                AllDeptStatistic[deptCount] = 0
            AllDeptStatistic[deptCount] += 1
                        
            peerCount = int(row[6])
            
            if peerCount > 0:
                AtLeastOnePeerDetps += 1
                
            if peerCount not in PeerDeptStatistic:
                PeerDeptStatistic[peerCount] = 0
            PeerDeptStatistic[peerCount] += 1
            
            line_count += 1
            
    print(f'Processed {line_count - 1} versions.')


AllDeptStatistic = dict(sorted(AllDeptStatistic.items()))
PeerDeptStatistic = dict(sorted(PeerDeptStatistic.items()))
    
with open('DeptCount.json', 'w') as f:
    data = {
        'AllDeptStatistic': AllDeptStatistic,
        'PeerDeptStatistic': PeerDeptStatistic
    }
    json.dump(data, f)

print('Has No Depts: ', AllDeptStatistic[0])
DeptsLess100 = 0
DeptsLess500 = 0
DeptsLess1000 = 0
DeptsTotal = 0
for key, value in AllDeptStatistic.items():
    if key <= 100:
        DeptsLess100 += value
    if key <= 500:
        DeptsLess500 += value
    if key <= 1000:
        DeptsLess1000 += value
    DeptsTotal += key * value
    
print('DeptsLess100: ', DeptsLess100, '{:.4f}'.format(DeptsLess100 / (line_count - 1)))
print('DeptsLess500: ', DeptsLess500, '{:.4f}'.format(DeptsLess500 / (line_count - 1)))
print('DeptsLess1000: ', DeptsLess1000, '{:.4f}'.format(DeptsLess1000 / (line_count - 1)))
print('DeptsTotal: ', DeptsTotal, ' DeptsAvg: ', DeptsTotal / AtLeastOneDetps)

print('Has No Peer Depts: ', PeerDeptStatistic[0])
DeptsLess100 = 0
DeptsLess500 = 0
DeptsLess1000 = 0
DeptsTotal = 0
for key, value in PeerDeptStatistic.items():
    if key <= 100:
        DeptsLess100 += value
    if key <= 500:
        DeptsLess500 += value
    if key <= 1000:
        DeptsLess1000 += value
    DeptsTotal += key * value

print('PeerDeptsLess100: ', DeptsLess100, '{:.4f}'.format(DeptsLess100 / (line_count - 1)))
print('PeerDeptsLess500: ', DeptsLess500, '{:.4f}'.format(DeptsLess500 / (line_count - 1)))
print('PeerDeptsLess1000: ', DeptsLess1000, '{:.4f}'.format(DeptsLess1000 / (line_count - 1)))
print('PeerDeptsTotal: ', DeptsTotal, ' PeerDeptsAvg: ', DeptsTotal / AtLeastOnePeerDetps)

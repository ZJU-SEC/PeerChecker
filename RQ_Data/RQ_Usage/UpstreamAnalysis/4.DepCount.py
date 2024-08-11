import csv
import json
import numpy as np

AllDepStatistic = {}
PeerDepStatistic = {} 

A = list()
B = list()
with open('../final/analysis.all.csv') as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    line_count = 0
    for row in csv_reader:
        if line_count == 0:
            # print(f'Colums are {", ".join(row)}')
            line_count += 1
        else:
            depCount = int(row[2])
            if depCount not in AllDepStatistic:
                AllDepStatistic[depCount] = 0
            AllDepStatistic[depCount] += 1
            
            A.append(depCount)
            
            peerCount = int(row[3])
            if peerCount not in PeerDepStatistic:
                PeerDepStatistic[peerCount] = 0
            PeerDepStatistic[peerCount] += 1
            
            line_count += 1
            
            B.append(peerCount)

    print(f'Processed {line_count - 1} versions.')


AllDepStatistic = dict(sorted(AllDepStatistic.items()))
PeerDepStatistic = dict(sorted(PeerDepStatistic.items()))
    
with open('DepCount.json', 'w') as f:
    data = {
        'AllDepStatistic': AllDepStatistic,
        'PeerDepStatistic': PeerDepStatistic
    }
    json.dump(data, f)

# print('Has No Deps: ', AllDepStatistic[0])
# DepsLess100 = 0
# DepsLess500 = 0
# DepsLess1000 = 0
# DepsTotal = 0
# for key, value in AllDepStatistic.items():
#     if key <= 100:
#         DepsLess100 += value
#     if key <= 500:
#         DepsLess500 += value
#     if key <= 1000:
#         DepsLess1000 += value
#     DepsTotal += key * value
    
# print('DepsLess100: ', DepsLess100, '{:.4f}'.format(DepsLess100 / (line_count - 1)))
# print('DepsLess500: ', DepsLess500, '{:.4f}'.format(DepsLess500 / (line_count - 1)))
# print('DepsLess1000: ', DepsLess1000, '{:.4f}'.format(DepsLess1000 / (line_count - 1)))
# print('DepsTotal: ', DepsTotal, ' DepsAvg: ', DepsTotal / (line_count - 1), ' DepsMedian: ', np.median(A))

# print('Has No Peer Deps: ', PeerDepStatistic[0])
# DepsLess100 = 0
# DepsLess500 = 0
# DepsLess1000 = 0
# DepsTotal = 0
# for key, value in PeerDepStatistic.items():
#     if key <= 100:
#         DepsLess100 += value
#     if key <= 500:
#         DepsLess500 += value
#     if key <= 1000:
#         DepsLess1000 += value
#     DepsTotal += key * value

# print('PeerDepsLess100: ', DepsLess100, '{:.4f}'.format(DepsLess100 / (line_count - 1)))
# print('PeerDepsLess500: ', DepsLess500, '{:.4f}'.format(DepsLess500 / (line_count - 1)))
# print('PeerDepsLess1000: ', DepsLess1000, '{:.4f}'.format(DepsLess1000 / (line_count - 1)))
# print('PeerDepsTotal: ', DepsTotal, ' PeerDepsAvg: ', DepsTotal / (line_count - 1), ' PeerDepsMedian: ', np.median(B))

import csv
import json
import numpy as np

PeerDeptCount = {}
with open('../final/analysis.all.csv') as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    line_count = 0
    for row in csv_reader:
        if line_count == 0:
            # print(f'Colums are {", ".join(row)}')
            line_count += 1
        else:
            depCount = int(row[5])
            peerCount = int(row[6])
            
            if depCount != 0:                
                if depCount not in PeerDeptCount:
                    PeerDeptCount[depCount] = [0, 0]
                    
                PeerDeptCount[depCount][0] += 1
                if peerCount != 0:
                    PeerDeptCount[depCount][1] += 1
                    
            line_count += 1
    print(f'Processed {line_count - 1} versions.')


PeerDeptCount = dict(sorted(PeerDeptCount.items()))
    
with open('PeerDeptCount.json', 'w') as f:
    json.dump(PeerDeptCount, f)
    
    
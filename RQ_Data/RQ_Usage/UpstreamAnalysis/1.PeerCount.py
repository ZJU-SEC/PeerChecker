import csv
import json
import numpy as np

PeerCount = {}
with open('../final/analysis.all.csv') as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    line_count = 0
    for row in csv_reader:
        if line_count == 0:
            # print(f'Colums are {", ".join(row)}')
            line_count += 1
        else:
            depCount = int(row[2])
            peerCount = int(row[3])
            
            if depCount != 0:                
                if depCount not in PeerCount:
                    PeerCount[depCount] = [0, 0]
                    
                PeerCount[depCount][0] += 1
                if peerCount != 0:
                    PeerCount[depCount][1] += 1
                    
            line_count += 1
    print(f'Processed {line_count - 1} versions.')


PeerCount = dict(sorted(PeerCount.items()))
    
with open('PeerCount.json', 'w') as f:
    json.dump(PeerCount, f)
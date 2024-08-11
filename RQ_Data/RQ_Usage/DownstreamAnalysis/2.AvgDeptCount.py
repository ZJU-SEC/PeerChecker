import csv
import json
from datetime import datetime

AvgDeptCount = {}

with open('../final/analysis.all.csv') as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    line_count = 0
    for row in csv_reader:
        if line_count == 0:
            # print(f'Colums are {", ".join(row)}')
            line_count += 1
        else:                 
            dt = datetime.strptime(row[7][:19], "%Y-%m-%dT%H:%M:%S")
            year = dt.year
            
            if year < 2000:
                continue
            
            allDepts = int(row[5])
            peerDepts = int(row[6])
            
            if year not in AvgDeptCount:
                AvgDeptCount[year] = [0, 0, 0]
            
            AvgDeptCount[year][0] += 1
            AvgDeptCount[year][1] += allDepts
            AvgDeptCount[year][2] += peerDepts
            
            
            line_count += 1
    
    print(f'Processed {line_count - 1} versions.')

AvgDeptCount = dict(sorted(AvgDeptCount.items()))

with open('AvgDeptCount.json', 'w') as f:
    json.dump(AvgDeptCount, f)
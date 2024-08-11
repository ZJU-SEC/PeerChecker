import csv
import json
from datetime import datetime

AvgCount = {}

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
            
            allDeps = int(row[2])
            peerDeps = int(row[3])
            
            if year not in AvgCount:
                AvgCount[year] = [0, 0, 0]
            
            AvgCount[year][0] += 1
            AvgCount[year][1] += allDeps
            AvgCount[year][2] += peerDeps
            
            
            line_count += 1
    
    print(f'Processed {line_count - 1} versions.')

AvgCount = dict(sorted(AvgCount.items()))

with open('AvgCount.json', 'w') as f:
    json.dump(AvgCount, f)
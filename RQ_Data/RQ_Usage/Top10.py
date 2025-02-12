import csv
import json
import numpy as np

TopDept = []
TopPeerDept = []
with open('./final/analysis.all.csv') as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    line_count = 0
    for row in csv_reader:
        if line_count == 0:
            # print(f'Colums are {", ".join(row)}')
            line_count += 1
        else:       
            TopDept.append((int(row[5]), row[0], row[1], row[7]))
            TopPeerDept.append((int(row[6]), row[0], row[1], row[7]))
            line_count += 1
    print(f'Processed {line_count - 1} versions.')

TopDept.sort(reverse=True) 
TopPeerDept.sort(reverse=True)
    
with open('Top.json', 'w') as f:
    data = {
        'TopDept': TopDept[:2000],
        'TopPeerDept': TopPeerDept[:2000]
    }
    json.dump(data, f)
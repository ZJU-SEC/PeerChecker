import csv
import json
from datetime import datetime

NoDeptByYear = {}
NoPeerDeptByYear = {}
HasPeerDeptByYear = {}

with open('../final/analysis.all.csv') as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    line_count = 0
    for row in csv_reader:
        if line_count == 0:
            print(f'Colums are {", ".join(row)}')
            line_count += 1
        else:
            dt = datetime.strptime(row[7][:19], "%Y-%m-%dT%H:%M:%S")
            year = dt.year
            
            if year < 2000:
                continue
            
            if int(row[5]) == 0:
                if year not in NoDeptByYear:
                    NoDeptByYear[year] = 0
                NoDeptByYear[year] += 1
            else:
                if int(row[6]) == 0:
                    if year not in NoPeerDeptByYear:
                        NoPeerDeptByYear[year] = 0
                    NoPeerDeptByYear[year] += 1
                else:
                    if year not in HasPeerDeptByYear:
                        HasPeerDeptByYear[year] = 0
                    HasPeerDeptByYear[year] += 1                    
            line_count += 1

    print(f'Processed {line_count - 1} versions.')


NoDeptByYear = dict(sorted(NoDeptByYear.items()))
NoPeerDeptByYear = dict(sorted(NoPeerDeptByYear.items()))
HasPeerDeptByYear = dict(sorted(HasPeerDeptByYear.items()))

with open('DeptEvolution.json', 'w') as f:
    data = {
        'NoDeptByYear': NoDeptByYear,
        'NoPeerDeptByYear': NoPeerDeptByYear,
        'HasPeerDeptByYear': HasPeerDeptByYear
    }
    json.dump(data, f)

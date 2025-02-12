import csv
import json
from datetime import datetime

NoDepByYear = {}
NoPeerByYear = {}
HasPeerByYear = {}

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
            
            if int(row[2]) == 0:
                if year not in NoDepByYear:
                    NoDepByYear[year] = 0
                NoDepByYear[year] += 1
            else:
                if int(row[3]) == 0:
                    if year not in NoPeerByYear:
                        NoPeerByYear[year] = 0
                    NoPeerByYear[year] += 1
                else:
                    if year not in HasPeerByYear:
                        HasPeerByYear[year] = 0
                    HasPeerByYear[year] += 1                    
            line_count += 1

    print(f'Processed {line_count - 1} versions.')


NoDepByYear = dict(sorted(NoDepByYear.items()))
NoPeerByYear = dict(sorted(NoPeerByYear.items()))
HasPeerByYear = dict(sorted(HasPeerByYear.items()))

with open('DepEvolution.json', 'w') as f:
    data = {
        'NoDepByYear': NoDepByYear,
        'NoPeerByYear': NoPeerByYear,
        'HasPeerByYear': HasPeerByYear
    }
    json.dump(data, f)

# with open('NoDepByYear.json', 'w') as f:
#     json.dump(NoDepByYear, f)
# with open('NoPeerByYear.json', 'w') as f:
#     json.dump(NoPeerByYear, f)
# with open('HasPeerByYear.json', 'w') as f:
#     json.dump(HasPeerByYear, f)


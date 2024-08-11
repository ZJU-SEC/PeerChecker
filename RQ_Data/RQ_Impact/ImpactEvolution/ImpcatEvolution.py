import json
from datetime import datetime

DistributionMap = {}

with open('../name_version_time_filtered.json', 'r') as f:
    TimeTable = json.load(f)
    
print('Package Number: ', len(TimeTable))

with open('../QueueLoopVersion.txt', 'r') as f:
    for line in f:
        info = line.strip('\n').split(' ')
        name, version = info[0], info[1]
        
        try:
            dt = TimeTable[name][version]
            dt = datetime.strptime(dt[:19], "%Y-%m-%dT%H:%M:%S")
            
            year = dt.year 
            month = dt.month
            
            key = '{}-{:02d}'.format(year, month)
            if key not in DistributionMap:
                DistributionMap[key] = 0
            DistributionMap[key] += 1
        except:
            pass

DistributionMap = dict(sorted(DistributionMap.items()))
        
with open('VersionDistribution-Month.json', 'w') as f:
    json.dump(DistributionMap, f)
            
            
            
import json
from datetime import datetime

Cache = {}
LegMap = {}

with open('../name_version_time_filtered.json', 'r') as f:
    TimeTable = json.load(f)

# print(sum(TimeTable.values()))    

with open('../QueueLoopVersion.txt', 'r') as f:
    for line in f:
        info = line.strip('\n').split(' ')
        name, version = info[0], info[1]
        
        try:
            dt = TimeTable[name][version]
            dt = datetime.strptime(dt[:19], "%Y-%m-%dT%H:%M:%S")
            
            latestVersion = list(TimeTable[name].keys())[-1]
            latest = TimeTable[name][latestVersion]
            latest = datetime.strptime(latest[:19], "%Y-%m-%dT%H:%M:%S")

            leg = (latest - dt).days
            
            if leg not in LegMap:
                LegMap[leg] = 0
            LegMap[leg] += 1

            
        except:
            pass


LegMap = dict(sorted(LegMap.items()))
        
print(sum(LegMap.values()))        
        
with open('LegMap.json', 'w') as f:
    json.dump(LegMap, f, indent=2)

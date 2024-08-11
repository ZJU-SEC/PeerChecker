import json
from datetime import datetime

Mapping = {}

with open('../QueueLoopVersionDetected.txt', 'r') as f:
    for line in f:
        
        kv = json.loads(line.strip('\n'))
        (key, value), = kv.items()
        
        affPkgName, affPkgVer =  key.split(' ')
        filepath, name, version = value.split(' ')
        
        if name not in Mapping:
            Mapping[name] = set()
            
        Mapping[name].add(affPkgName)

PkgCount = {}
for k, v in Mapping.items():
    PkgCount[k] = len(v)
        
PkgCount = dict(sorted(PkgCount.items(), key=lambda item: item[1], reverse=True)) 
        

VersionCount = {}

with open('../QueueLoopVersionDetected.txt', 'r') as f:
    for line in f:
        
        kv = json.loads(line.strip('\n'))
        (key, value), = kv.items()
        
        affPkgName, affPkgVer =  key.split(' ')
        filepath, name, version = value.split(' ')
        
        if name not in VersionCount:
            VersionCount[name] = 0
            
        VersionCount[name] += 1
        
VersionCount = dict(sorted(VersionCount.items(), key=lambda item: item[1], reverse=True)) 
        
with open('Top.json', 'w') as f:
    data = {
        'AffectedPkg': PkgCount,
        'AffectedVer': VersionCount
    }
    json.dump(data, f)
        
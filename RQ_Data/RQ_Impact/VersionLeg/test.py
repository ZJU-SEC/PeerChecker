import json


with open('LegMap.json', 'r') as f:
    raw = json.load(f)
    
    # 210 270
    
    totalVersion = 0
    
    Lag200 = 0
    Lag210 = 0
    Lag270 = 0
    Lag300 = 0
    
    for lag, cnt in raw.items():
        totalVersion += cnt
        
        if int(lag) <= 200:
            Lag200 += cnt        
        
        if int(lag) <= 210:
            Lag210 += cnt
            
        if int(lag) <= 270:
            Lag270 += cnt
        
        if int(lag) <= 300:
            Lag300 += cnt

print(totalVersion)
print(Lag200)
print(Lag210)
print(Lag270)
print(Lag300)
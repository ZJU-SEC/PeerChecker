# :robot: PeerChecker

An implementation of npm arborist with PeerSpin check.

## :hammer_and_wrench: Build

```bash
cmake -B build/release -D CMAKE_BUILD_TYPE=Release
cmake --build build/release
```

You should see and edit the ```config.hpp``` before building.

The default main.cpp will not get input and output.
YOU MUST RUN THE FUNCTION ON YOUR OWN DEMAND.
It's quite easy: just give input to and get output from the parse function in main.cpp
You can write your own multi-processing logic.

The inputs are: name and version of your parsing target.
The outputs are: a dep tree, a dir tree and corresponding log.

The dep tree is the same as "npm ls -a", which may like below:
```json
{
    "mocha@10.0.0": {
        "@ungap/promise-all-settled@1.1.2": {},
        "ansi-colors@4.1.1": {},
        "browser-stdout@1.3.1": {},
        "chokidar@3.5.3": {
            "anymatch@3.1.3": {
                "normalize-path@3.0.0": "deduped",
                "picomatch@2.3.1": {}
            },
            ...,
            "normalize-path@3.0.0": {},
            "readdirp@3.6.0": {
                "picomatch@2.3.1": "deduped"
            }
        },
        ...
    }
}
```

The dir tree is the same as the node_modules directory created by npm, which may like below:
```json
{
    "@ungap/promise-all-settled@1.1.2": {},
    "ansi-colors@4.1.1": {},
    "ansi-regex@5.0.1": {},
    ...,
    "debug@4.3.4": {
        "ms@2.1.2": {}
    },
    "glob@7.2.0": {
        "brace-expansion@1.1.11": {},
        "minimatch@3.1.2": {}
    },
    ...
}
```

The dep tree and dir tree are both in JSON format.

The log will be "Ok" if parsing is successful.

When it's not "Ok", the parsing target has an install-time error.

* ```Conflict``` means there is dependency conflict.

* ```QueueLoop``` or "QueueLoopReplacementDetected" means a loop problem is resolved.

* ```LoadLoop``` means there is a loading loop problem.

* ```NpmError``` means there is a nullptr problem.

* ```Empty```, ```PlaceLoop``` and ```UnknownError``` should not appear in theory.

Note, ```QueueLoop```, ```LoadLoop``` and ```PlaceLoop``` means the loop exceeds the limit, while "QueueLoopReplacementDetected" means we detected the resolving loop problem.



## :book: Citation
```
@INPROCEEDINGS {,
author = { Wang, Xingyu and Wang, Mingsen and Shen, Wenbo and Chang, Rui },
booktitle = { 2025 IEEE/ACM 47th International Conference on Software Engineering (ICSE) },
title = {{ Understanding and Detecting Peer Dependency Resolving Loop in npm Ecosystem }},
year = {2025},
volume = {},
ISSN = {1558-1225},
pages = {591-591},
keywords = {},
doi = {10.1109/ICSE55347.2025.00054},
url = {https://doi.ieeecomputersociety.org/10.1109/ICSE55347.2025.00054},
publisher = {IEEE Computer Society},
address = {Los Alamitos, CA, USA},
month =May}
```

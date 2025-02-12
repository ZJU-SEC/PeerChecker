#!/bin/bash

docker run --name npm_syncer -v $HOME/sync_seq:/app/last_seq --network host -d replicate_update_crawler
echo "mount $HOME/last_seq to /app/last_seq"
echo "use $HOME/last_seq/last_seq.json to store progress"

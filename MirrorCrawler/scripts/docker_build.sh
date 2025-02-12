#!/bin/bash

docker build --tag replicate_update_crawler ../crawler
echo "should edit COUCHDB_URL and other couchdb related settings in crawler's settings.py before build"

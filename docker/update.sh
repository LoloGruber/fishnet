#!/bin/bash
PATH_TO_PROJECT_ROOT=../
VERSION=$(grep -oP 'project\s*\([^)]*VERSION\s+\K[0-9]+\.[0-9]+\.[0-9]+' $PATH_TO_PROJECT_ROOT/CMakeLists.txt)
IMAGE=fishnet-apps
docker build -t logru/$IMAGE:$VERSION -t logru/$IMAGE:latest -f Dockerfile.apps $PATH_TO_PROJECT_ROOT
docker login 
docker push logru/$IMAGE:$VERSION 
docker push logru/$IMAGE:latest

# Update fishnet-deps
# IMAGE=fishnet-deps
# docker build -t logru/$IMAGE:$VERSION -t logru/$IMAGE:latest -f Dockerfile.deps $PATH_TO_PROJECT_ROOT
# docker push logru/$IMAGE:$VERSION
# docker push logru/$IMAGE:latest
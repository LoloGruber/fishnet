#!/bin/bash
docker build -t logru/fishnet-deps:latest .
docker login 
docker push logru/fishnet-deps:latest
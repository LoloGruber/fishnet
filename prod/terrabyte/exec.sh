#!/bin/bash
# Check if a script is provided
if [ -z "$1" ]; then
  echo "Usage: $0 <script> [parameters]"
  exit 1
fi
# Load charliecloud module
module use /dss/dsstbyfs01/pn56su/pn56su-dss-0020/usr/share/modules/files/
module load charliecloud
# Run the charliecloud command with the provided script and parameters
ch-run --home -b ~/home:/home/ ~/.fishnet/fishnet-container -- ./start.sh "$@"
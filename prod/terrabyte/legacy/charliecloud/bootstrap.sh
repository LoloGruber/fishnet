#!/bin/bash

# Add /opt/venv/bin to the PATH
export PATH=/opt/venv/bin:$PATH
cd /home/workingDirectory
# Check if a script is provided
if [ -z "$1" ]; then
  echo "Usage: $0 <script> [parameters]"
  exit 1
fi

# Get the script and parameters
SCRIPT=$1
shift

# Run the script with the provided parameters
$SCRIPT "$@"
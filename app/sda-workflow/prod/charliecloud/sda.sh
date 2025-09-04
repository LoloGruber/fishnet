#!/bin/bash
#SBATCH --job-name=settDelAna
#SBATCH --output=stdout.log
#SBATCH --error=stderr.log
#SBATCH --exclusive
#SBATCH --nodes=1
#SBATCH --cpus-per-task=70
#SBATCH --time=7-00:00:00
#SBATCH --partition=hpda2_compute
#SBATCH --mem=0
#SBATCH --mail-type=end,fail
#SBATCH --mail-user=lorenz.gruber@uni-wuerzburg.de

# Parse Input Arguments
while getopts "i:c:o:" opt; do
    case $opt in
        i) inputPath="$OPTARG" ;;
        c) cfgFile="$OPTARG" ;;
        o) outputPath="$OPTARG" ;;
        *) echo "Usage: $0 -i <inputPath> -c <cfgFile> -o <outputPath>"; exit 1 ;;
    esac
done

if [ -z "$inputPath" ] || [ -z "$cfgFile" ] || [ -z "$outputPath" ]; then
        echo "Usage: $0 -i <inputPath> -c <cfgFile> -o <outputPath>"
        exit 1
fi

start_db(){
    echo "Starting Memgraph Database..."
    module use /dss/dsstbyfs01/pn56su/pn56su-dss-0020/usr/share/modules/files/
    module load charliecloud
    ch-run --write -b ~/sda-workflow/logs/memgraph:/var/log/memgraph logru%sda+latest --  /usr/lib/memgraph/memgraph --query-execution-timeout-sec=0 --storage-mode=IN_MEMORY_ANALYTICAL &
    sleep 5
    DB_PID=$!
    echo "Memgraph started with PID $DB_PID"
}

stop_db(){
    echo "Stopping Memgraph with PID $DB_PID..."
    pkill -f "memgraph"
    echo "Memgraph stopped."
}



# Extract parent directory and filename for input, cfg and output
input_parent_path="$(dirname "$inputPath")"
input_filename="$(basename "$inputPath")"
cfg_parent_path="$(dirname "$cfgFile")"
cfg_filename="$(basename "$cfgFile")"
output_parent_path="$(dirname "$outputPath")"
output_filename="$(basename "$outputPath")"

# Create Output Directory if it does not exist
if [ ! -d "$output_parent_path" ]; then
    mkdir -p "$output_parent_path"
fi

trap stop_db EXIT
start_db
echo "Waiting for Database to start..."
sleep 1
echo "Running Settlement Delineation..."
module use /dss/dsstbyfs01/pn56su/pn56su-dss-0020/usr/share/modules/files/
module load charliecloud
ch-run --write \
    -b "$input_parent_path:/data/input/" \
    -b "$cfg_parent_path:/data/config/" \
    -b "$output_parent_path:/data/output/" \
    logru%sda+latest -- \
    SettlementDelineation -i /data/input/$input_filename -o /data/output/$output_filename -c /data/config/$cfg_filename
stop_db
echo "SDA Workflow completed."
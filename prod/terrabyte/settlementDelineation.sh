inputDirname="Germany"
outputFilename="Germany.shp"

inputDirectory=/home/input/
outputDirectory=/home/output/
cfgFile=/home/cfg/workflow.json
outputPath=$outputDirectory$outputFilename
inputPath=$inputDirectory$inputDirname

start_db(){
    echo "Starting Memgraph Database..."
    ./memgraph/startMemgraph.sh &
    sleep 1
    DB_PID=$!
    echo "Memgraph started with PID $DB_PID"
}

stop_db(){
    echo "Stopping Memgraph with PID $DB_PID..."
    pkill -TERM -P $DB_PID
    wait $DB_PID 2>/dev/null
    echo "Memgraph stopped."
}

trap stop_db EXIT
start_db
echo "Waiting for Database to start..."
sleep 30
echo "Running Settlement Delineation..."
./exec.sh SettlementDelineation -i $inputPath -o $outputPath -c $cfgFile
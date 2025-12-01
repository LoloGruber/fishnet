import subprocess
import os
from pathlib import Path
import argparse
import time
import json

SDA_IMAGE = "logru/sda:1.2.1"
CONTAINER_NAME = "SDAWorkflowContainer"

class SettlementDelineationAnalysis:
    def __init__(self, config_file, input_file, output_file, observer_file= None):
        self.config_file = config_file
        self.input_file = input_file
        self.output_file = output_file
        self.observer_file = observer_file
        self._setup_paths_and_config()

    def __enter__(self):
        return self
    
    def _setup_paths_and_config(self):
        # Ensure the config file has the correct executor attribute
        with open(self.config_file, "r") as f:
            config_data = json.load(f)

        expected_executor = {
            "name": "CWLTOOL",
            "flags": "--log-dir /data/logs --no-container",
            "cwl-directory": "/cwl"
        }
        if "executor" not in config_data or config_data["executor"] != expected_executor:
            config_data["executor"] = expected_executor
            config_data["memgraph-host"] = "localhost"  # Ensure memgraph-host is set to localhost
            config_data["memgraph-port"] = 7687  # Ensure memgraph-port is set to 7687
            with open(self.config_file, "w") as f:
                json.dump(config_data, f, indent=4)
        # Ensure output and observer parent directories exist
        os.makedirs(os.path.dirname(self.output_file), exist_ok=True)
        if self.observer_file:
            os.makedirs(os.path.dirname(self.observer_file), exist_ok=True)

    def _mounts(self):
        """
        Returns a list of volume mounts for the Docker container.
        """
        mounts = [
            f"{os.path.abspath(os.path.dirname(self.config_file))}:/data/cfg",
            f"{os.path.abspath(os.path.dirname(self.input_file))}:/data/input",
            f"{os.path.abspath(os.path.dirname(self.output_file))}:/data/output"
        ]
        if self.observer_file:
            mounts.append(f"{os.path.dirname(self.observer_file)}:/data/observer")
        return mounts

    def _build_docker_run_command(self):
        docker_run_command = [
            "docker", "run","--name", CONTAINER_NAME,"-d","--rm"
        ]
        for mount in self._mounts():
            docker_run_command.extend(["-v", mount])
        docker_run_command.append(SDA_IMAGE)

        return docker_run_command
    
    def _build_docker_sda_command(self):
        docker_exec_command = [
            "docker", "exec", CONTAINER_NAME
        ]

        docker_exec_command.extend([
            "SettlementDelineation",
            "-c", f"/data/cfg/{Path(self.config_file).name}",
            "-i", f"/data/input/{Path(self.input_file).name}",
            "--listener", f"/data/observer/{Path(self.observer_file).name}" if self.observer_file else "",
            "-o", f"/data/output/{Path(self.output_file).name}"
        ])
        return docker_exec_command
    
    def run(self):
        print(f"Waiting for memgraph database to start...")
        subprocess.run(self._build_docker_run_command(),capture_output=False)
        time.sleep(1)  # Wait for the database to be ready
        try:
            print("Running SettlementDelineationAnalysis Workflow...")
            result = subprocess.run(self._build_docker_sda_command(), check=True, capture_output=True, text=True)
            print(result.stdout)
            if result.stderr:
                print("STDERR:", result.stderr)
            output_path = os.path.abspath(self.output_file)
            output_dir = os.path.dirname(output_path)
            print(f"\nOutput directory: file://{output_dir}\nOutput file: file://{output_path}\n")
        except subprocess.CalledProcessError as e:
            print("Error executing SDA Workflow:\n", e.stdout, e.stderr)

    def __exit__(self, exc_type, exc_value, traceback):
        """
        Clean up resources, if necessary.
        """
        try:
            subprocess.run(
                ["docker", "stop", CONTAINER_NAME],
                check=True,
                capture_output=True,
                text=True
            )
        except subprocess.CalledProcessError as e:
            print(f"Error stopping Docker container:\n", e.stdout, e.stderr)
        
def parse_args_to_workflow_object() -> SettlementDelineationAnalysis:
    parser = argparse.ArgumentParser(description="Run SettlementDelineationAnalysis Workflow in Docker Compose")
    parser.add_argument("-c", "--config", required=True, help="Path to config file")
    parser.add_argument("-i", "--input", required=True, help="Path to input file")
    parser.add_argument("-o", "--output", required=True, help="Path to output file")
    parser.add_argument("--listener", required=False, help="Path to observer/listener file")
    args = parser.parse_args()
    return SettlementDelineationAnalysis(
        config_file=args.config,
        input_file=args.input,
        output_file=args.output,
        observer_file=Path(args.listener) if args.listener else None
    )

def get_debug_workflow() -> SettlementDelineationAnalysis:
    return SettlementDelineationAnalysis(
        config_file="/home/lolo/Projects/fishnet/app/sda-workflow/prod/docker/sda-docker.json",
        input_file="/home/lolo/Projects/fishnet/data/samples/Corvara_IT.tiff",
        output_file="/home/lolo/Desktop/SDA/Corvara_IT_SettlementDelineation.shp",
        observer_file="/home/lolo/Desktop/SDA/Corvara_IT_SettlementDelineation_Observer.json"
    )

if __name__ == "__main__":
    debug = False
    workflow = get_debug_workflow() if debug else parse_args_to_workflow_object()
    with workflow as workflow:
        workflow.run()
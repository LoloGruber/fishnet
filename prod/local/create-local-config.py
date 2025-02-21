import os
import json
from pathlib import Path

def config_template() -> dict:
    return {
        "analysis-output-stem": "Analysis",
        "binary-filters": [
            {
                "name": "InsidePolygonFilter"
            }
        ],
        "centrality-measures": [
            {
                "name": "DegreeCentrality"
            },
            {
                "name": "MeanLocalSignificance"
            },
            {
                "name": "SmallerNeighboursRatio"
            }
        ],
        "cleanup": True,
        "concurrent-runs": True,
        "contraction-output-stem": "Contraction",
        "contraction-predicates": [
            {
                "distance": 500.0,
                "name": "DistanceBiPredicate"
            }
        ],
        "executor": {
            "cwl-directory": "/home/lolo/Projects/fishnet/cwl/sda/",
            "flags": "--quiet --no-container",
            "name": "CWLTOOL"
        },
        "hardware-concurrency": 0,
        "last-job-type": "MERGE",
        "maxDistanceMeters": 3000.0,
        "maxNeighbours": 5,
        "memgraph-host": "localhost",
        "memgraph-port": 7687,
        "merge-workers": 2,
        "neighbouring-files-predicate": "WSF",
        "neighbouring-predicates": [],
        "splits": 0,
        "unary-filters": [
            {
                "name": "ApproxAreaFilter",
                "requiredArea": 5000.0
            }
        ],
        "visualize-edges": True
    }

configFileRelativePath = "prod/local/sda-workflow-local.json"
cwlRelativePath = "cwl/sda"

def get_project_directory() -> str:
    current_dir = Path(os.path.abspath(os.path.curdir))
    while(not str(current_dir.name) in ("/","fishnet")):
        current_dir = Path(current_dir.parent)
    if(current_dir.name == "/"):
        raise ValueError("Project directory not found")
    return current_dir

def create_local_config() -> dict:
    file = get_project_directory() / configFileRelativePath
    cfg = config_template()
    cfg["executor"]["cwl-directory"] = str(get_project_directory() / cwlRelativePath)
    print(cfg)
    with open(file,"w") as json_file:
        json.dump(cfg,json_file,indent=4)

create_local_config()

#!/usr/bin/python
import os
from pathlib import Path

def get_project_directory() -> str:
    current_dir = Path(os.path.abspath(os.path.curdir))
    while(not str(current_dir.name) in ("/","fishnet")):
        current_dir = Path(current_dir.parent)
    if(current_dir.name == "/"):
        raise ValueError("Project directory not found")
    return current_dir

def createFilterExample(project_dir: Path)->str:
    return f'''{{
    "shpFile": {{
        "class": "File",
        "path": "{project_dir}/data/testing/Punjab_Small/Punjab_Small.shp",
        "secondaryFiles": [
            {{"class": "File", "path": "{project_dir}/data/testing/Punjab_Small/Punjab_Small.shx"}},
            {{"class": "File", "path": "{project_dir}/data/testing/Punjab_Small/Punjab_Small.dbf"}},
            {{"class": "File", "path": "{project_dir}/data/testing/Punjab_Small/Punjab_Small.prj"}}
        ]
    }},
    "config": {{
        "class": "File",
        "path": "{project_dir}/prod/local/cfg/sda-workflow.json"
    }}
}}'''

def createNeighboursExample(project_dir: Path)->str:
    return f'''{{
    "primaryInput":{{
        "class": "File",
        "path": "{project_dir}/tests/workflow/example/workingDir/Punjab_Small_filtered.shp",
        "secondaryFiles": [
            {{"class": "File", "path": "{project_dir}/tests/workflow/example/workingDir/Punjab_Small_filtered.shx"}},
            {{"class": "File", "path": "{project_dir}/tests/workflow/example/workingDir/Punjab_Small_filtered.dbf"}},
            {{"class": "File", "path": "{project_dir}/tests/workflow/example/workingDir/Punjab_Small_filtered.prj"}}
        ]
    }},
    "additionalInput":[
        
    ],
    "config":{{
        "class":"File",
        "path": "{project_dir}/prod/local/cfg/sda-workflow.json"
    }},
    "taskID":1
}}'''

def createContractionExample(project_dir:Path)->str:
    return f'''{{
    "shpFiles":[{{
        "class": "File",
        "path": "{project_dir}/tests/workflow/example/workingDir/Punjab_Small_filtered.shp",
        "secondaryFiles": [
            {{"class": "File", "path": "{project_dir}/tests/workflow/example/workingDir/Punjab_Small_filtered.shx"}},
            {{"class": "File", "path": "{project_dir}/tests/workflow/example/workingDir/Punjab_Small_filtered.dbf"}},
            {{"class": "File", "path": "{project_dir}/tests/workflow/example/workingDir/Punjab_Small_filtered.prj"}}
        ]
    }}],
    "config":{{
        "class":"File",
        "path": "{project_dir}/prod/local/cfg/sda-workflow.json"
    }},
    "outputStem": "Punjab_Small_merged",
    "taskID":1 
}}'''

def createAnalysisExample(project_dir:Path)->str:
    return f'''{{
    "shpFile": {{
        "class": "File",
        "path": "{project_dir}/tests/workflow/example/workingDir/Punjab_Small_merged.shp",
        "secondaryFiles": [
            {{"class": "File", "path": "{project_dir}/tests/workflow/example/workingDir/Punjab_Small_merged.shx"}},
            {{"class": "File", "path": "{project_dir}/tests/workflow/example/workingDir/Punjab_Small_merged.dbf"}},
            {{"class": "File", "path": "{project_dir}/tests/workflow/example/workingDir/Punjab_Small_merged.prj"}}
        ]
    }},
    "config":{{
        "class":"File",
        "path": "{project_dir}/prod/local/cfg/sda-workflow.json"
    }},
    "outputStem": "Punjab_Small_Analysis"
}}'''

def writeJSONFile(content:str, filename: str):
    if not filename.endswith(".json"):
        filename += ".json"
    filename = "jobs/"+filename
    with open(filename,"w") as f:
        f.write(content)

if(not os.path.exists("./jobs")):
    os.mkdir("jobs")
project_dir = get_project_directory()
writeJSONFile(createFilterExample(project_dir),"filterExample")
writeJSONFile(createNeighboursExample(project_dir),"neighboursExample")
writeJSONFile(createContractionExample(project_dir),"contractionExample")
writeJSONFile(createAnalysisExample(project_dir),"analysisExample")

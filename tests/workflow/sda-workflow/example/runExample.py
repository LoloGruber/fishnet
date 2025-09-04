#!/usr/bin/python
import os
import shutil
from pathlib import Path
import runpy

configFileRelativePath = "app/sda-workflow/prod/local/sda-workflow-local.json"
WORKING_DIR_NAME = "workingDir"

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
        "path": "{project_dir}/{configFileRelativePath}"
    }}
}}'''

def createNeighboursExample(project_dir: Path, working_dir: Path)->str:
    return f'''{{
    "primaryInput":{{
        "class": "File",
        "path": "{working_dir}/Punjab_Small_filtered.shp",
        "secondaryFiles": [
            {{"class": "File", "path": "{working_dir}/Punjab_Small_filtered.shx"}},
            {{"class": "File", "path": "{working_dir}/Punjab_Small_filtered.dbf"}},
            {{"class": "File", "path": "{working_dir}/Punjab_Small_filtered.prj"}}
        ]
    }},
    "additionalInput":[
        
    ],
    "config":{{
        "class":"File",
        "path": "{project_dir}/{configFileRelativePath}"
    }},
    "taskID":1
}}'''

def createContractionExample(project_dir:Path, working_dir: Path)->str:
    return f'''{{
    "shpFiles":[{{
        "class": "File",
        "path": "{working_dir}/Punjab_Small_filtered.shp",
        "secondaryFiles": [
            {{"class": "File", "path": "{working_dir}/Punjab_Small_filtered.shx"}},
            {{"class": "File", "path": "{working_dir}/Punjab_Small_filtered.dbf"}},
            {{"class": "File", "path": "{working_dir}/Punjab_Small_filtered.prj"}}
        ]
    }}],
    "config":{{
        "class":"File",
        "path": "{project_dir}/{configFileRelativePath}"
    }},
    "outputStem": "Punjab_Small_merged",
    "taskID":1 
}}'''

def createAnalysisExample(project_dir:Path,working_dir: Path)->str:
    return f'''{{
    "shpFile": {{
        "class": "File",
        "path": "{working_dir}/Punjab_Small_merged.shp",
        "secondaryFiles": [
            {{"class": "File", "path": "{working_dir}/Punjab_Small_merged.shx"}},
            {{"class": "File", "path": "{working_dir}/Punjab_Small_merged.dbf"}},
            {{"class": "File", "path": "{working_dir}/Punjab_Small_merged.prj"}}
        ]
    }},
    "config":{{
        "class":"File",
        "path": "{project_dir}/{configFileRelativePath}"
    }},
    "outputStem": "Punjab_Small_Analysis"
}}'''

def writeJSONFile(content:str, filename: str):
    if not filename.endswith(".json"):
        filename += ".json"
    filename = "jobs/"+filename
    with open(filename,"w") as f:
        f.write(content)

def createJobs(project_dir: Path, working_dir: Path):
    local_config = project_dir / configFileRelativePath
    if (not os.path.exists(local_config)):
        runpy.run_path(str(local_config.parent / "create-local-config.py"))
    writeJSONFile(createFilterExample(project_dir),"filterExample")
    writeJSONFile(createNeighboursExample(project_dir,working_dir),"neighboursExample")
    writeJSONFile(createContractionExample(project_dir,working_dir),"contractionExample")
    writeJSONFile(createAnalysisExample(project_dir,working_dir),"analysisExample")

def runExample(project_dir: Path, working_dir: Path):
    shutil.rmtree(working_dir)
    os.makedirs(working_dir, exist_ok=True)
    os.chdir(working_dir)
    os.system(f"cwltool {project_dir}/cwl/sda/filter.cwl ../jobs/filterExample.json")
    os.system(f"cwltool {project_dir}/cwl/sda/neighbours.cwl ../jobs/neighboursExample.json")
    os.system(f"cwltool {project_dir}/cwl/sda/contraction.cwl ../jobs/contractionExample.json")
    os.system(f"cwltool {project_dir}/cwl/sda/analysis.cwl ../jobs/analysisExample.json")

def main():
    current_dir = Path(os.path.abspath(__file__)).parent
    working_dir = current_dir / WORKING_DIR_NAME
    if (not os.path.exists(working_dir)):
        os.mkdir(working_dir)
    job_dir = current_dir / "jobs"
    if(not os.path.exists(job_dir)):
        os.mkdir(job_dir)
    project_dir = get_project_directory()
    createJobs(project_dir, working_dir)
    runExample(project_dir,working_dir)

if __name__ == "__main__":
    main()
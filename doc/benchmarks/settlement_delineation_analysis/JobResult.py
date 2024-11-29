import json
from datetime import timedelta
from enum import Enum
import re

class JobType(Enum):
    SPLIT = 0
    FILTER = 1
    NEIGHBOURS = 2
    COMPONENTS = 3
    CONTRACTION = 4
    ANALYSIS = 5
    MERGE = 6
    ORCHESTRATION = 7

class JobResult:
    def __init__(self, type: JobType, duration: timedelta , attributes):
        self.type = type
        self.duration = duration
        self.attributes = attributes

    def __repr__(self):
        return f"JobResult(type={self.type}, duration={self.duration})"

def map_job_type(job_type_str: str) -> JobType:
    job_type_mapping = {
        "SPLIT": JobType.SPLIT,
        "FILTER": JobType.FILTER,
        "NEIGHBOURS": JobType.NEIGHBOURS,
        "COMPONENTS": JobType.COMPONENTS,
        "CONTRACTION": JobType.CONTRACTION,
        "ANALYSIS": JobType.ANALYSIS,
        "MERGE": JobType.MERGE
    }
    return job_type_mapping.get(job_type_str.upper(), None)

def parse_job_type_from_json(json_data: str) -> JobType:
    try:
        job_type_str = json_data.get('type')
        if job_type_str is not None:
            return map_job_type(job_type_str)
        else:
            return JobType.SPLIT
            # raise ValueError("Job type attribute not found in JSON data")
    except json.JSONDecodeError as e:
        raise ValueError("Invalid JSON data") from e
def parse_job(path:str)->JobResult:
    with open(path, 'r') as f:
        json_str = f.read()
        if len(json_str)==0:
            return None
        json_str = re.sub(r'([{,]\s*)([a-zA-Z0-9_\[\]]+)(\s*):', r'\1"\2":', json_str)
        log = json.loads(json_str)
        return JobResult(parse_job_type_from_json(log),timedelta(seconds=log["duration[s]"]), log)

if __name__ == '__main__':
    path = "/home/lolo/Documents/results/Baseline/logs/filter.cwl/WSF2019_v1_10_48_0_0_filter_stdout.log"
    job = parse_job(path)
    print(job.duration)
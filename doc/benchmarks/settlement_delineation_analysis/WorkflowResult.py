import math
from typing import List
from datetime import datetime
from JobResult import *
from SlurmReport import *
import os
import glob


class WorkflowResult:
    graph_properties = ["Connected Components", "#Nodes-after-contraction", "polygon count", "Adjacencies"]

    def init_stats(self):
        stats = {}
        for f in WorkflowResult.graph_properties:
            stats[f] = 0
        for jobs in self.jobs.values():
            for job in jobs:
                for f in WorkflowResult.graph_properties:
                    if job.attributes.__contains__(f):
                        stats[f] += int(job.attributes[f])
        return stats

    def __init__(self, name: str, config, jobs: List[JobResult], report: SlurmReport):
        self.name = name
        self.config = config
        self.report = report
        self.jobs = {}
        jobs_time = sum([job.duration for job in jobs], timedelta(seconds=0))
        overhead = abs(report.cpu_time - jobs_time)
        self.jobs[JobType.ORCHESTRATION] = [
            JobResult(JobType.ORCHESTRATION, overhead, {"duration[s]": overhead})]
        for job in jobs:
            if job.type not in self.jobs:
                self.jobs[job.type] = []
            self.jobs[job.type].append(job)
        self.stats = self.init_stats()

    def total_cpu_time_per_type(self, type: JobType):
        return sum([job.duration for job in self.jobs.get(type)], timedelta(seconds=0))

    def cpu_time_percentage_per_type(self, type: JobType):
        return self.total_cpu_time_per_type(type).total_seconds() / self.report.cpu_time.total_seconds()

    def cpu_time_percentages(self):
        return {job_type.name: self.cpu_time_percentage_per_type(job_type) for job_type in JobType}

    def cpu_times(self):
        return {job_type.name: self.total_cpu_time_per_type(job_type) for job_type in JobType}

    def get_jobs(self, type: JobType):
        return self.jobs.get(type)

    def min(self, type: JobType):
        return min(self.get_jobs(type), key=lambda x: x.duration)

    def max(self, type: JobType):
        return max(self.get_jobs(type), key=lambda x: x.duration)

    def avg(self, type: JobType):
        return sum([job.duration for job in self.jobs.get(type)], timedelta(seconds=0)) / len(self.jobs.get(type))

    def var(self, type: JobType):
        avg = self.avg(type)
        return sum([timedelta(seconds=(job.duration - avg).total_seconds() ** 2) for job in self.jobs.get(type)],
                   timedelta(seconds=0)) / len(self.jobs.get(type))

    def std(self, type: JobType):
        return timedelta(seconds=math.sqrt(self.var(type).total_seconds()))

    def count(self, type: JobType):
        return len(self.jobs.get(type))


def parse(directory: str) -> WorkflowResult:
    json_files = glob.glob(os.path.join(directory, '*.json'))
    if not json_files:
        raise FileNotFoundError("No .json file found in the specified directory")
    # Assuming there is only one .json file
    json_file_path = json_files[0]

    # Find the slurm.txt file
    slurm_file_path = os.path.join(directory, 'slurm.txt')
    if not os.path.exists(slurm_file_path):
        raise FileNotFoundError("slurm.txt file not found in the specified directory")

    # stdout_file = os.path.join(directory,"stdout.log")
    # jobs = []
    # with open(stdout_file, 'r') as stdout:
    #     out = stdout.read()
    #     duration_pattern = r'"duration\[s\]":\s*([\d.]+)'
    #
    #     # Search for the duration in the log content
    #     match = re.search(duration_pattern, out)
    #     if match:
    #         # Convert the matched duration value to a float
    #         duration = float(match.group(1))
    #         orchestration_job = JobResult(JobType.ORCHESTRATION,timedelta(seconds=duration),{"duration[s]":duration})
    #         jobs.append(orchestration_job)

    # Load JSON config file
    with open(json_file_path, 'r') as json_file:
        config = json.load(json_file)
    # Load report file
    slurm_report = parse_report(slurm_file_path)
    # Load job files
    log_directory = os.path.join(directory, 'logs')
    job_directories = [os.path.join(log_directory, job_log_dir)
                       for job_log_dir in os.listdir(log_directory)
                       if os.path.isdir(os.path.join(log_directory, job_log_dir))
                       ]
    job_files = [
        os.path.join(log_directory, job_directory, file)
        for job_directory in job_directories
        for file in os.listdir(job_directory)
        if file.endswith("stdout.log")
    ]
    jobs = [job for job in (parse_job(file) for file in job_files) if job is not None]
    name = os.path.basename(directory)
    return WorkflowResult(name, config, jobs, slurm_report)


def load_directory(directory: str) -> List[WorkflowResult]:
    workflow_results = []
    # Iterate through each subdirectory in the root directory
    for subdir in os.listdir(directory):
        subdir_path = os.path.join(directory, subdir)
        if os.path.isdir(subdir_path):  # Ensure it's a directory
            workflow_result = parse(subdir_path)
            if workflow_result is not None:  # Ensure the parsing was successful
                workflow_results.append(workflow_result)
    return workflow_results


if __name__ == '__main__':
    result = parse(
        "C:\\Users\\Lolo\\OneDrive\\Dokumente\\Master Informatik\\5_WS23\\2024_MA_Lorenz_Gruber\\Results\\Parameter Impact\\Baseline")
    type = JobType.NEIGHBOURS
    print(result.name)
    print(result.avg(type))
    print(result.std(type))
    print(result.max(type))
    print(result.min(type))
    print(result.count(type))

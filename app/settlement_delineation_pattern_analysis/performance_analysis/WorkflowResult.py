import math
from typing import List
from datetime import datetime
from JobResult import *
from SlurmReport import *
import os
import glob


class WorkflowResult:
    def __init__(self, name: str, config, jobs: List[JobResult], report: SlurmReport):
        self.name = name
        self.config = config
        self.report = report
        self.jobs = {}
        jobs_time = sum([job.duration for job in jobs], timedelta(seconds=0))
        orchestration_time = report.cpu_time - jobs_time
        self.jobs[JobType.ORCHESTRATION] = [
            JobResult(JobType.ORCHESTRATION, orchestration_time, {"duration[s]": orchestration_time})]
        for job in jobs:
            if job.type not in self.jobs:
                self.jobs[job.type] = []
            self.jobs[job.type].append(job)

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
    jobs = [parse_job(file) for file in job_files]
    name = os.path.basename(directory)
    return WorkflowResult(name, config, jobs, slurm_report)


if __name__ == '__main__':
    result = parse("/home/lolo/Documents/results/Baseline")
    type = JobType.NEIGHBOURS
    print(result.name)
    print(result.avg(type))
    print(result.std(type))
    print(result.max(type))
    print(result.min(type))
    print(result.count(type))

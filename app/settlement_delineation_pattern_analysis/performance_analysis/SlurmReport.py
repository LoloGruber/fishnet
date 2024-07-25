import re
from datetime import timedelta


class SlurmReport:
    def __init__(self, cores_per_node: int, cpu_time: timedelta, run_time: timedelta, memory_usage: float,
                 cpu_efficiency: float, memory_efficiency: float):
        self.cores_per_node = cores_per_node
        self.cpu_time = cpu_time
        self.run_time = run_time
        self.memory_usage = memory_usage
        self.cpu_efficiency = cpu_efficiency
        self.memory_efficiency = memory_efficiency

    def __repr__(self):
        return (f"SlurmReport(cores_per_node={self.cores_per_node}, cpu_time={self.cpu_time}, "
                f"run_time={self.run_time}, memory_usage={self.memory_usage} GB, "
                f"cpu_efficiency={self.cpu_efficiency}%, memory_efficiency={self.memory_efficiency}%)")


def parse_timedelta(time_str):
    """Parses a time string in the format 'DD-HH:MM:SS' or 'HH:MM:SS' into a timedelta object."""
    if '-' in time_str:
        days, time_part = time_str.split('-')
        days = int(days)
    else:
        days = 0
        time_part = time_str
    hours, minutes, seconds = map(int, time_part.split(':'))
    return timedelta(days=days, hours=hours, minutes=minutes, seconds=seconds)

def parse_report_string(report: str) -> SlurmReport:
    cores_per_node = int(re.search(r'Cores per node:\s+(\d+)', report).group(1))
    cpu_time_str = re.search(r'CPU Utilized:\s+([\d:-]+)', report).group(1)
    cpu_time = parse_timedelta(cpu_time_str)

    run_time_str = re.search(r'Job Wall-clock time:\s+([\d:-]+)', report).group(1)
    run_time = parse_timedelta(run_time_str)

    memory_usage = float(re.search(r'Memory Utilized:\s+([\d.]+) GB', report).group(1))

    cpu_efficiency = float(re.search(r'CPU Efficiency:\s+([\d.]+)%', report).group(1))

    memory_efficiency = float(re.search(r'Memory Efficiency:\s+([\d.]+)%', report).group(1))

    return SlurmReport(
        cores_per_node=cores_per_node,
        cpu_time=cpu_time,
        run_time=run_time,
        memory_usage=memory_usage,
        cpu_efficiency=cpu_efficiency,
        memory_efficiency=memory_efficiency
    )


def parse_report(path: str) -> SlurmReport:
    with open(path, "r") as file:
        report = file.read()
        return parse_report_string(report)


if __name__ == '__main__':
    path = "/home/lolo/Documents/results/Baseline/slurm.txt"
    report = parse_report(path)
    print(report)

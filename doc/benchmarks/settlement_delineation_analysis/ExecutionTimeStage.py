from WorkflowResult import *
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Patch

def aggregate_job_durations(workflow_results, job_type):
    durations = []
    for result in workflow_results:
        if job_type in result.jobs:
            durations.extend([job.duration for job in result.get_jobs(job_type)])
    return durations


def plot_min_avg_max_computation_sorted(workflow_results: List[WorkflowResult]):
    # Define JobType order excluding ORCHESTRATION
    job_type_order = [job_type for job_type in JobType if job_type != JobType.ORCHESTRATION]

    job_types = []
    min_durations = []
    avg_durations = []
    max_durations = []
    job_type_names = []
    durations_per_stage = []

    for job_type in job_type_order:
        # Aggregate durations for the current job type
        durations = aggregate_job_durations(workflow_results, job_type)
        if durations:
            durations_per_stage.append([d.total_seconds() for d in durations])
            job_types.append(job_type)
            durations_in_seconds = [d.total_seconds() for d in durations]
            # min_durations.append(min(durations_in_seconds))
            # avg_durations.append(np.mean(durations_in_seconds))
            # max_durations.append(max(durations_in_seconds))
            job_type_names.append(f"{job_type.name} ({len(durations)})")
    # Create the plot
    x = np.arange(len(job_types))  # label locations
    width = 0.2  # width of the bars

    plt.figure(figsize=(12, 8))
    plt.gca().set_axisbelow(True)
    plt.grid(which='both', linestyle='--', linewidth=0.5)
    parts = plt.violinplot(durations_per_stage, showmeans=False, showmedians=False, showextrema=False)

    colors = plt.cm.viridis(np.linspace(0, 1, len(durations_per_stage)))

    for i, pc in enumerate(parts['bodies']):
        pc.set_facecolor(colors[i])
        pc.set_edgecolor('black')
        pc.set_alpha(0.7)

    # Plotting medians, averages, min, and max manually
    for i, data in enumerate(durations_per_stage):
        min_val = np.min(data)
        max_val = np.max(data)
        median_val = np.median(data)
        mean_val = np.mean(data)

        plt.scatter([i + 1], [min_val], color='red', marker='o', label='Min' if i == 0 else "",zorder=3)
        plt.scatter([i + 1], [max_val], color='blue', marker='o', label='Max' if i == 0 else "",zorder=3)
        plt.scatter([i + 1], [median_val], color='green', marker='D', label='Median' if i == 0 else "",zorder=3)
        plt.scatter([i + 1], [mean_val], color='orange', marker='s', label='Mean' if i == 0 else "",zorder=3)

    # plt.bar(x - width, min_durations, width, label='Min', color='lightgreen', zorder=3)
    # plt.bar(x, avg_durations, width, label='Avg', color='skyblue', zorder=3)
    # plt.bar(x + width, max_durations, width, label='Max', color='salmon', zorder=3)

    # Logarithmic y-axis
    plt.yscale('log')
    plt.minorticks_on()
    # Add labels, title, and legend
    plt.xlabel('Job Type (#Samples)', fontsize=16)
    plt.ylabel('Computation time log(s)', fontsize=16)
    # plt.title('Min, Avg, and Max Computation Time per Job Type')
    plt.xticks(np.arange(1, len(job_type_names) + 1), job_type_names, rotation=45, fontsize=14)
    # Custom legend for points
    handles = [
        Patch(color='red', label='Min'),
        Patch(color='blue', label='Max'),
        Patch(color='orange', label='Average'),
        Patch(color='green', label='Median')
    ]
    plt.legend(handles=handles, fontsize=14)

    # Show the plot
    plt.tight_layout()
    plt.savefig("workflow-stage-execution-time.pdf")
    plt.show()

def get_resource_stats(workflows: List[WorkflowResult], delimiter = ' & ', newline = ' \\\\\n'):
    header = ["Resource","MIN","MAX","AVG","Median"]
    output = delimiter.join(header) + newline
    properties = ["\\ac{TTR} [s]","\\ac{CPU} Time [s]","CPU Efficiency", "Memory Usage","Memory Efficiency"]
    ttrs = np.array([r.report.run_time.total_seconds() for r in workflows])
    cputimes= np.array([r.report.cpu_time.total_seconds() for r in workflows])
    cpueff = np.array([r.report.cpu_efficiency for r in workflows])
    mem = np.array([r.report.memory_usage for r in workflows])
    memeff = np.array([r.report.memory_efficiency for r in workflows])
    distributions = [ttrs,cputimes,cpueff,mem,memeff]
    for i,resource in enumerate(properties):
        distribution = distributions[i]
        min = timedelta(seconds=np.min(distribution))
        max = timedelta(seconds=np.max(distribution))
        avg = timedelta(seconds=int(np.average(distribution)))
        median = timedelta(seconds=int(np.median(distribution)))
        min = np.min(distribution)
        max = np.max(distribution)
        avg = np.average(distribution)
        median = np.median(distribution)

        output += delimiter.join([resource,f"{min:,.2f}",f"{max:,.2f}",f"{avg:,.2f}",f"{median:,.2f}"]) + newline
    print(output)


if __name__ == '__main__':
    rootDir = "C:\\Users\\Lolo\\OneDrive\\Dokumente\\Master Informatik\\5_WS23\\2024_MA_Lorenz_Gruber\\Results\\Settlement Distribution"
    varyingConfig ="C:\\Users\\Lolo\\OneDrive\\Dokumente\\Master Informatik\\5_WS23\\2024_MA_Lorenz_Gruber\\Results\\Parameter Impact"
    workflows = load_directory(rootDir) + load_directory(varyingConfig)
    for r in workflows:
        if r.name == "Baseline":
            workflows.remove(r)
    plot_min_avg_max_computation_sorted(workflows)
    # get_resource_stats(workflows)
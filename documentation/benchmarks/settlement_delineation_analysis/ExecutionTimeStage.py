from WorkflowResult import *
import numpy as np
import matplotlib.pyplot as plt


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

    for job_type in job_type_order:
        # Aggregate durations for the current job type
        durations = aggregate_job_durations(workflow_results, job_type)
        if durations:
            job_types.append(job_type)
            durations_in_seconds = [d.total_seconds() for d in durations]
            min_durations.append(min(durations_in_seconds))
            avg_durations.append(np.mean(durations_in_seconds))
            max_durations.append(max(durations_in_seconds))
            job_type_names.append(f"{job_type.name} ({len(durations)})")
    # Create the plot
    x = np.arange(len(job_types))  # label locations
    width = 0.2  # width of the bars

    plt.figure(figsize=(12, 8))

    plt.bar(x - width, min_durations, width, label='Min', color='lightgreen', zorder=3)
    plt.bar(x, avg_durations, width, label='Avg', color='skyblue', zorder=3)
    plt.bar(x + width, max_durations, width, label='Max', color='salmon', zorder=3)

    # Logarithmic y-axis
    plt.yscale('log')
    plt.minorticks_on()
    plt.grid(which='both', linestyle='--', linewidth=0.5)
    # Add labels, title, and legend
    plt.xlabel('Job Type (#Samples)', fontsize=16)
    plt.ylabel('Computation time log(s)', fontsize=16)
    # plt.title('Min, Avg, and Max Computation Time per Job Type')
    plt.xticks(x, job_type_names, rotation=45, fontsize=14)
    plt.legend(fontsize=14)

    # Show the plot
    plt.tight_layout()
    plt.savefig("workflow-stage-execution-time.pdf")
    plt.show()


if __name__ == '__main__':
    rootDir = "C:\\Users\\Lolo\\OneDrive\\Dokumente\\Master Informatik\\5_WS23\\2024_MA_Lorenz_Gruber\\Results\\Settlement_Distribution"
    varyingConfig ="C:\\Users\\Lolo\\OneDrive\\Dokumente\\Master Informatik\\5_WS23\\2024_MA_Lorenz_Gruber\\Results\\Parameter Impact"
    workflows = load_directory(rootDir) + load_directory(varyingConfig)
    for r in workflows:
        if r.name == "Baseline":
            workflows.remove(r)
    plot_min_avg_max_computation_sorted(workflows)

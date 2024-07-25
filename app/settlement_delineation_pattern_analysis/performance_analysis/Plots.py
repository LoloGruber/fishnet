import matplotlib.pyplot as plt
import numpy as np
from WorkflowResult import *


def plot_stacked_step_times(results: List[WorkflowResult]):
    step_times = {jobType.name: [] for jobType in JobType}
    runs = [result.name for result in results]
    for result in results:
        for type,time in result.cpu_times().items():
            step_times[type].append(time.total_seconds())
    workflow_steps = step_times.keys()

    # Prepare data for plotting
    num_steps = len(workflow_steps)
    num_runs = len(runs)

    # Convert step times into a numpy array for easier manipulation
    step_times_array = np.array([step_times[step] for step in workflow_steps])

    # Create an array of x positions for each run
    x = np.arange(num_runs)

    # Create the plot
    fig, ax = plt.subplots(figsize=(12, 7))

    # Initialize bottom positions
    bottoms = np.zeros(num_runs)

    # Plot each step's times as a stacked bar
    for i, step in enumerate(workflow_steps):
        ax.bar(x, step_times_array[i], bottom=bottoms, label=step)
        bottoms += step_times_array[i]
    for i in range(num_runs):
        accumulated_time = timedelta(seconds=bottoms[i])  # Total time for the run
        ax.text(
            x[i],  # x position (center of the column)
            accumulated_time.total_seconds() + 1,  # y position (slightly above the top of the stack)
            f'{accumulated_time}',  # text (accumulated time)
            ha='center',  # horizontal alignment
            va='bottom',  # vertical alignment
            fontsize=10,
            color='black'
        )
    # Adding labels and title
    ax.set_xlabel('Workflow Runs')
    ax.set_ylabel('Accumulated CPU Time in Seconds')
    ax.set_title('CPU Times by Run for Different Workflow Steps')
    ax.set_xticks(x)
    ax.set_xticklabels(runs)
    ax.legend(title='Workflow Steps')
    # Show the plot
    fig.show()
    return fig

def plot_stats_of_type(results: List[WorkflowResult], type: JobType):
    averages = np.array([ result.avg(type).total_seconds() for result in results])
    std = np.array([result.std(type).total_seconds() for result in results])
    x = np.arange(len(results))  # the label locations
    width = 0.6  # the width of the bars

    fig, ax = plt.subplots(figsize=(10, 6))
    for i in range(len(results)):
        ax.errorbar(x[i], averages[i], yerr=std[i], fmt='o', color='black', capsize=5, label=results[i].name)


    # Add labels and title
    ax.set_xlabel('Run')
    ax.set_ylabel('Runtime (seconds)')
    ax.set_title('Average and Standard Deviation of Job Runtimes Across Runs')
    ax.set_xticks(x)
    ax.set_ylim(bottom=0)
    ax.set_xticklabels([result.name for result in results])
    ax.grid(True, linestyle='--', alpha=0.7)

    # Show plot
    fig.show()

def get_stats_table(results: List[WorkflowResult], type: JobType, delimiter = ' & ', newline = ' \\\\'):
    header = ["Run", "N","MIN","AVG","MAX","STD"]
    output = delimiter.join(header) + newline
    for result in results:
        output += delimiter.join([result.name,str(len(result.get_jobs(type))),str(result.min(type).duration),str(result.avg(type)),str(result.max(type).duration),str(result.std(type))]) + newline
    return output



if __name__ == '__main__':
    baseline = parse("/home/lolo/Documents/results/Baseline")
    contraction1000 = parse("/home/lolo/Documents/results/Contraction_Distance_1000")
    contraction200 = parse("/home/lolo/Documents/results/Contraction_Distance_200")
    results = [baseline,contraction1000,contraction200]
    plot_stats_of_type(results,JobType.ANALYSIS)
    fig = plot_stacked_step_times(results)
    fig.savefig("./Example.png")
    print(get_stats_table(results,JobType.ANALYSIS,"\t","\n"))


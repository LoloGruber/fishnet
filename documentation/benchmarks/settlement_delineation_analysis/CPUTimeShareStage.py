from typing import List
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
from WorkflowResult import *
def plot_runtime_log(results:List[WorkflowResult]):
    fig, ax = plt.subplots(figsize=(12, 7))
    x = np.array([r.report.run_time.total_seconds() for r in results])
    custom_ticks = [1e-3, 1e-2, 1e-1, 1, 10, 60, 600, 3600, 36000, 86400, 2 * 86400]  # 1ms to 2 days
    mask = {
        1e-3:10,
        1e-2:10,
        1e-1:10,
        1: 10,
        10:6,
        60:10,
        600:10,
        3600:24,
        36000:0,
        86400:7,
        2*86400:0
    }
    minor_ticks = []
    def factor(seconds):
        if seconds < 1:
            return 1e-3
        if seconds < 60:
            return 1
        if seconds < 3600:
            return 60
        if seconds < 86400:
            return 3600
        else:
            return 86400
    for tick in custom_ticks:
        for number in range(1,mask[tick]):
            minor_ticks.append(factor(tick)*number)
    ax.set_xticks(minor_ticks,minor=True)

    colors = plt.cm.Paired(np.linspace(0, 1, len(x)))

    # Plot each bar individually to control color
    for i, (val, color) in enumerate(zip(x, colors)):
        ax.barh(i, val, color=color, log=True,zorder=3)
    ax.set_yticks(np.arange(len(results)))
    ax.set_yticklabels([r.name for r in results])
    custom_ticks = [1e-3, 1e-2, 1e-1, 1, 10, 60, 600, 3600, 36000, 86400, 2*86400]  # 1ms to 2 days

    # Set x-ticks and labels
    ax.set_xticks(custom_ticks)
    def format_func(value, tick_number):
        if value < 1:
            return f'{value * 1e3:.0f}ms'  # Milliseconds
        elif value < 60:
            return f'{value:.0f}s'  # Seconds
        elif value < 3600:
            return f'{value / 60:.0f}min'  # Minutes
        elif value < 86400:
            return f'{value / 3600:.0f}h'  # Hours
        else:
            return f'{value / 86400:.0f}d'  # Days

    ax.xaxis.set_major_formatter(plt.FuncFormatter(format_func))
    # # Adding labels and title
    ax.set_ylabel('Input Region', fontsize=16)
    ax.set_xlabel('Time to Result (log)',fontsize=16)
    # # ax.set_title('CPU Times by Run for Different Workflow Steps')
    # Add a grid
    ax.grid(True, which='major', axis='x', linestyle='-', color='gray', alpha=0.95)  # Major grid with solid lines
    ax.grid(True, which='minor', axis='x', linestyle='--', color='gray', alpha=0.5)  # Minor
    ax.minorticks_on()
    # Show the plot
    fig.tight_layout()
    fig.show()
    return fig

def plot_stage_distribution(results:List[WorkflowResult]):
    fig, ax = plt.subplots(figsize=(12, 7))
    x = np.array([[r.cpu_time_percentage_per_type(stage) for stage in JobType] for r in results])
    # Accumulate values for stacking
    bottoms = np.zeros(len(results))
    # Plot each workflow stage as a separate bar on top of the previous one
    for i, stage in enumerate(JobType):
        stage_values = x[:, i]
        ax.barh(np.arange(len(results)), stage_values, left=bottoms,
                label=stage.name, zorder=3)
        bottoms += stage_values

    ax.set_yticks(np.arange(len(results)))
    ax.set_yticklabels([r.name +f"\n({r.report.cpu_time})" for r in results])
    # Set x-ticks for every 10 percent
    ax.set_xticks(np.arange(0, 1.1, 0.1))
    ax.set_xticklabels(["{:.2f}%".format(i*100) for i in np.arange(0,1.1,0.1)])
    # # Adding labels and title
    ax.set_ylabel('Input Region with Acc. CPU Time', fontsize=16)
    ax.set_xlabel('CPU time percentage',fontsize=16)
    # # ax.set_title('CPU Times by Run for Different Workflow Steps')
    # Add a grid
    ax.grid(True, which='major', axis='x', linestyle='-', color='gray', alpha=0.95)  # Major grid with solid lines
    # ax.grid(True, which='minor', axis='x', linestyle='--', color='gray', alpha=0.5)  # Minor
    ax.legend(title='Workflow Stage',title_fontsize=16)
    ax.minorticks_on()
    # Show the plot
    fig.tight_layout()
    fig.show()
    return fig



def plot_stacked_step_times(results: List[WorkflowResult]):
    fig, ax = plt.subplots(figsize=(12, 7))
    # Set y-axis
    y = np.arange(len(results))
    # Initialize left positions
    total_times = np.zeros(len(results))
    # Plot each workflow stage times as a stacked horizontal bar
    for stage in JobType:
        cpu_time_of_stages = [r.total_cpu_time_per_type(stage).total_seconds() for r in results]
        ax.barh(y,cpu_time_of_stages,left=total_times, label=stage.name, zorder=3)
        total_times += cpu_time_of_stages
    # Plot text with background and ensure it is on top
    for i,r in enumerate(results):
        accumulated_time = total_times[i]  # Total time for the run in seconds
        ax.text(
            accumulated_time + 1000,  # x position (slightly to the right of the stack)
            y[i],  # y position (center of the bar)
            f'{timedelta(seconds=accumulated_time)}',  # text (accumulated time in HH:MM:SS format)
            ha='left',  # horizontal alignment
            va='center',  # vertical alignment
            fontsize=10,
            fontweight='bold',  # Make the text bold
            color='black',
            bbox=dict(facecolor='white', edgecolor='none', alpha=0.7),  # Background for readability
            zorder=10  # Ensure text is on top of grid lines
        )

    # Set x-axis to logarithmic scale
    # ax.set_xscale('log')

    # Set x-ticks to hours
    max_time_in_seconds = total_times.max()
    x_ticks = np.arange(0, max_time_in_seconds + 3600, 3600)  # Tick every hour

    ax.set_xticks(x_ticks)
    ax.set_xticklabels([f'{int(x/3600)}h' for x in x_ticks])

    # Set the y-tick labels to the names of the runs
    ax.set_yticks(y)
    ax.set_yticklabels([r.name for r in results])
    # Adding labels and title
    ax.set_ylabel('Workflow Runs', fontsize=16)
    ax.set_xlabel('Accumulated CPU Time',fontsize=16)
    # ax.set_title('CPU Times by Run for Different Workflow Steps')
    ax.legend(title='Workflow Stage',title_fontsize=16)
    # Add a grid
    ax.grid(True, axis='x', linestyle='--', color='gray', alpha=0.7)
    # Show the plot
    fig.tight_layout()
    fig.show()
    return fig


def get_stats_table(results: List[WorkflowResult], type: JobType, delimiter = ' & ', newline = ' \\\\'):
    header = ["Run", "N","MIN","AVG","MAX","STD"]
    output = delimiter.join(header) + newline
    for result in results:
        output += delimiter.join([result.name,str(len(result.get_jobs(type))),str(result.min(type).duration),str(result.avg(type)),str(result.max(type).duration),str(result.std(type))]) + newline
    return output


def get_graph_properties_as_latex_table(results: List[WorkflowResult], delimiter = ' & ', newline = ' \\\\\n'):
    header = ["Run","\\ac{TTR}","Nodes ($G$)","Edges","Components","Nodes ($G'$)"]
    output = delimiter.join(header) + newline
    counter = 1
    for result in results:
        ttr = result.report.run_time
        nodes_initially = result.stats.get("#Nodes-before-contraction")
        edges_initially = result.stats.get("Adjacencies")
        components = result.stats.get("Connected Components")
        nodes_finally = result.stats.get("#Nodes-after-contraction")
        output += delimiter.join([str(result.name),str(ttr),str(nodes_initially),str(edges_initially),str(components),str(nodes_finally)]) + newline
        counter += 1
    return output

def evaluate_workflows(directory: str, output_file: str, log_scale=False, plot_distributions_file = None):
    workflows = load_directory(directory)
    fig = plot_runtime_log(workflows) if log_scale else plot_stacked_step_times(workflows)
    fig.savefig(output_file)
    if plot_distributions_file is not None:
        dist = plot_stage_distribution(workflows)
        fig.savefig(plot_distributions_file)
    tab = get_graph_properties_as_latex_table(workflows)
    print(tab)



if __name__ == '__main__':
    cfg = "C:\\Users\\Lolo\\OneDrive\\Dokumente\\Master Informatik\\5_WS23\\2024_MA_Lorenz_Gruber\\Results\\Parameter Impact"
    input = "C:\\Users\\Lolo\\OneDrive\\Dokumente\\Master Informatik\\5_WS23\\2024_MA_Lorenz_Gruber\\Results\\Settlement Distribution"
    evaluate_workflows(cfg,"cpu-time-per-stage-different-config.pdf")
    evaluate_workflows(input,"ttr-different-input.pdf",True,"workflow-stage-percentage-different-inputs.pdf")



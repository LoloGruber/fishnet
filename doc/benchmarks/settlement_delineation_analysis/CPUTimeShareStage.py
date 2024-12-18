from typing import List
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
from WorkflowResult import *
from matplotlib.ticker import LogLocator, NullFormatter
def plot_runtime_log(results:List[WorkflowResult]):
    fig, ax = plt.subplots(figsize=(12, 7))
    x = np.array([r.report.run_time.total_seconds() for r in results])
    custom_ticks = [60, 600, 3600, 36000, 86400, 2 * 86400,3*86400]  # 1ms to 2 days

    minor_ticks = np.concatenate([
        np.arange(120, 600, 60),       # 1 minute to 10 minutes
        np.arange(1200, 3600, 600),    # 10 minutes to 1 hour
        np.arange(7200, 86400, 3600), # 1 hour to 1 day
    ])
    labeled_minor_ticks = [120,180,300,1200,1800,7200,10800,18000]

    ax.set_xscale('log')
    # Set x-ticks and labels
    ax.set_xticks(custom_ticks)
    ax.set_xticks(minor_ticks,minor=True)
    def get_best_time_unit(value):
        if value < 1:
            return value*1e-3
        elif value < 60:
            return value
        elif value < 3600:
            return value / 60.0
        elif value < 86400:
            return value / 3600
        else:
            return value / 86400

    def minor_formatter(value, tick):
        unit = get_best_time_unit(value)
        if value in labeled_minor_ticks:
            return f'{unit:.0f}'
        return ""



    def time_formatter(value, tick_number):
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
    ax.xaxis.set_major_formatter(plt.FuncFormatter(time_formatter))
    ax.xaxis.set_minor_formatter(plt.FuncFormatter(minor_formatter))  # Hide minor tick labels
    ax.tick_params(axis='x', which='major', labelsize=14)  # Major ticks
    ax.tick_params(axis='x', which='minor', labelsize=14)
    # Y Axis
    colors = plt.cm.Greys(np.linspace(0, 1, len(x)))
    for i, (val, color) in enumerate(zip(x, colors)):
        ax.barh(i, val, color="black",zorder=3)
    ax.set_yticks(np.arange(len(results)))
    ax.set_yticklabels([r.name +f"\n({r.report.run_time})" for r in results],fontsize=14)

    # # Adding labels and title
    ax.set_ylabel('Input Region', fontsize=18)
    ax.set_xlabel('Time to Result (log)',fontsize=18)
    # # ax.set_title('CPU Times by Run for Different Workflow Steps')

    # Add a grid
    ax.grid(True, which='major', axis='x', linestyle='-', color='black', alpha=1,zorder=1)  # Major grid with solid lines
    ax.grid(True, which='minor', axis='x', linestyle='--', color='gray', alpha=0.5)  # Minor
    # Ensure all custom major ticks are displayed
    for tick in custom_ticks:
        ax.axvline(x=tick, color='gray', linestyle='--', linewidth=0.5, zorder=0)
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
    ax.set_xlim(0,1)
    # # Adding labels and title
    ax.set_ylabel('Input Region with Acc. CPU Time', fontsize=16)
    ax.set_xlabel('CPU time percentage',fontsize=16)
    # # ax.set_title('CPU Times by Run for Different Workflow Steps')
    # Add a grid
    ax.grid(True, which='major', axis='x', linestyle='-', color='gray', alpha=0.95)  # Major grid with solid lines
    # ax.grid(True, which='minor', axis='x', linestyle='--', color='gray', alpha=0.5)  # Minor
    ax.legend(title='Workflow Stage',title_fontsize=16,loc="lower right")
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
    labels = {
        "baseline":"Baseline",
        "edge distance 1000":"Edge Distance 1km",
        "edge distance 3000":"Edge Distance 3km",
        "edge distance 5000":"Edge Distance 5km",
        "max neighbours 1": "Max Neighbors 1",
        "max neighbours 3": "Max Neighbors 3",
        "max neighbours 5": "Max Neighbors 5",
        "max neighbours 10": "Max Neighbors 10",
        "contraction distance 200":"Contraction Distance 200m",
        "contraction distance 500":"Contraction Distance 500m",
        "contraction distance 1000": "Contraction Distance 1000m",
        "required area 10k":"Required Area 10.000 m²",
        "required area 25k":"Required Area 25.000 m²"
    }


    for i,r in enumerate(results):
        accumulated_time = total_times[i]  # Total time for the run in seconds
        ax.text(
            30000 if accumulated_time >ax.get_xlim()[1]/2 else accumulated_time +500 ,
            y[i],  # y position (center of the bar)
            f'{labels[r.name]} ({timedelta(seconds=accumulated_time)})',  # text (accumulated time in HH:MM:SS format)
            ha='left',  # horizontal alignment
            va='center',  # vertical alignment
            fontsize=14,
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
    ax.set_yticklabels([''] * len(results))
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
    output = "\\begin{tabular}{l|c|r|r|r|r}\n"
    output = """
\\begin{table}[h]
    \\centering
    \\begin{tabular}{c|c|r|r|r|r}
    TODO Title & \\ac{TTR} & Nodes ($G$) & Edges & Components & Nodes ($G'$) \\\\
    \\hline\n"""
    content = []
    for result in results:
        ttr = result.report.run_time
        nodes_initially = result.stats.get("polygon count")
        edges_initially = result.stats.get("Adjacencies")
        components = result.stats.get("Connected Components")
        nodes_finally = result.stats.get("#Nodes-after-contraction")
        row = ("\t"+delimiter.join([str(result.name),str(ttr),f"{nodes_initially:,}",f"{edges_initially:,}",f"{components:,}",f"{nodes_finally:,}"]))
        content.append(row)
    output += newline.join(content)
    output += """
    \\end{tabular}
    \\caption[TODO]{TODO}
    \\label{tab:ttr-TODO}
\\end{table}"""
    return output

def evaluate_input_regions( plot_distributions_file = None):
    input = "C:\\Users\\Lorenz\\OneDrive\\Dokumente\\Master Informatik\\5_WS23\\2024_MA_Lorenz_Gruber\\Results\\Settlement Distribution"
    workflows = load_directory(input)
    workflows.sort(key=lambda x: x.report.run_time.total_seconds())
    tab = get_graph_properties_as_latex_table(workflows)
    print(tab)
    fig = plot_runtime_log(workflows)
    fig.savefig("ttr-different-input-zoomed-bw.pdf")

def evaluate_different_cfg():
    cfg = "C:\\Users\\Lorenz\\OneDrive\\Dokumente\\Master Informatik\\5_WS23\\2024_MA_Lorenz_Gruber\\Results\\Parameter Impact"
    workflows = load_directory(cfg)
    workflows.sort(key=lambda x: x.report.cpu_time.total_seconds())
    create_custom_table(cfg)
    fig =  plot_stacked_step_times(workflows)
    fig.savefig("cpu-time-per-stage-different-config.pdf")

def create_custom_table(directory:str):
    runs = load_directory(directory)
    for i,r in enumerate(runs):
        print(f"r[{i}]: {r.name}")
    required_area = 0,8,9,
    max_neighbours = 0,5,6,7
    edge_distance = 0,3,4
    contraction_distance = 0,1,2

    configurations = [required_area,max_neighbours,edge_distance,contraction_distance]

    for indices in configurations:
        filtered_runs = []
        for i, r in enumerate(runs):
            if i in indices:
                filtered_runs.append(r)
        filtered_runs.sort(key=lambda x: x.report.run_time.total_seconds())
        tab = get_graph_properties_as_latex_table(filtered_runs)
        print(tab)


if __name__ == '__main__':
    evaluate_different_cfg()
    # evaluate_input_regions()

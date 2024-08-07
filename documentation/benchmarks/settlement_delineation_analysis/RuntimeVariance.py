from WorkflowResult import *
import numpy as np

def get_as_latex_table(results: List[WorkflowResult], delimiter = ' & ', newline = ' \\\\\n'):
    header = ["Run","Exec Time","CPU Time","Nodes ($G$)","Edges","Components","Nodes ($G'$)"]
    output = delimiter.join(header) + newline
    counter = 1
    for result in results:
        time = result.report.run_time
        cpu = result.report.cpu_time
        nodes_initially = result.stats.get("#Nodes-before-contraction")
        edges_initially = result.stats.get("Adjacencies")
        components = result.stats.get("Connected Components")
        nodes_finally = result.stats.get("#Nodes-after-contraction")
        output += delimiter.join([str(counter),str(time),str(cpu),str(nodes_initially),str(edges_initially),str(components),str(nodes_finally)]) + newline
        counter += 1
    return output


if __name__ == '__main__':
    runs =  load_directory("C:\\Users\\Lolo\\OneDrive\\Dokumente\\Master Informatik\\5_WS23\\2024_MA_Lorenz_Gruber\\Results\\Runtime Variance")
    # res = get_as_latex_table(runs)
    # print(res)
    cpu_times = np.array([r.report.cpu_time for r in runs])
    exec_times = np.array([r.report.run_time for r in runs])
    avg_cpu = np.average(cpu_times)
    avg_exec = np.average(exec_times)
    print(avg_cpu)
    print(avg_exec)
    max_delta_cpu = np.max([abs((r.report.cpu_time - avg_cpu).total_seconds() / avg_cpu.total_seconds()) for r in runs])
    print(max_delta_cpu)
    max_delta_exec = np.max([abs((r.report.run_time - avg_exec).total_seconds() / avg_exec.total_seconds()) for r in runs])
    print(max_delta_exec)

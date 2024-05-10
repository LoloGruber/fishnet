cwlVersion: v1.2
class: CommandLineTool
baseCommand: [/home/lolo/Documents/fishnet/build/app/settlement_delineation_pattern_analysis/2_neighbours/SettlementDelineationPatternAnalysisNeighbours]
requirements:
    InlineJavascriptRequirement: {}
inputs:
    shpFiles:
        type: File[]
        inputBinding:
            prefix: -i
        doc: "List of input shapefiles, with their required secondary files (.dbf, .shx, .prj)"
    config:
        type: File
        inputBinding:
            prefix: -c
        doc: "Path to configuration for neighbours task"
    taskID:
        type: int?
        doc: "Optional task id to distinguish log files"
outputs:
    standardOut:
        type: stdout
    errorOut:
        type: stderr
stdout: $(inputs.shpFiles[0].nameroot)_neighbours$(inputs.taskID==null?"":"_"+inputs.taskID)_stdout.log
stderr: $(inputs.shpFiles[0].nameroot)_neighbours$(inputs.taskID==null?"":"_"+inputs.taskID)_stderr.log
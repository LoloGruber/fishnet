cwlVersion: v1.2
class: CommandLineTool
baseCommand: [SettlementDelineationNeighbours]
requirements:
    InlineJavascriptRequirement: {}
inputs:
    primaryInput:
        type: File
        inputBinding:
            prefix: -i
        doc: "Primary input shapefile, with its required secondary files (.dbf, .shx, .prj)"
    additionalInput:
        type: File[]?
        inputBinding:
            prefix: -a
        doc: "List of additional input shapefiles in proximity to the primary input, with their required secondary files (.dbf, .shx, .prj)"
    config:
        type: File
        inputBinding:
            prefix: -c
        doc: "Path to configuration for neighbours task"
    taskID:
        type: long?
        doc: "Optional task id to distinguish log files"
outputs:
    standardOut:
        type: stdout
    errorOut:
        type: stderr
stdout: Neighbours_$(inputs.primaryInput.nameroot)$(inputs.taskID==null?"":"_"+inputs.taskID)_stdout.log
stderr: Neighbours_$(inputs.primaryInput.nameroot)$(inputs.taskID==null?"":"_"+inputs.taskID)_stderr.log
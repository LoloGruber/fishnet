cwlVersion: v1.2
class: CommandLineTool
baseCommand: [SettlementDelineationContraction]
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
        doc: "Path to configuration for contraction task"
    components:
        type: int[]?
        inputBinding:
            prefix: --components
        doc: "List of connected components to be processed by this job"
    taskID:
        type: int?
        doc: "Optional task id to distinguish log files"
    outputStem:
        type: string
        inputBinding:
            position: 2
            prefix: --outputStem 
        doc: "Output filename storing the merged polygons"
        
outputs:
    standardOut:
        type: stdout
    errorOut:
        type: stderr
    outFile:
        type: File[]
        outputBinding:
            glob: "$(inputs.outputStem).*"
        doc: "Merged output file"
stdout: $(inputs.outputStem)$(inputs.taskID==null?"":"_"+inputs.taskID)_stdout.log
stderr: $(inputs.outputStem)$(inputs.taskID==null?"":"_"+inputs.taskID)_stderr.log
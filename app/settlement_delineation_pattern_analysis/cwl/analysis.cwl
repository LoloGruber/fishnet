cwlVersion: v1.2
class: CommandLineTool
baseCommand: [SettlementDelineationAnalysis]
requirements:
    InlineJavascriptRequirement: {}
inputs:
    shpFile:
        type: File
        inputBinding:
            prefix: -i
        doc: "Input shapefile, with its required secondary files (.dbf, .shx, .prj)"
    config:
        type: File
        inputBinding:
            prefix: -c
        doc: "Path to configuration for analysis task"
    taskID:
        type: int?
        doc: "Optional task id to distinguish log files"
    outputStem:
        type: string
        inputBinding:
            prefix: --outputStem 
        doc: "Output filename stem"
        
outputs:
    standardOut:
        type: stdout
    errorOut:
        type: stderr
    outFile:
        type: File[]
        outputBinding:
            glob: $(inputs.outputStem).*
        doc: "Analysis output file"
    outEdges:
        type: File[]
        outputBinding:
            glob: $(inputs.outputStem)_edges.*
        doc: "Edges output file"
stdout: $(inputs.outputStem)$(inputs.taskID==null?"":"_"+inputs.taskID)_stdout.log
stderr: $(inputs.outputStem)$(inputs.taskID==null?"":"_"+inputs.taskID)_stderr.log
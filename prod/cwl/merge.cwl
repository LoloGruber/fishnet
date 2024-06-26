cwlVersion: v1.2
class: CommandLineTool
baseCommand: [SettlementDelineationMerge]
requirements:
    InlineJavascriptRequirement: {}
inputs:
    shpFiles:
        type: File[]
        inputBinding:
            prefix: -i
        doc: "List of input shapefiles, with their required secondary files (.dbf, .shx, .prj)"
    outputPath:
        type: string
        inputBinding:
            position: 2
            prefix: -o 
        doc: "Output filename for result (Shapefile)"
    taskID:
        type: int?
        doc: "Optional task id to distinguish log files"   
outputs:
    standardOut:
        type: stdout
    errorOut:
        type: stderr
stdout: Merge$(inputs.taskID==null?"":"_"+inputs.taskID)_stdout.log
stderr: Merge$(inputs.taskID==null?"":"_"+inputs.taskID)_stderr.log
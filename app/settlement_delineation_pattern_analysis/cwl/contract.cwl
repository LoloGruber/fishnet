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
    taskID:
        type: int?
        doc: "Optional task id to distinguish log files"
    outputDir:
        type: Directory
        inputBinding:
            prefix: --outputDir 
        doc: "Output filename storing the merged polygons"
    outputStem:
        type: string
        inputBinding:
            prefix: --outputStem 
        
outputs:
    standardOut:
        type: stdout
    errorOut:
        type: stderr
    outFile:
        type: File[]
        outputBinding:
            glob: $(inputs.outputStem).*
        doc: "Filtered output file"
stdout: $(inputs.shpFiles[0].nameroot)_contraction$(inputs.taskID==null?"":"_"+inputs.taskID)_stdout.log
stderr: $(inputs.shpFiles[0].nameroot)_contraction$(inputs.taskID==null?"":"_"+inputs.taskID)_stderr.log
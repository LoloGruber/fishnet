cwlVersion: v1.2
class: CommandLineTool
baseCommand: [/home/lolo/Documents/fishnet/build/app/settlement_delineation_pattern_analysis/1_filter/SettlementDelineationPatternAnalysisFilter]
requirements:
  InlineJavascriptRequirement: {}
inputs:
  input:
    type: File
    secondaryFiles: 
      - $(inputs.input.nameroot + ".shx")
      - $(inputs.input.nameroot + ".dbf")
      - $(inputs.input.nameroot + ".prj")
    inputBinding:
      position: 1
      prefix: -i
    doc: "Input "

  output:
    type: string
    inputBinding:
      position: 2
      prefix: -o
    doc: "Output file for the analysis"

  filterConfig:
    type: File
    inputBinding:
      position: 3
      prefix: -c
    doc: "Configuration"

outputs:
  exampleOut:
    type: stdout
  errorOut:
    type: stderr

stdout: output.log
stderr: error.log

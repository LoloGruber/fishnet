cwlVersion: v1.2
class: CommandLineTool
baseCommand: [/home/lolo/Documents/fishnet/build/app/settlement_delineation_pattern_analysis/1_filter/SettlementDelineationPatternAnalysisFilter]
requirements:
  InlineJavascriptRequirement: {}
inputs:
  shpFile:
    type: File
    secondaryFiles: 
      - $(inputs.shpFile.nameroot + ".shx")
      - $(inputs.shpFile.nameroot + ".dbf")
      - $(inputs.shpFile.nameroot + ".prj")
    inputBinding:
      position: 1
      prefix: -i
    doc: "Input shapefiles for the filter step"
    
  filterConfig:
    type: File
    inputBinding:
      position: 2
      prefix: -c
    doc: "Path to configuration for the filters"

outputs:
  standardOut:
    type: stdout
  errorOut:
    type: stderr
  outFile:
    type: File[]
    outputBinding:
      glob: "$(inputs.shpFile.nameroot)_filtered.*"
    doc: "Filtered output file"

stdout: $(inputs.shpFile.nameroot)_filter_stdout.log
stderr: $(inputs.shpFile.nameroot)_filter_stderr.log
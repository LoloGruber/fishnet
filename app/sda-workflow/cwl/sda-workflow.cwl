cwlVersion: v1.2
class: CommandLineTool
baseCommand: [SettlementDelineation]
hints:
    DockerRequirement:
        dockerPull: fishnet:1.0
inputs:
  shpFile:
    type: File
    inputBinding:
      position: 1
      prefix: -i
    doc: "Input shapefiles for the filter step"
    
  config:
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

stdout: stdout.log
stderr: stderr.log
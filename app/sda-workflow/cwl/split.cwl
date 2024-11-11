cwlVersion: v1.2
class: CommandLineTool
baseCommand: [FishnetShapefileSplitter]
requirements:
  InlineJavascriptRequirement: {}
inputs:
  shpFile:
    type: File
    inputBinding:
      position: 1
      prefix: -i
    doc: "Input shapefiles for the filter step"  
  outputDir:
    type: string
    inputBinding:
      position: 3
      prefix: -o
    doc: "Output directory"
  splits:
    type: int
    inputBinding:
      position: 2
      prefix: -s
    doc: "Number of horizontal/vertical splits"
  xOffset:
    type: int
    inputBinding:
        prefix: -x
    doc: "X offset for the naming of the output tiles"
  yOffset:
    type: int
    inputBinding:
        prefix: -y
    doc: "Y offset for the naming of the output tiles"

outputs:
  standardOut:
    type: stdout
  errorOut:
    type: stderr
  outFile:
    type: File[]
    outputBinding:
      glob: "$(inputs.shpFile.nameroot)*"
    doc: "Split output files"

stdout: $(inputs.shpFile.nameroot)_split_stdout.log
stderr: $(inputs.shpFile.nameroot)_split_stderr.log
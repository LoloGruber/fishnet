cwlVersion: v1.2
class: CommandLineTool
baseCommand: [AfricapolisGraphComponents]
requirements:
  - class: InlineJavascriptRequirement
inputs:
  config:
    type: File
    inputBinding:
      position: 1
      prefix: -c
    doc: "Path to configuration file for africapolis components step. Contains database credentials and parallelization target"
  componentOutputBasename:
    type: string
    default: "components"
    inputBinding:
      position: 2
      prefix: -o
outputs:
  components:
    type: File
    outputBinding:
      glob: "$(inputs.componentOutputBasename).json"
    doc: "Output file containing the components of the graph"
  standardOut:
    type: stdout
  errorOut:
    type: stderr

stdout: AfricapolisGraphComponents_stdout.log
stderr: AfricapolisGraphComponents_stderr.log
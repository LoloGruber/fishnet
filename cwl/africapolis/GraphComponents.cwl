cwlVersion: v1.2
class: CommandLineTool
requirements:
  - class: SchemaDefRequirement
    types: 
      - $import: ../types/Shapefile.yaml
      - $import: ClusterWorkload.yaml
  - class: InlineJavascriptRequirement
baseCommand: [AfricapolisGraphComponents]
inputs:
  config:
    type: File
    doc: "Path to configuration file for africapolis components step. Contains database credentials and parallelization target"
    inputBinding:
      position: 1
      prefix: -c
  componentOutputBasename:
    type: string
    default: "components"
    inputBinding:
      position: 2
      prefix: -o

outputs:
  clusterWorkloadFiles:
    type: File[]
    outputBinding:
      glob: "$(inputs.componentOutputBasename)*.json"
    doc: "Output file containing the components of the graph"
  standardOut:
    type: stdout
  errorOut:
    type: stderr
stdout: COMPONENTS_stdout.log
stderr: COMPONENTS_stderr.log
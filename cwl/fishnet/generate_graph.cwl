cwlVersion: v1.2
class: CommandLineTool
baseCommand: [SettlementDelineationNeighbours]
requirements:
    - $import: ../GIS.cwl
    - class: InlineJavascriptRequirement
inputs:
  primaryInput:
    type: ../GIS.cwl#Shapefile
    inputBinding:
        prefix: -i
        valueFrom: $(self.file)
    doc: "Primary input, supplied as shapefile object"
  additionalInput:
    type: ../GIS.cwl#Shapefile[]
    inputBinding:
        prefix: -a
        valueFrom: ${return self ? self.map(s=>s.file):[]}
    doc: "List of additional input shapefiles in proximity to the primary input, with their required secondary files (.dbf, .shx, .prj)"
  config:
    type: File
    inputBinding:
        prefix: -c
    doc: "Path to configuration for neighbours task. Contains graph database credentials, neighbouring criteria, ..."
outputs:
    standardOut:
        type: stdout
    errorOut:
        type: stderr
stdout: GENERATE_GRAPH_$(inputs.primaryInput.file.nameroot)_stdout.log
stderr: GENERATE_GRAPH_$(inputs.primaryInput.file.nameroot)_stderr.log
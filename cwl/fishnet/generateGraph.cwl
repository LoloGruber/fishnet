cwlVersion: v1.2
class: CommandLineTool
baseCommand: [SettlementDelineationNeighbours]
requirements:
    - class: SchemaDefRequirement
      types: 
        - $import: ../types/Shapefile.yaml
    - class: InlineJavascriptRequirement
inputs:
  primaryInput:
    type: ../types/Shapefile.yaml#Shapefile
    inputBinding:
        prefix: -i
        valueFrom: $(self.file)
    doc: "Primary input, supplied as shapefile object"
  additionalInput:
    type: ../types/Shapefile.yaml#Shapefile[]?
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
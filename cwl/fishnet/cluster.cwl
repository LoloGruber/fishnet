cwlVersion: v1.2
class: CommandLineTool
baseCommand: [SettlementDelineationContraction]
requirements:
  - class: InlineJavascriptRequirement
  - class: SchemaDefRequirement
    types:
      - $import: ../types/Shapefile.yaml
inputs:
    shpFiles:
        type: ../types/Shapefile.yaml#Shapefile[]
        inputBinding:
            prefix: -i
            valueFrom: $(self.map(shp => shp.file.path).join(" "))
        doc: "List of input shapefiles, with their required secondary files (.dbf, .shx, .prj)"
    config:
        type: File
        inputBinding:
            prefix: -c
        doc: "Path to configuration for contraction task"
    components:
        type: int[]
        inputBinding:
            prefix: --components
        doc: "List of connected components to be processed by this job"
    outputStem:
        type: string
        inputBinding:
            position: 2
            prefix: --outputStem 
        doc: "Output filename storing the merged polygons"     
outputs:
    standardOut:
        type: stdout
    errorOut:
        type: stderr
    clusteredOutput:
        type: ../types/Shapefile.yaml#Shapefile
        outputBinding:
            glob: "$(inputs.outputStem).*"
            outputEval:
                $include: ../utils/groupToShapefile.js
        doc: "Merged output file"
stdout: CLUSTER_$(inputs.components[0])_stdout.log
stderr: CLUSTER_$(inputs.components[0])_stderr.log
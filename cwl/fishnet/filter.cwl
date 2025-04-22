cwlVersion: v1.2
class: CommandLineTool
requirements:
  - $import: ../GIS.cwl
  - class: InlineJavascriptRequirement
baseCommand: [SettlementDelineationFilter]
inputs:
  gisFile:
    type: 
      # - ../GIS.cwl#GeoTIFF
      - ../GIS.cwl#Shapefile
    inputBinding:
      position: 1
      prefix: --input
      valueFrom: $(self.file)

  config:
    type: File
    doc: "Configuration file for filter process"
    inputBinding:
      prefix: --config
      position: 2
outputs:
  standardOut:
    type: stdout
  errorOut:
    type: stderr
  filtered_shapefile:
    type: ../GIS.cwl#Shapefile
    outputBinding:
      glob: "*_filtered.*"  # Gather all files associate with the shapefile
      outputEval:
        $include: ../utils/groupToShapefile.js 
stdout: Filter_$(inputs.gisFile.file.nameroot)_stdout.log
stderr: Filter_$(inputs.gisFile.file.nameroot)_stderr.log

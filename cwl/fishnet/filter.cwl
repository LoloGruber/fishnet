cwlVersion: v1.2
class: Workflow
requirements:
  - $import: ../GIS.cwl
inputs:
  gisFile:
    type: 
      # - ../GIS.cwl#GeoTIFF
      - ../GIS.cwl#Shapefile

  config:
    type: File
    doc: "Configuration file for filter process specifying exclusion criteria on shapes"

outputs:
  filtered_shapefile:
    type: ../GIS.cwl#Shapefile
    outputSource: outputToShp/shapefile
  standardOut:
    type: File
    outputSource: filter_task/standardOut
  errorOut:
    type: File
    outputSource: filter_task/errorOut

steps:
  filter_task:
    run:
      class: CommandLineTool
      baseCommand: [SettlementDelineationFilter]
      requirements: 
        InlineJavascriptRequirement: {}
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
        raw_output_files:
          type: File[]
          outputBinding:
            glob: "*_filtered.*"  # Gather all files associate with the shapefile
      stdout: Filter_$(inputs.gisFile.file.nameroot)_stdout.log
      stderr: Filter_$(inputs.gisFile.file.nameroot)_stderr.log
    in: 
      gisFile: gisFile
      config: config
    out: [raw_output_files,standardOut,errorOut]
  outputToShp:
    run: ../OutputToShapefile.cwl
    in: 
      files: 
        source: filter_task/raw_output_files
    out: [shapefile]
      
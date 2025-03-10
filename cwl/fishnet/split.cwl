cwlVersion: v1.2
class: Workflow
requirements:
- $import: ../GIS.cwl
inputs:
  gisFile:
    type: 
      - ../GIS.cwl#Shapefile
      - ../GIS.cwl#GeoTIFF
    doc: "Input shapefiles for the filter step"  
  splits:
    type: int
    doc: "Number of horizontal/vertical splits"
    
outputs:
  split_shapefiles:
    type: ../GIS.cwl#Shapefile[]
    outputSource: groupToShapefiles/shapefiles
steps:
  split:
    run:
      class: CommandLineTool
      baseCommand: [FishnetShapefileSplitter]
      requirements:
        - class: InlineJavascriptRequirement
      inputs:
        input_file:
          type: 
            - ../GIS.cwl#Shapefile
            - ../GIS.cwl#GeoTIFF
          inputBinding:
            position: 1
            prefix: --input 
            valueFrom: $(self.file)
        outputDir:
          type: string
          default: "./"
          inputBinding:
            position: 3
            prefix: -o
          doc: "Output directory"
        splits:
          type: int
          inputBinding:
            position: 2
            prefix: -s
        xOffset:
          type: int
          default: 0
          inputBinding:
              prefix: -x
          doc: "X offset for the naming of the output tiles"
        yOffset:
          type: int
          default: 0
          inputBinding:
              prefix: -y
          doc: "Y offset for the naming of the output tiles"

      outputs:
        standardOut:
          type: stdout
        errorOut:
          type: stderr
        raw_output_files:
          type: File[]
          outputBinding:
            glob: "$(inputs.input_file.file.nameroot)*"
          doc: "Split output files"

      stdout: $(inputs.input_file.file.nameroot)_split_stdout.log
      stderr: $(inputs.input_file.file.nameroot)_split_stderr.log
    in:
      input_file: gisFile
      splits: splits

    out: [raw_output_files,standardOut,errorOut]
  groupToShapefiles:
    run: ../OutputToShapefiles.cwl
    in: 
      files:
        source: split/raw_output_files
    out: [shapefiles]
    
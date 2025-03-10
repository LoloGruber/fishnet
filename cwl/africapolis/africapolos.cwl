cwlVersion: v1.2
class: Workflow
requirements:
- class: InitialWorkDirRequirement
  listing:
    - entryname: fishnet
      writable: true
      entry: ""
- class: ScatterFeatureRequirement
- class: StepInputExpressionRequirement
- class: InlineJavascriptRequirement
- class: SubworkflowFeatureRequirement
- $import: ../GIS.cwl
inputs:
  vector_input:
    type: 
      - ../GIS.cwl#Shapefile
      - ../GIS.cwl#GeoTIFF
    doc: "Input vector file to Africapolis workflow"
  config:
    type: File
    doc: "Configuration file for Africapolis workflow"
  partitions:
    type: int
    default: 4
    doc: "Number of partitions created on the input for parallel computation"
outputs:
  vector_output:
    type: ../GIS.cwl#Shapefile[]
    outputSource: filter/filtered_shapefile
steps:
  split:
    run: ../fishnet/split.cwl
    in:
      gisFile: vector_input
      # Pass the relative path "fishnet", which will resolve to the directory created by the InitialWorkDirRequirement.
      splits: partitions
    out: [split_shapefiles]
  filter:
    run: ../fishnet/filter.cwl
    in:
      gisFile:
        source: split/split_shapefiles
      config: config
    scatter: [gisFile]
    out: [filtered_shapefile]
  # flatten:
  #   run:
  #     class: ExpressionTool
  #     cwlVersion: v1.2
  #     requirements:
  #       - class: InlineJavascriptRequirement
  #     inputs:
  #       files:
  #         type:
  #           type: array
  #           items: 
  #             type: array
  #             items: File
  #     outputs:
  #       flatFiles:
  #         type: File[]
  #     expression: "$( [].concat.apply([], inputs.files) )"

  #   in:
  #     files: filter/filtered_shapefile
  #   out: [flatFiles]
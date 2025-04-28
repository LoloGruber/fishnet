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
- class: SchemaDefRequirement
  types: 
    - $import: ../types/Shapefile.yaml
    - $import: ../types/GeoTIFF.yaml
inputs:
  gisInput:
    type: 
      # - ../GIS.cwl#Shapefile
      - ../types/GeoTIFF.yaml#GeoTIFF
    doc: "Input vector file to Africapolis workflow"
  config:
    type: File
    doc: "Configuration file for Africapolis workflow"
  partitions:
    type: int
    default: 1
    doc: "Number of partitions created on the input for parallel computation"
outputs:
  vector_output:
    type: ../types/Shapefile.yaml#Shapefile[]
    outputSource: filter/filtered_shapefile
steps:
  split:
    run: ../fishnet/split.cwl
    in:
      gisFile: gisInput
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
  graph_construction:
    run: GraphConstruction.cwl
    in: 
      shapefiles: filter/filtered_shapefile
      filenamePrefix: 
        source: gisInput
        valueFrom: $(self.file.nameroot)
      config: config
    out: [standardOut]
  


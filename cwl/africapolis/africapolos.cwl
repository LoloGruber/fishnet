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
inputs:
  vector_input:
    type: File
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
    type: File[]
    outputSource: flatten/flatFiles
steps:
  split:
    run: ../util/split.cwl
    in:
      shpFile: vector_input
      # Pass the relative path "fishnet", which will resolve to the directory created by the InitialWorkDirRequirement.
      splits: partitions
    out: [outFile]
  filter:
    run: ../sda/filter.cwl
    in:
      shpFile:
        source: split/outFile
        valueFrom: >
          $(
            // self is an array of Files for one shapefile group.
            // Filter to get those with a basename ending with ".shp" and select the first one.
            (self.filter(file => file.basename && file.basename.endsWith('.shp')))[0]
          )
      config: config
    scatter: [shpFile]
    out: [outFile]
  flatten:
    run:
      class: ExpressionTool
      cwlVersion: v1.2
      requirements:
        - class: InlineJavascriptRequirement
      inputs:
        files:
          type:
            type: array
            items: 
              type: array
              items: File
      outputs:
        flatFiles:
          type: File[]
      expression: "$( [].concat.apply([], inputs.files) )"

    in:
      files: filter/outFile
    out: [flatFiles]
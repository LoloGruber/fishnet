cwlVersion: v1.2
class: Workflow
requirements:
  - class: SchemaDefRequirement
    types: 
      - $import: ../types/Shapefile.yaml
      - $import: GraphConstructionWorkload.yaml
  - class: StepInputExpressionRequirement
  - class: ScatterFeatureRequirement
  - class: InlineJavascriptRequirement
inputs:
  shapefiles:
    type: ../types/Shapefile.yaml#Shapefile[]
    doc: "List of shapefiles to be used for graph construction. Each polygon must be associated with a unique FISHNET ID."
  filenamePrefix:
    type: string?
    doc: "Prefix used to identify the shapefiles. The prefix is used to extract the grid coordinates from the filenames."
  # graph_construction_workload:
  #   type: GraphConstructionWorkload.yaml#GraphConstructionWorkload
  config:
    type: File
outputs: 
    trigger:
        type: boolean
        outputSource: done/trigger
steps:
  prepare_workload:
    run: PrepareGraphConstruction.cwl
    in:
      shapefiles: shapefiles
      filenamePrefix: filenamePrefix
    out: [graph_construction_workload]
  generate_graph:
    run: ../fishnet/generateGraph.cwl
    in:
      graph_construction_workload: prepare_workload/graph_construction_workload
      primaryInput: 
        source: prepare_workload/graph_construction_workload
        valueFrom: $(inputs.graph_construction_workload.primaryInput)
      additionalInput:
        source: prepare_workload/graph_construction_workload
        valueFrom: $(inputs.graph_construction_workload.additionalInput)
      config: config
    scatter: graph_construction_workload
    scatterMethod: dotproduct
    out: [standardOut]
  done:
    run:
      class: ExpressionTool
      cwlVersion: v1.2
      inputs:
          dummy:
            type: File[]
      outputs:
          trigger:
            type: boolean
      expression: |
          ${ return { trigger: true }; }
    in:
      dummy: generate_graph/standardOut
    out: [trigger]
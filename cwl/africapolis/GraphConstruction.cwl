cwlVersion: v1.2
class: Workflow
inputs:
  graph_construction_workload:
    type: 
        name: GraphConstructionWorkload 
        type: record
        fields:
        - name: primaryInput
            type: ../GIS.cwl#Shapefile
        - name: additionalInput
            type: ../GIS.cwl#Shapefile[]
  config:
    type: File
outputs: 
    standardOut:
        type: boolean
        outputSource: done/trigger
steps:
    generate_graph:
        run: ../fishnet/generate_graph.cwl
        in:
        primaryInput: 
            source: graph_construction_workload
            valueFrom: $(self.primaryInput)
        additionalInput:
            source: graph_construction_workload
            valueFrom: $(self.additionalInput)
        config: config
        out: [standardOut]
    done:
      run:
        class: ExpressionTool
        cwlVersion: v1.2
        inputs:
            dummy:
            type: File
        outputs:
            trigger:
            type: boolean
        expression: |
            ${ return { trigger: true }; }
      in:
        dummy: generate_graph/standardOut
      out: [trigger]
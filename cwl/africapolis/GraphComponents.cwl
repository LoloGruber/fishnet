cwlVersion: v1.2
class: Workflow
requirements:
  - class: SchemaDefRequirement
    types: 
      - $import: ../types/Shapefile.yaml
      - $import: ClusterWorkload.yaml
inputs:
  config:
    type: File
    doc: "Path to configuration file for africapolis components step. Contains database credentials and parallelization target"
  files:
    type: ../types/Shapefile.yaml#Shapefile[]
    doc: "List of shapefiles to be used for assigning the workload for the clustering step"
outputs:
  clusterWorkload:
    type: ClusterWorkload.yaml#ClusterWorkload[]
    outputSource: prepare_cluster_workload/clusterWorkload
steps:
  graph_components:
    run:
      class: CommandLineTool
      baseCommand: [AfricapolisGraphComponents]
      requirements:
        - class: InlineJavascriptRequirement
      inputs:
        config:
          type: File
          inputBinding:
            position: 1
            prefix: -c
          doc: "Path to configuration file for africapolis components step. Contains database credentials and parallelization target"
        componentOutputBasename:
          type: string
          default: "components"
          inputBinding:
            position: 2
            prefix: -o
      outputs:
        components:
          type: string
          outputBinding:
            glob: "$(inputs.componentOutputBasename).json"
            loadContents: true
            outputEval: $(self[0].contents)
          doc: "Output file containing the components of the graph"
        standardOut:
          type: stdout
        errorOut:
          type: stderr
      stdout: COMPONENTS_stdout.log
      stderr: COMPONENTS_stderr.log
    in:
      config: config
    out: [components]
  prepare_cluster_workload:
    run:
      class: ExpressionTool
      inputs:
        components:
          type: string
          doc: "String in json format containing the list of cluster workloads derived from the components of the graph"
        files: 
          type: ../types/Shapefile.yaml#Shapefile[]
          doc: "List of shapefiles to be used for assigning the workload for the clustering step"

      outputs:
        clusterWorkload:
          type: ClusterWorkload.yaml#ClusterWorkload[]
          doc: "Parsed ClusterWorkload object"
      expression: |
        ${
            const json = JSON.parse(inputs.components);
            let workloads = [];
            for(const workloadJson of json){
              if(workloadJson.components.length == 0){
                continue;
              }
              let fileNames = [...new Set(workloadJson.files.map(file => file.split("/").pop()))];
              let files = fileNames.map(fileName => {
                  let fileObject = inputs.files.find(f => f.file.basename == fileName);
                  return fileObject;
                });
              let workload = {
                components: workloadJson.components,
                files: files
              };
              workloads.push(workload);
            }
            return { clusterWorkload: workloads};
        }
    in:
      components: graph_components/components
      files: files
    out: [clusterWorkload]
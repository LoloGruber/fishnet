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
    doc: "Path to configuration file for africapolis clustering step. Contains database credentials"
  workloadFile: 
    type: File
    doc: "File containing the workload for the clustering step"
  files: 
    type: ../types/Shapefile.yaml#Shapefile[]
    doc: "List of shapefiles to be used for assigning the workload for the clustering"
outputs:
  clusteredOutput:
    type: ../types/Shapefile.yaml#Shapefile
    outputSource: clustering/clusteredOutput
steps:
    prepare_cluster_workload:
        run:
            class: ExpressionTool
            inputs:
                component:
                    type: File?
                    loadContents: true
                    doc: "String in json format containing the list of cluster workloads derived from the components of the graph"
                files: 
                    type: ../types/Shapefile.yaml#Shapefile[]
                    doc: "List of shapefiles to be used for assigning the workload for the clustering step"

            outputs:
                clusterWorkload:
                    type: ClusterWorkload.yaml#ClusterWorkload
                    doc: "Parsed ClusterWorkload object"
            expression: |
                ${
                    let workloadJson = JSON.parse(inputs.component.contents);
                    let fileNames = [...new Set(workloadJson.files.map(file => file.split("/").pop()))];
                    let files = fileNames.map(fileName => {
                        let fileObject = inputs.files.find(f => f.file.basename == fileName);
                        return fileObject;
                        });
                    let result = {
                        components: workloadJson.components,
                        files: files
                    };
                    return {
                        clusterWorkload: result,
                    };
                }
        in:
            component: workloadFile
            files: files
        out: [clusterWorkload]
    clustering:
      run: ../fishnet/cluster.cwl
      in:
        clusterWorkload: prepare_cluster_workload/clusterWorkload
        config: config
        components:
          source: prepare_cluster_workload/clusterWorkload
          valueFrom: $(inputs.clusterWorkload.components)
        shpFiles: 
          source: prepare_cluster_workload/clusterWorkload
          valueFrom: $(inputs.clusterWorkload.files)
        outputStem:
          source: prepare_cluster_workload/clusterWorkload
          valueFrom: $("Clustered_"+ inputs.clusterWorkload.components[0])
      out: [clusteredOutput]

cwlVersion: v1.2
class: ExpressionTool
requirements:
- $import: GIS.cwl
inputs:
  files:
    type: File[]

outputs:
  shapefile:
    type: GIS.cwl#Shapefile

expression: |
  ${
    if (inputs.files.length === 0) {
      throw new Error("No filtered shapefile found.");
    }
    function findAssociatedFiles(ext){
        return inputs.files.find(f => f.basename === inputs.files[0].nameroot + ext) || null;
    }
    return {
      "file": findAssociatedFiles(".shp"),
      "shx": findAssociatedFiles(".shx"),
      "dbf": findAssociatedFiles(".dbf"),
      "prj": findAssociatedFiles(".prj"),
      "cpg": findAssociatedFiles(".cpg"),
      "qpj": findAssociatedFiles(".qpj")
    };
  }

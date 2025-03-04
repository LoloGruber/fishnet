cwlVersion: v1.2
class: CommandLineTool
baseCommand: [SettlementDelineationFilter]
requirements: 
  InlineJavascriptRequirement: {}
inputs:
  gisFile:
    type: 
      - $import: ../GIS.yml#GeoTIFF
      - $import: ../GIS.yml#Shapefile
    inputBinding:
      position: 1
      prefix: --input
      valueFrom: $(self.file)  # Correctly reference the GeoTIFF file

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
  filtered_shapefile:
    type: ../GIS.yml#Shapefile
    outputBinding:
      glob: "*_filtered.*"  # Gather all files associate with the shapefile
      outputEval: |
        ${  
          if (self.length === 0) {
            throw new Error("No filtered shapefile found.");
          }
          function findAssociatedFiles(ext){
              return self.find(function(f){return f.basename === self[0].nameroot + ext;}) || null;
          }
          return {
            "shp": findAssociatedFiles(".shp"),
            "shx": findAssociatedFiles(".shx"),
            "dbf": findAssociatedFiles(".dbf"),
            "prj": findAssociatedFiles(".prj"),
            "cpg": findAssociatedFiles(".cpg"),
            "qpj": findAssociatedFiles(".qpj")
          };
        }
stdout: Filter_$(inputs.gisFile.file.nameroot)_stdout.log
stderr: Filter_$(inputs.gisFile.file.nameroot)_stderr.log

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
  outFile:
    type: ../GIS.yml#Shapefile
    outputBinding:
      glob: "*_filtered.shp"  # Ensure we find the shapefile output
      outputEval: |
        ${  
          if (self.length === 0) {
            throw new Error("No filtered shapefile found.");
          }
          
          var shpFile = self[0];  // The matched .shp file
          var baseName = shpFile.nameroot;  // Remove .shp extension

          return {
            "shp": shpFile,
            "shx": self.find(function(f) { return f.basename === baseName + ".shx"; }) || null,
            "dbf": self.find(function(f) { return f.basename === baseName + ".dbf"; }) || null,
            "prj": self.find(function(f) { return f.basename === baseName + ".prj"; }) || null,
            "cpg": self.find(function(f) { return f.basename === baseName + ".cpg"; }) || null,
            "qpj": self.find(function(f) { return f.basename === baseName + ".qpj"; }) || null
          };
        }


stderr: Filter_stderr.log

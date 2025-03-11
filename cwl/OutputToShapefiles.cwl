cwlVersion: v1.2
class: ExpressionTool
requirements:
- $import: GIS.cwl
- class: InlineJavascriptRequirement
inputs:
  files:
    type: File[]
outputs:
  shapefiles:
      type: GIS.cwl#Shapefile[]
expression: |
  ${
    if (inputs.files.length === 0) {
      throw new Error("No shapefiles found.");
    }

    // Helper function to group files by nameroot
    function groupByNameroot(files) {
      var grouped = {};

      files.forEach(function(f) {
        var nameroot = f.nameroot;

        if (!grouped[nameroot]) {
          grouped[nameroot] = {
            shp: null,
            shx: null,
            dbf: null,
            prj: null,
            cpg: null,
            qpj: null
          };
        }

        // Group files by extension
        if (f.basename.endsWith('.shp')) grouped[nameroot].shp = f;
        if (f.basename.endsWith('.shx')) grouped[nameroot].shx = f;
        if (f.basename.endsWith('.dbf')) grouped[nameroot].dbf = f;
        if (f.basename.endsWith('.prj')) grouped[nameroot].prj = f;
        if (f.basename.endsWith('.cpg')) grouped[nameroot].cpg = f;
        if (f.basename.endsWith('.qpj')) grouped[nameroot].qpj = f;
      });

      return grouped;
    }
    var result = Object.values(groupByNameroot(inputs.files)).map(function(group){
      return {
        "file": {
          "class": "File",
          "path": group.shp,
          "secondaryFiles": [
              group.shx, group.dbf,group.prj
          ]
      }
    };}).filter(function(g){return g.file !== null});
    return {"shapefiles": result};
  }
cwlVersion: v1.2
class: ExpressionTool
requirements:
- $import: GIS.cwl
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
      const grouped = {};

      files.forEach(f => {
        const nameroot = f.nameroot;

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

    // Group files by nameroot
    const groupedFiles = groupByNameroot(inputs.files);

    // Convert grouped files into the desired output format
    return Object.values(groupedFiles).map(group => {
      return {
        "shp": group.shp,
        "shx": group.shx,
        "dbf": group.dbf,
        "prj": group.prj,
        "cpg": group.cpg,
        "qpj": group.qpj
      };
    });
  }

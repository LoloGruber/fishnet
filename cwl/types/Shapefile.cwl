cwlVersion: v1.2
name: Shapefile
type: record
fields:
  - name: shp
    type: File
    doc: "The main Shapefile (.shp)"
  - name: shx
    type: File
    doc: "The shape index file (.shx)"
  - name: dbf
    type: File
    doc: "The attribute data file (.dbf)"
  - name: prj
    type: File
    doc: "The projection file (.prj)"
  - name: cpg
    type: File?
    doc: "The character encoding file (.cpg) (optional)"
  - name: qpj
    type: File?
    doc: "The additional projection file (.qpj) (optional)"

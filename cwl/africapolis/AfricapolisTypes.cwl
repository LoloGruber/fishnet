class: SchemaDefRequirement
types:
  - $include: ../GIS.cwl
  - name: GraphConstructionWorkload
    type: record
    fields:
      - name: test
        type: string
      - name: primaryInput
        type: ../GIS.cwl#Shapefile
      - name: additionalInput
        type: ../GIS.cwl#Shapefile[]

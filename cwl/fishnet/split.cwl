cwlVersion: v1.2
class: CommandLineTool
requirements:
- $import: ../GIS.cwl
- class: InlineJavascriptRequirement
baseCommand: [FishnetShapefileSplitter]

inputs:
    gisFile:
        type: 
        # - ../GIS.cwl#Shapefile
        - ../GIS.cwl#GeoTIFF
        inputBinding:
            position: 1
            prefix: --input 
            valueFrom: $(self.file)
    outputDir:
        type: string
        default: "./"
        inputBinding:
            position: 3
            prefix: -o
        doc: "Output directory"
    splits:
        type: int
        inputBinding:
            position: 2
            prefix: -s
    xOffset:
        type: int
        default: 0
        inputBinding:
            prefix: -x
        doc: "X offset for the naming of the output tiles"
    yOffset:
        type: int
        default: 0
        inputBinding:
            prefix: -y
        doc: "Y offset for the naming of the output tiles"

outputs:
    standardOut:
        type: stdout
    errorOut:
        type: stderr
    split_shapefiles:
        type: ../GIS.cwl#Shapefile[]
        outputBinding:
            glob: "$(inputs.gisFile.file.nameroot)*"
            outputEval:
                $include: ../utils/groupToShapefile.js
        doc: "Split output files"


stdout: $(inputs.gisFile.file.nameroot)_split_stdout.log
stderr: $(inputs.gisFile.file.nameroot)_split_stderr.log
    
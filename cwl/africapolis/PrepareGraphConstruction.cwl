cwlVersion: v1.2
class: ExpressionTool
requirements:
  - $import: ../GIS.cwl
inputs:
    shapefiles: ../GIS.cwl#Shapefile[]
    vector_input: ../GIS.cwl#GeoTIFF
outputs: 
    graph_construction_workload:
      type: 
        type: array
        items:
            name: GraphConstructionWorkload 
            type: record
            fields:
              - name: primaryInput
                type: ../GIS.cwl#Shapefile
              - name: additionalInput
                type: ../GIS.cwl#Shapefile[]
expression: | 
    ${
    class Coordinate{
        constructor(x,y){
            this.x = x;
            this.y = y;
        }
        infinityDistance(other) {
            return Math.max(Math.abs(this.x-other.x),Math.abs(this.y-other.y));
        }

        toString() {
            return this.x+","+this.y;
        }
    };

    function stringToCoordinate(coordinateString){
        return new Coordinate(...coordinateString.split(",").map(Number));
    }

    function shapefileObjectToCoordinate(filename,prefix){
        const regex = new RegExp(`${prefix}_(\\d+)_(\\d+)_.*`);
        const match = filename.match(regex);
        if (match) {
            return new Coordinate(parseInt(match[1]), parseInt(match[2]));
        }
        throw new Error("Could not parse grid coordinates from filename \""+filename+"\" with prefix \""+prefix+"\"");
    }
    function neighbouringShapefiles(shapefiles, prefix){
        const shapefileCoordinateMap = {};
        shapefiles.forEach(s => {
            const nameroot = s.file.nameroot;
            const coordinate = shapefileObjectToCoordinate(nameroot,prefix);
            shapefileCoordinateMap[coordinate] = s;
        });
        return Object.keys(shapefileCoordinateMap).map(coordinate => {
            const primaryInput = shapefileCoordinateMap[coordinate];
            const currentCoordinate = stringToCoordinate(coordinate);
            const additionalInput = Object.keys(shapefileCoordinateMap).filter(other => {
                const distance = stringToCoordinate(other).infinityDistance(currentCoordinate);
                return distance > 0 && distance <=1;
            }).map(c => shapefileCoordinateMap[c]);
            return {
                "primaryInput":primaryInput,
                "additionalInput":additionalInput
            };
        });
    }
    const result = neighbouringShapefiles(inputs.shapefiles,inputs.vector_input.file.nameroot);
    return {"graph_construction_workload":result};
    }
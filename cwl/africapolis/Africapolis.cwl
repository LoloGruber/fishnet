{
    "$graph": [
        {
            "class": "Workflow",
            "requirements": [
                {
                    "class": "InitialWorkDirRequirement",
                    "listing": [
                        {
                            "entryname": "fishnet",
                            "writable": true,
                            "entry": ""
                        }
                    ]
                },
                {
                    "class": "ScatterFeatureRequirement"
                },
                {
                    "class": "StepInputExpressionRequirement"
                },
                {
                    "class": "InlineJavascriptRequirement"
                },
                {
                    "class": "SubworkflowFeatureRequirement"
                },
                {
                    "class": "SchemaDefRequirement",
                    "types": [
                        {
                            "name": "#Shapefile.yaml/Shapefile",
                            "type": "record",
                            "fields": [
                                {
                                    "name": "#Shapefile.yaml/Shapefile/file",
                                    "type": "File",
                                    "secondaryFiles": [
                                        {
                                            "pattern": "^.shx",
                                            "required": null
                                        },
                                        {
                                            "pattern": "^.dbf",
                                            "required": null
                                        },
                                        {
                                            "pattern": "^.prj",
                                            "required": null
                                        },
                                        {
                                            "pattern": "^.cpg",
                                            "required": false
                                        },
                                        {
                                            "pattern": "^.qpj",
                                            "required": false
                                        }
                                    ],
                                    "doc": "The main Shapefile (.shp)"
                                }
                            ],
                            "inputBinding": {
                                "valueFrom": "$(self.file)"
                            }
                        },
                        {
                            "name": "#GeoTIFF.yaml/GeoTIFF",
                            "type": "record",
                            "fields": [
                                {
                                    "name": "#GeoTIFF.yaml/GeoTIFF/file",
                                    "type": "File"
                                }
                            ]
                        },
                        {
                            "name": "#GeoTIFF.yaml/Shapefile",
                            "type": "record",
                            "fields": [
                                {
                                    "name": "#GeoTIFF.yaml/Shapefile/file",
                                    "type": "File",
                                    "secondaryFiles": [
                                        {
                                            "pattern": "^.shx",
                                            "required": null
                                        },
                                        {
                                            "pattern": "^.dbf",
                                            "required": null
                                        },
                                        {
                                            "pattern": "^.prj",
                                            "required": null
                                        },
                                        {
                                            "pattern": "^.cpg",
                                            "required": false
                                        },
                                        {
                                            "pattern": "^.qpj",
                                            "required": false
                                        }
                                    ],
                                    "doc": "The main Shapefile (.shp)"
                                }
                            ]
                        },
                        {
                            "name": "#ClusterWorkload.yaml/ClusterWorkload",
                            "type": "record",
                            "fields": [
                                {
                                    "name": "#ClusterWorkload.yaml/ClusterWorkload/components",
                                    "type": {
                                        "type": "array",
                                        "items": "int"
                                    }
                                },
                                {
                                    "name": "#ClusterWorkload.yaml/ClusterWorkload/files",
                                    "type": {
                                        "type": "array",
                                        "items": "#Shapefile.yaml/Shapefile"
                                    }
                                }
                            ]
                        }
                    ]
                }
            ],
            "inputs": [
                {
                    "type": "File",
                    "doc": "Configuration file for Africapolis workflow",
                    "id": "#main/config"
                },
                {
                    "type": [
                        "#GeoTIFF.yaml/GeoTIFF"
                    ],
                    "doc": "Input vector file to Africapolis workflow",
                    "id": "#main/gisInput"
                },
                {
                    "type": "int",
                    "default": 1,
                    "doc": "Number of partitions created on the input for parallel computation",
                    "id": "#main/partitions"
                }
            ],
            "steps": [
                {
                    "run": {
                        "class": "CommandLineTool",
                        "baseCommand": [
                            "AfricapolisClearDatabase"
                        ],
                        "inputs": [
                            {
                                "type": "File",
                                "inputBinding": {
                                    "position": 1,
                                    "prefix": "-c"
                                },
                                "id": "#main/clearDatabase/run/config"
                            }
                        ],
                        "outputs": []
                    },
                    "in": [
                        {
                            "source": "#main/config",
                            "id": "#main/clearDatabase/config"
                        }
                    ],
                    "out": [],
                    "id": "#main/clearDatabase"
                },
                {
                    "run": "#cluster.cwl",
                    "in": [
                        {
                            "source": "#main/graph_components/clusterWorkload",
                            "id": "#main/clustering/clusterWorkload"
                        },
                        {
                            "source": "#main/graph_components/clusterWorkload",
                            "valueFrom": "$(inputs.clusterWorkload.components)",
                            "id": "#main/clustering/components"
                        },
                        {
                            "source": "#main/config",
                            "id": "#main/clustering/config"
                        },
                        {
                            "source": "#main/graph_components/clusterWorkload",
                            "valueFrom": "$(\"Clustered_\"+ inputs.clusterWorkload.components[0])",
                            "id": "#main/clustering/outputStem"
                        },
                        {
                            "source": "#main/graph_components/clusterWorkload",
                            "valueFrom": "$(inputs.clusterWorkload.files)",
                            "id": "#main/clustering/shpFiles"
                        }
                    ],
                    "scatter": "#main/clustering/clusterWorkload",
                    "scatterMethod": "dotproduct",
                    "out": [
                        "#main/clustering/clusteredOutput"
                    ],
                    "id": "#main/clustering"
                },
                {
                    "run": "#filter.cwl",
                    "in": [
                        {
                            "source": "#main/config",
                            "id": "#main/filter/config"
                        },
                        {
                            "source": "#main/split/split_shapefiles",
                            "id": "#main/filter/gisFile"
                        }
                    ],
                    "scatter": [
                        "#main/filter/gisFile"
                    ],
                    "out": [
                        "#main/filter/filtered_shapefile"
                    ],
                    "id": "#main/filter"
                },
                {
                    "run": "#GraphComponents.cwl",
                    "in": [
                        {
                            "source": "#main/config",
                            "id": "#main/graph_components/config"
                        },
                        {
                            "source": "#main/filter/filtered_shapefile",
                            "id": "#main/graph_components/files"
                        },
                        {
                            "source": "#main/graph_construction/trigger",
                            "id": "#main/graph_components/trigger"
                        }
                    ],
                    "out": [
                        "#main/graph_components/clusterWorkload"
                    ],
                    "id": "#main/graph_components"
                },
                {
                    "run": "#GraphConstruction.cwl",
                    "in": [
                        {
                            "source": "#main/config",
                            "id": "#main/graph_construction/config"
                        },
                        {
                            "source": "#main/gisInput",
                            "valueFrom": "$(self.file.nameroot)",
                            "id": "#main/graph_construction/filenamePrefix"
                        },
                        {
                            "source": "#main/filter/filtered_shapefile",
                            "id": "#main/graph_construction/shapefiles"
                        }
                    ],
                    "out": [
                        "#main/graph_construction/trigger"
                    ],
                    "id": "#main/graph_construction"
                },
                {
                    "run": "#mergeShapefiles.cwl",
                    "in": [
                        {
                            "source": "#main/gisInput",
                            "id": "#main/merge/gisInput"
                        },
                        {
                            "source": "#main/gisInput",
                            "valueFrom": "$(\"./\"+self.file.nameroot+\"_Africapolis\")",
                            "id": "#main/merge/outputPath"
                        },
                        {
                            "source": "#main/clustering/clusteredOutput",
                            "id": "#main/merge/shpFiles"
                        }
                    ],
                    "out": [
                        "#main/merge/mergedOutput"
                    ],
                    "id": "#main/merge"
                },
                {
                    "run": "#split.cwl",
                    "in": [
                        {
                            "source": "#main/gisInput",
                            "id": "#main/split/gisFile"
                        },
                        {
                            "source": "#main/partitions",
                            "id": "#main/split/splits"
                        }
                    ],
                    "out": [
                        "#main/split/split_shapefiles"
                    ],
                    "id": "#main/split"
                }
            ],
            "id": "#main",
            "outputs": [
                {
                    "type": "#Shapefile.yaml/Shapefile",
                    "outputSource": "#main/merge/mergedOutput",
                    "id": "#main/result"
                }
            ]
        },
        {
            "class": "Workflow",
            "requirements": [
                {
                    "class": "SchemaDefRequirement",
                    "types": [
                        {
                            "$import": "#Shapefile.yaml/Shapefile"
                        },
                        {
                            "$import": "#ClusterWorkload.yaml/ClusterWorkload"
                        }
                    ]
                }
            ],
            "inputs": [
                {
                    "type": "File",
                    "doc": "Path to configuration file for africapolis components step. Contains database credentials and parallelization target",
                    "id": "#GraphComponents.cwl/config"
                },
                {
                    "type": {
                        "type": "array",
                        "items": "#Shapefile.yaml/Shapefile"
                    },
                    "doc": "List of shapefiles to be used for assigning the workload for the clustering step",
                    "id": "#GraphComponents.cwl/files"
                }
            ],
            "outputs": [
                {
                    "type": {
                        "type": "array",
                        "items": "#ClusterWorkload.yaml/ClusterWorkload"
                    },
                    "outputSource": "#GraphComponents.cwl/prepare_cluster_workload/clusterWorkload",
                    "id": "#GraphComponents.cwl/clusterWorkload"
                }
            ],
            "steps": [
                {
                    "run": {
                        "class": "CommandLineTool",
                        "baseCommand": [
                            "AfricapolisGraphComponents"
                        ],
                        "requirements": [
                            {
                                "class": "InlineJavascriptRequirement"
                            }
                        ],
                        "inputs": [
                            {
                                "type": "string",
                                "default": "components",
                                "inputBinding": {
                                    "position": 2,
                                    "prefix": "-o"
                                },
                                "id": "#GraphComponents.cwl/graph_components/run/componentOutputBasename"
                            },
                            {
                                "type": "File",
                                "inputBinding": {
                                    "position": 1,
                                    "prefix": "-c"
                                },
                                "doc": "Path to configuration file for africapolis components step. Contains database credentials and parallelization target",
                                "id": "#GraphComponents.cwl/graph_components/run/config"
                            }
                        ],
                        "outputs": [
                            {
                                "type": {
                                    "type": "array",
                                    "items": "File"
                                },
                                "outputBinding": {
                                    "glob": "$(inputs.componentOutputBasename)*.json"
                                },
                                "doc": "Output file containing the components of the graph",
                                "id": "#GraphComponents.cwl/graph_components/run/components"
                            },
                            {
                                "type": "File",
                                "id": "#GraphComponents.cwl/graph_components/run/errorOut",
                                "outputBinding": {
                                    "glob": "COMPONENTS_stderr.log"
                                }
                            },
                            {
                                "type": "File",
                                "id": "#GraphComponents.cwl/graph_components/run/standardOut",
                                "outputBinding": {
                                    "glob": "COMPONENTS_stdout.log"
                                }
                            }
                        ],
                        "stdout": "COMPONENTS_stdout.log",
                        "stderr": "COMPONENTS_stderr.log"
                    },
                    "in": [
                        {
                            "source": "#GraphComponents.cwl/config",
                            "id": "#GraphComponents.cwl/graph_components/config"
                        }
                    ],
                    "out": [
                        "#GraphComponents.cwl/graph_components/components"
                    ],
                    "id": "#GraphComponents.cwl/graph_components"
                },
                {
                    "run": {
                        "class": "ExpressionTool",
                        "inputs": [
                            {
                                "type": [
                                    "null",
                                    "File"
                                ],
                                "loadContents": true,
                                "doc": "String in json format containing the list of cluster workloads derived from the components of the graph",
                                "id": "#GraphComponents.cwl/prepare_cluster_workload/run/component"
                            },
                            {
                                "type": {
                                    "type": "array",
                                    "items": "#Shapefile.yaml/Shapefile"
                                },
                                "doc": "List of shapefiles to be used for assigning the workload for the clustering step",
                                "id": "#GraphComponents.cwl/prepare_cluster_workload/run/files"
                            }
                        ],
                        "outputs": [
                            {
                                "type": "#ClusterWorkload.yaml/ClusterWorkload",
                                "doc": "Parsed ClusterWorkload object",
                                "id": "#GraphComponents.cwl/prepare_cluster_workload/run/clusterWorkload"
                            }
                        ],
                        "expression": "${\n      let workloadJson = JSON.parse(inputs.component.contents);\n      let fileNames = [...new Set(workloadJson.files.map(file => file.split(\"/\").pop()))];\n      let files = fileNames.map(fileName => {\n          let fileObject = inputs.files.find(f => f.file.basename == fileName);\n          return fileObject;\n        });\n      let result = {\n        components: workloadJson.components,\n        files: files\n      };\n      return {\n        clusterWorkload: result,\n      };\n}\n"
                    },
                    "in": [
                        {
                            "source": "#GraphComponents.cwl/graph_components/components",
                            "id": "#GraphComponents.cwl/prepare_cluster_workload/component"
                        },
                        {
                            "source": "#GraphComponents.cwl/files",
                            "id": "#GraphComponents.cwl/prepare_cluster_workload/files"
                        }
                    ],
                    "out": [
                        "#GraphComponents.cwl/prepare_cluster_workload/clusterWorkload"
                    ],
                    "scatter": [
                        "#GraphComponents.cwl/prepare_cluster_workload/component"
                    ],
                    "scatterMethod": "dotproduct",
                    "id": "#GraphComponents.cwl/prepare_cluster_workload"
                }
            ],
            "id": "#GraphComponents.cwl"
        },
        {
            "class": "Workflow",
            "requirements": [
                {
                    "class": "SchemaDefRequirement",
                    "types": [
                        {
                            "$import": "#Shapefile.yaml/Shapefile"
                        },
                        {
                            "name": "#GraphConstructionWorkload.yaml/GraphConstructionWorkload",
                            "type": "record",
                            "fields": [
                                {
                                    "name": "#GraphConstructionWorkload.yaml/GraphConstructionWorkload/primaryInput",
                                    "type": "#Shapefile.yaml/Shapefile"
                                },
                                {
                                    "name": "#GraphConstructionWorkload.yaml/GraphConstructionWorkload/additionalInput",
                                    "type": [
                                        "null",
                                        {
                                            "type": "array",
                                            "items": "#Shapefile.yaml/Shapefile"
                                        }
                                    ]
                                }
                            ]
                        }
                    ]
                },
                {
                    "class": "StepInputExpressionRequirement"
                },
                {
                    "class": "ScatterFeatureRequirement"
                },
                {
                    "class": "InlineJavascriptRequirement"
                }
            ],
            "inputs": [
                {
                    "type": "File",
                    "id": "#GraphConstruction.cwl/config"
                },
                {
                    "type": [
                        "null",
                        "string"
                    ],
                    "doc": "Prefix used to identify the shapefiles. The prefix is used to extract the grid coordinates from the filenames.",
                    "id": "#GraphConstruction.cwl/filenamePrefix"
                },
                {
                    "type": {
                        "type": "array",
                        "items": "#Shapefile.yaml/Shapefile"
                    },
                    "doc": "List of shapefiles to be used for graph construction. Each polygon must be associated with a unique FISHNET ID.",
                    "id": "#GraphConstruction.cwl/shapefiles"
                }
            ],
            "outputs": [
                {
                    "type": "boolean",
                    "outputSource": "#GraphConstruction.cwl/done/trigger",
                    "id": "#GraphConstruction.cwl/trigger"
                }
            ],
            "steps": [
                {
                    "run": {
                        "class": "ExpressionTool",
                        "cwlVersion": "v1.2",
                        "inputs": [
                            {
                                "type": {
                                    "type": "array",
                                    "items": "File"
                                },
                                "id": "#GraphConstruction.cwl/done/run/dummy"
                            }
                        ],
                        "outputs": [
                            {
                                "type": "boolean",
                                "id": "#GraphConstruction.cwl/done/run/trigger"
                            }
                        ],
                        "expression": "${ return { trigger: true }; }\n"
                    },
                    "in": [
                        {
                            "source": "#GraphConstruction.cwl/generate_graph/standardOut",
                            "id": "#GraphConstruction.cwl/done/dummy"
                        }
                    ],
                    "out": [
                        "#GraphConstruction.cwl/done/trigger"
                    ],
                    "id": "#GraphConstruction.cwl/done"
                },
                {
                    "run": "#generateGraph.cwl",
                    "in": [
                        {
                            "source": "#GraphConstruction.cwl/prepare_workload/graph_construction_workload",
                            "valueFrom": "$(inputs.graph_construction_workload.additionalInput)",
                            "id": "#GraphConstruction.cwl/generate_graph/additionalInput"
                        },
                        {
                            "source": "#GraphConstruction.cwl/config",
                            "id": "#GraphConstruction.cwl/generate_graph/config"
                        },
                        {
                            "source": "#GraphConstruction.cwl/prepare_workload/graph_construction_workload",
                            "id": "#GraphConstruction.cwl/generate_graph/graph_construction_workload"
                        },
                        {
                            "source": "#GraphConstruction.cwl/prepare_workload/graph_construction_workload",
                            "valueFrom": "$(inputs.graph_construction_workload.primaryInput)",
                            "id": "#GraphConstruction.cwl/generate_graph/primaryInput"
                        }
                    ],
                    "scatter": "#GraphConstruction.cwl/generate_graph/graph_construction_workload",
                    "scatterMethod": "dotproduct",
                    "out": [
                        "#GraphConstruction.cwl/generate_graph/standardOut"
                    ],
                    "id": "#GraphConstruction.cwl/generate_graph"
                },
                {
                    "run": "#PrepareGraphConstruction.cwl",
                    "in": [
                        {
                            "source": "#GraphConstruction.cwl/filenamePrefix",
                            "id": "#GraphConstruction.cwl/prepare_workload/filenamePrefix"
                        },
                        {
                            "source": "#GraphConstruction.cwl/shapefiles",
                            "id": "#GraphConstruction.cwl/prepare_workload/shapefiles"
                        }
                    ],
                    "out": [
                        "#GraphConstruction.cwl/prepare_workload/graph_construction_workload"
                    ],
                    "id": "#GraphConstruction.cwl/prepare_workload"
                }
            ],
            "id": "#GraphConstruction.cwl"
        },
        {
            "class": "ExpressionTool",
            "requirements": [
                {
                    "class": "SchemaDefRequirement",
                    "types": [
                        {
                            "$import": "#Shapefile.yaml/Shapefile"
                        },
                        {
                            "$import": "#GeoTIFF.yaml/GeoTIFF"
                        },
                        {
                            "$import": "#GeoTIFF.yaml/Shapefile"
                        },
                        {
                            "$import": "#GraphConstructionWorkload.yaml/GraphConstructionWorkload"
                        }
                    ]
                }
            ],
            "inputs": [
                {
                    "type": [
                        "null",
                        "string"
                    ],
                    "id": "#PrepareGraphConstruction.cwl/filenamePrefix"
                },
                {
                    "type": {
                        "type": "array",
                        "items": "#Shapefile.yaml/Shapefile"
                    },
                    "id": "#PrepareGraphConstruction.cwl/shapefiles"
                }
            ],
            "outputs": [
                {
                    "type": {
                        "type": "array",
                        "items": "#GraphConstructionWorkload.yaml/GraphConstructionWorkload"
                    },
                    "id": "#PrepareGraphConstruction.cwl/graph_construction_workload"
                }
            ],
            "expression": "${\nclass Coordinate{\n    constructor(x,y){\n        this.x = x;\n        this.y = y;\n    }\n    infinityDistance(other) {\n        return Math.max(Math.abs(this.x-other.x),Math.abs(this.y-other.y));\n    }\n\n    toString() {\n        return this.x+\",\"+this.y;\n    }\n};\n\nfunction stringToCoordinate(coordinateString){\n    return new Coordinate(...coordinateString.split(\",\").map(Number));\n}\n\nfunction shapefileObjectToCoordinate(filename,prefix){\n    const regex = new RegExp(`${prefix}_(\\\\d+)_(\\\\d+)_.*`);\n    const match = filename.match(regex);\n    if (match) {\n        return new Coordinate(parseInt(match[1]), parseInt(match[2]));\n    }\n    throw new Error(\"Could not parse grid coordinates from filename \\\"\"+filename+\"\\\" with prefix \\\"\"+prefix+\"\\\"\");\n}\nfunction neighbouringShapefiles(shapefiles, prefix){\n    const shapefileCoordinateMap = {};\n    shapefiles.forEach(s => {\n        const nameroot = s.file.nameroot;\n        const coordinate = shapefileObjectToCoordinate(nameroot,prefix);\n        shapefileCoordinateMap[coordinate] = s;\n    });\n    return Object.keys(shapefileCoordinateMap).map(coordinate => {\n        const primaryInput = shapefileCoordinateMap[coordinate];\n        const currentCoordinate = stringToCoordinate(coordinate);\n        const additionalInput = Object.keys(shapefileCoordinateMap).filter(other => {\n            const distance = stringToCoordinate(other).infinityDistance(currentCoordinate);\n            return distance > 0 && distance <=1;\n        }).map(c => shapefileCoordinateMap[c]);\n        if(additionalInput.length == 0){\n            return {\n                \"primaryInput\":primaryInput,\n                \"additionalInput\":null\n            };\n        }\n        else return {\n            \"primaryInput\":primaryInput,\n            \"additionalInput\":additionalInput\n        };\n    });\n}\n// TODO make it work even when no prefix is given\nconst result = neighbouringShapefiles(inputs.shapefiles,inputs.filenamePrefix);\nreturn {\"graph_construction_workload\":result};\n}",
            "id": "#PrepareGraphConstruction.cwl"
        },
        {
            "class": "CommandLineTool",
            "baseCommand": [
                "SettlementDelineationContraction"
            ],
            "requirements": [
                {
                    "class": "InlineJavascriptRequirement"
                },
                {
                    "class": "SchemaDefRequirement",
                    "types": [
                        {
                            "$import": "#Shapefile.yaml/Shapefile"
                        }
                    ]
                }
            ],
            "inputs": [
                {
                    "type": {
                        "type": "array",
                        "items": "int"
                    },
                    "inputBinding": {
                        "prefix": "--components"
                    },
                    "doc": "List of connected components to be processed by this job",
                    "id": "#cluster.cwl/components"
                },
                {
                    "type": "File",
                    "inputBinding": {
                        "prefix": "-c"
                    },
                    "doc": "Path to configuration for contraction task",
                    "id": "#cluster.cwl/config"
                },
                {
                    "type": "string",
                    "inputBinding": {
                        "position": 2,
                        "prefix": "--outputStem"
                    },
                    "doc": "Output filename storing the merged polygons",
                    "id": "#cluster.cwl/outputStem"
                },
                {
                    "type": {
                        "type": "array",
                        "items": "#Shapefile.yaml/Shapefile",
                        "inputBinding": {
                            "valueFrom": "$(self.file)"
                        }
                    },
                    "inputBinding": {
                        "prefix": "-i"
                    },
                    "doc": "List of input shapefiles, with their required secondary files (.dbf, .shx, .prj)",
                    "id": "#cluster.cwl/shpFiles"
                }
            ],
            "outputs": [
                {
                    "type": "#Shapefile.yaml/Shapefile",
                    "outputBinding": {
                        "glob": "$(inputs.outputStem).*",
                        "outputEval": "${\nfunction groupShapefilesByNameroot(files) {\n    var grouped = {};\n    files.forEach(function(f) {\n      var nameroot = f.nameroot || f.basename.replace(/\\.[^/.]+$/, \"\");\n      if (!grouped[nameroot]) {\n        grouped[nameroot] = {};\n      }\n      if (f.basename.endsWith(\".shp\")) grouped[nameroot].shp = f;\n      if (f.basename.endsWith(\".shx\")) grouped[nameroot].shx = f;\n      if (f.basename.endsWith(\".dbf\")) grouped[nameroot].dbf = f;\n      if (f.basename.endsWith(\".prj\")) grouped[nameroot].prj = f;\n      if (f.basename.endsWith(\".cpg\")) grouped[nameroot].cpg = f;\n      if (f.basename.endsWith(\".qpj\")) grouped[nameroot].qpj = f;\n    });\n    return Object.values(grouped).filter(f => f && f.shp);;\n}\n\nfunction groupToShapefileObject(group) {\n    if (!group.shp) return null;\n    return {\n        \"file\":{\n            class: \"File\",\n            path: group.shp.path,\n            basename: group.shp.basename,\n            nameroot: group.shp.nameroot,\n            nameext: group.shp.nameext,\n            secondaryFiles: [group.shx, group.dbf, group.prj,group.cpg,group.qpj].filter(Boolean)\n        }\n    };\n}\n\nvar shapefiles = groupShapefilesByNameroot(self);\nif(shapefiles.length == 1)\n    return groupToShapefileObject(shapefiles.at(0));\nreturn shapefiles.map(groupToShapefileObject).filter(Boolean);\n}\n  "
                    },
                    "doc": "Merged output file",
                    "id": "#cluster.cwl/clusteredOutput"
                },
                {
                    "type": "File",
                    "id": "#cluster.cwl/errorOut",
                    "outputBinding": {
                        "glob": "CLUSTER_$(inputs.components[0])_stderr.log"
                    }
                },
                {
                    "type": "File",
                    "id": "#cluster.cwl/standardOut",
                    "outputBinding": {
                        "glob": "CLUSTER_$(inputs.components[0])_stdout.log"
                    }
                }
            ],
            "stdout": "CLUSTER_$(inputs.components[0])_stdout.log",
            "stderr": "CLUSTER_$(inputs.components[0])_stderr.log",
            "id": "#cluster.cwl"
        },
        {
            "class": "CommandLineTool",
            "requirements": [
                {
                    "class": "SchemaDefRequirement",
                    "types": [
                        {
                            "$import": "#Shapefile.yaml/Shapefile"
                        }
                    ]
                },
                {
                    "class": "InlineJavascriptRequirement"
                }
            ],
            "baseCommand": [
                "SettlementDelineationFilter"
            ],
            "inputs": [
                {
                    "type": "File",
                    "doc": "Configuration file for filter process",
                    "inputBinding": {
                        "prefix": "--config",
                        "position": 2
                    },
                    "id": "#filter.cwl/config"
                },
                {
                    "type": [
                        "#Shapefile.yaml/Shapefile"
                    ],
                    "inputBinding": {
                        "position": 1,
                        "prefix": "--input",
                        "valueFrom": "$(self.file)"
                    },
                    "id": "#filter.cwl/gisFile"
                }
            ],
            "outputs": [
                {
                    "type": "File",
                    "id": "#filter.cwl/errorOut",
                    "outputBinding": {
                        "glob": "FILTER_$(inputs.gisFile.file.nameroot)_stderr.log"
                    }
                },
                {
                    "type": "#Shapefile.yaml/Shapefile",
                    "outputBinding": {
                        "glob": "*_filtered.*",
                        "outputEval": "${\nfunction groupShapefilesByNameroot(files) {\n    var grouped = {};\n    files.forEach(function(f) {\n      var nameroot = f.nameroot || f.basename.replace(/\\.[^/.]+$/, \"\");\n      if (!grouped[nameroot]) {\n        grouped[nameroot] = {};\n      }\n      if (f.basename.endsWith(\".shp\")) grouped[nameroot].shp = f;\n      if (f.basename.endsWith(\".shx\")) grouped[nameroot].shx = f;\n      if (f.basename.endsWith(\".dbf\")) grouped[nameroot].dbf = f;\n      if (f.basename.endsWith(\".prj\")) grouped[nameroot].prj = f;\n      if (f.basename.endsWith(\".cpg\")) grouped[nameroot].cpg = f;\n      if (f.basename.endsWith(\".qpj\")) grouped[nameroot].qpj = f;\n    });\n    return Object.values(grouped).filter(f => f && f.shp);;\n}\n\nfunction groupToShapefileObject(group) {\n    if (!group.shp) return null;\n    return {\n        \"file\":{\n            class: \"File\",\n            path: group.shp.path,\n            basename: group.shp.basename,\n            nameroot: group.shp.nameroot,\n            nameext: group.shp.nameext,\n            secondaryFiles: [group.shx, group.dbf, group.prj,group.cpg,group.qpj].filter(Boolean)\n        }\n    };\n}\n\nvar shapefiles = groupShapefilesByNameroot(self);\nif(shapefiles.length == 1)\n    return groupToShapefileObject(shapefiles.at(0));\nreturn shapefiles.map(groupToShapefileObject).filter(Boolean);\n}\n  "
                    },
                    "id": "#filter.cwl/filtered_shapefile"
                },
                {
                    "type": "File",
                    "id": "#filter.cwl/standardOut",
                    "outputBinding": {
                        "glob": "FILTER_$(inputs.gisFile.file.nameroot)_stdout.log"
                    }
                }
            ],
            "stdout": "FILTER_$(inputs.gisFile.file.nameroot)_stdout.log",
            "stderr": "FILTER_$(inputs.gisFile.file.nameroot)_stderr.log",
            "id": "#filter.cwl"
        },
        {
            "class": "CommandLineTool",
            "baseCommand": [
                "SettlementDelineationNeighbours"
            ],
            "requirements": [
                {
                    "class": "SchemaDefRequirement",
                    "types": [
                        {
                            "$import": "#Shapefile.yaml/Shapefile"
                        }
                    ]
                },
                {
                    "class": "InlineJavascriptRequirement"
                }
            ],
            "inputs": [
                {
                    "type": {
                        "type": "array",
                        "items": "#Shapefile.yaml/Shapefile",
                        "inputBinding": {
                            "valueFrom": "$(self.file)"
                        }
                    },
                    "inputBinding": {
                        "prefix": "-a"
                    },
                    "doc": "List of additional input shapefiles in proximity to the primary input, with their required secondary files (.dbf, .shx, .prj)",
                    "id": "#generateGraph.cwl/additionalInput"
                },
                {
                    "type": "File",
                    "inputBinding": {
                        "prefix": "-c"
                    },
                    "doc": "Path to configuration for neighbours task. Contains graph database credentials, neighbouring criteria, ...",
                    "id": "#generateGraph.cwl/config"
                },
                {
                    "type": "#Shapefile.yaml/Shapefile",
                    "inputBinding": {
                        "prefix": "-i",
                        "valueFrom": "$(self.file)"
                    },
                    "doc": "Primary input, supplied as shapefile object",
                    "id": "#generateGraph.cwl/primaryInput"
                }
            ],
            "outputs": [
                {
                    "type": "File",
                    "id": "#generateGraph.cwl/errorOut",
                    "outputBinding": {
                        "glob": "GENERATE_GRAPH_$(inputs.primaryInput.file.nameroot)_stderr.log"
                    }
                },
                {
                    "type": "File",
                    "id": "#generateGraph.cwl/standardOut",
                    "outputBinding": {
                        "glob": "GENERATE_GRAPH_$(inputs.primaryInput.file.nameroot)_stdout.log"
                    }
                }
            ],
            "stdout": "GENERATE_GRAPH_$(inputs.primaryInput.file.nameroot)_stdout.log",
            "stderr": "GENERATE_GRAPH_$(inputs.primaryInput.file.nameroot)_stderr.log",
            "id": "#generateGraph.cwl"
        },
        {
            "class": "CommandLineTool",
            "baseCommand": [
                "SettlementDelineationMerge"
            ],
            "requirements": [
                {
                    "class": "InlineJavascriptRequirement"
                },
                {
                    "class": "SchemaDefRequirement",
                    "types": [
                        {
                            "$import": "#Shapefile.yaml/Shapefile"
                        }
                    ]
                }
            ],
            "inputs": [
                {
                    "type": "string",
                    "inputBinding": {
                        "position": 2,
                        "prefix": "-o",
                        "valueFrom": "$(self+\".shp\")"
                    },
                    "doc": "Output filename for result (Shapefile)",
                    "id": "#mergeShapefiles.cwl/outputPath"
                },
                {
                    "type": {
                        "type": "array",
                        "items": "#Shapefile.yaml/Shapefile",
                        "inputBinding": {
                            "valueFrom": "$(self.file)"
                        }
                    },
                    "inputBinding": {
                        "prefix": "-i"
                    },
                    "doc": "List of input shapefiles, with their required secondary files (.dbf, .shx, .prj)",
                    "id": "#mergeShapefiles.cwl/shpFiles"
                }
            ],
            "outputs": [
                {
                    "type": "File",
                    "id": "#mergeShapefiles.cwl/errorOut",
                    "outputBinding": {
                        "glob": "MERGE_stderr.log"
                    }
                },
                {
                    "type": "#Shapefile.yaml/Shapefile",
                    "outputBinding": {
                        "glob": "$(inputs.outputPath).*",
                        "outputEval": "${\nfunction groupShapefilesByNameroot(files) {\n    var grouped = {};\n    files.forEach(function(f) {\n      var nameroot = f.nameroot || f.basename.replace(/\\.[^/.]+$/, \"\");\n      if (!grouped[nameroot]) {\n        grouped[nameroot] = {};\n      }\n      if (f.basename.endsWith(\".shp\")) grouped[nameroot].shp = f;\n      if (f.basename.endsWith(\".shx\")) grouped[nameroot].shx = f;\n      if (f.basename.endsWith(\".dbf\")) grouped[nameroot].dbf = f;\n      if (f.basename.endsWith(\".prj\")) grouped[nameroot].prj = f;\n      if (f.basename.endsWith(\".cpg\")) grouped[nameroot].cpg = f;\n      if (f.basename.endsWith(\".qpj\")) grouped[nameroot].qpj = f;\n    });\n    return Object.values(grouped).filter(f => f && f.shp);;\n}\n\nfunction groupToShapefileObject(group) {\n    if (!group.shp) return null;\n    return {\n        \"file\":{\n            class: \"File\",\n            path: group.shp.path,\n            basename: group.shp.basename,\n            nameroot: group.shp.nameroot,\n            nameext: group.shp.nameext,\n            secondaryFiles: [group.shx, group.dbf, group.prj,group.cpg,group.qpj].filter(Boolean)\n        }\n    };\n}\n\nvar shapefiles = groupShapefilesByNameroot(self);\nif(shapefiles.length == 1)\n    return groupToShapefileObject(shapefiles.at(0));\nreturn shapefiles.map(groupToShapefileObject).filter(Boolean);\n}\n  "
                    },
                    "doc": "Merged output file",
                    "id": "#mergeShapefiles.cwl/mergedOutput"
                },
                {
                    "type": "File",
                    "id": "#mergeShapefiles.cwl/standardOut",
                    "outputBinding": {
                        "glob": "MERGE_stdout.log"
                    }
                }
            ],
            "stdout": "MERGE_stdout.log",
            "stderr": "MERGE_stderr.log",
            "id": "#mergeShapefiles.cwl"
        },
        {
            "class": "CommandLineTool",
            "requirements": [
                {
                    "class": "SchemaDefRequirement",
                    "types": [
                        {
                            "$import": "#GeoTIFF.yaml/GeoTIFF"
                        },
                        {
                            "$import": "#GeoTIFF.yaml/Shapefile"
                        },
                        {
                            "$import": "#Shapefile.yaml/Shapefile"
                        }
                    ]
                },
                {
                    "class": "InlineJavascriptRequirement"
                }
            ],
            "baseCommand": [
                "FishnetShapefileSplitter"
            ],
            "inputs": [
                {
                    "type": [
                        "#GeoTIFF.yaml/GeoTIFF"
                    ],
                    "inputBinding": {
                        "position": 1,
                        "prefix": "--input",
                        "valueFrom": "$(self.file)"
                    },
                    "id": "#split.cwl/gisFile"
                },
                {
                    "type": "string",
                    "default": "./",
                    "inputBinding": {
                        "position": 3,
                        "prefix": "-o"
                    },
                    "doc": "Output directory",
                    "id": "#split.cwl/outputDir"
                },
                {
                    "type": "int",
                    "inputBinding": {
                        "position": 2,
                        "prefix": "-s"
                    },
                    "id": "#split.cwl/splits"
                },
                {
                    "type": "int",
                    "default": 0,
                    "inputBinding": {
                        "prefix": "-x"
                    },
                    "doc": "X offset for the naming of the output tiles",
                    "id": "#split.cwl/xOffset"
                },
                {
                    "type": "int",
                    "default": 0,
                    "inputBinding": {
                        "prefix": "-y"
                    },
                    "doc": "Y offset for the naming of the output tiles",
                    "id": "#split.cwl/yOffset"
                }
            ],
            "outputs": [
                {
                    "type": "File",
                    "id": "#split.cwl/errorOut",
                    "outputBinding": {
                        "glob": "SPLIT_$(inputs.gisFile.file.nameroot)_stderr.log"
                    }
                },
                {
                    "type": {
                        "type": "array",
                        "items": "#Shapefile.yaml/Shapefile"
                    },
                    "outputBinding": {
                        "glob": "$(inputs.gisFile.file.nameroot)*",
                        "outputEval": "${\nfunction groupShapefilesByNameroot(files) {\n    var grouped = {};\n    files.forEach(function(f) {\n      var nameroot = f.nameroot || f.basename.replace(/\\.[^/.]+$/, \"\");\n      if (!grouped[nameroot]) {\n        grouped[nameroot] = {};\n      }\n      if (f.basename.endsWith(\".shp\")) grouped[nameroot].shp = f;\n      if (f.basename.endsWith(\".shx\")) grouped[nameroot].shx = f;\n      if (f.basename.endsWith(\".dbf\")) grouped[nameroot].dbf = f;\n      if (f.basename.endsWith(\".prj\")) grouped[nameroot].prj = f;\n      if (f.basename.endsWith(\".cpg\")) grouped[nameroot].cpg = f;\n      if (f.basename.endsWith(\".qpj\")) grouped[nameroot].qpj = f;\n    });\n    return Object.values(grouped).filter(f => f && f.shp);;\n}\n\nfunction groupToShapefileObject(group) {\n    if (!group.shp) return null;\n    return {\n        \"file\":{\n            class: \"File\",\n            path: group.shp.path,\n            basename: group.shp.basename,\n            nameroot: group.shp.nameroot,\n            nameext: group.shp.nameext,\n            secondaryFiles: [group.shx, group.dbf, group.prj,group.cpg,group.qpj].filter(Boolean)\n        }\n    };\n}\n\nvar shapefiles = groupShapefilesByNameroot(self);\nif(shapefiles.length == 1)\n    return groupToShapefileObject(shapefiles.at(0));\nreturn shapefiles.map(groupToShapefileObject).filter(Boolean);\n}\n  "
                    },
                    "doc": "Split output files",
                    "id": "#split.cwl/split_shapefiles"
                },
                {
                    "type": "File",
                    "id": "#split.cwl/standardOut",
                    "outputBinding": {
                        "glob": "SPLIT_$(inputs.gisFile.file.nameroot)_stdout.log"
                    }
                }
            ],
            "stdout": "SPLIT_$(inputs.gisFile.file.nameroot)_stdout.log",
            "stderr": "SPLIT_$(inputs.gisFile.file.nameroot)_stderr.log",
            "id": "#split.cwl"
        }
    ],
    "cwlVersion": "v1.2"
}

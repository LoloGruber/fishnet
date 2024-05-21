import json
from os import listdir
from os.path import isfile, join

def createShpFile(path:str):
    secondaryFiles = []
    ext = [".shx",".dbf",".prj"]
    for e in ext:
        f = {}
        f["class"] = "File"
        f["path"] = path[:-4] + e
        secondaryFiles.append(f)
    shpFile = {}
    shpFile["class"] = "File"
    shpFile["path"] = path
    shpFile["secondaryFiles"]= [secondaryFiles] 
    return shpFile

def createConfigFile(path:str):
    configFile = {}
    configFile["class"] = "File"
    configFile["path"] = path
    return configFile

def writeJobFilter(pathToShp, pathToConfig,pathToJobDir script):
    jobPath = os.join(pathToJobDir,pathToShp.split("/")[-1][:-4]+".json")
    job = {}
    job["shpFile"] = createShpFile(pathToShp)
    job["filterConfig"] = createConfigFile(pathToConfig)
    writeJobToPath(job,jobPath)
    script.append(f"cwltool ../filter.cwl {jobPath}")

def writeJobToPath(job, path):
    with open(path,"w") as f:
        json.dump(job,f)

def main():
    script = []
    script.append("!/bin/bash")
    script.append("rm -rf workingDir")
    script.append("mkdir workingDir")
    script.append("cd workingDir")
    filterCfg = "/home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/cfg/"
    pathToFiles = "/home/lolo/Documents/fishnet/data/testing/Punjab_Split"
    jobDir = "/home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/cwl/jobs/split/"
    allshpFiles = [f for f in listdir(pathToFiles) if isfile(join(pathToFiles, f)) and f.endswith(".shp")]
    neighboursJob = {}
    contractionJob = {}
    neighboursJob["config"] = createConfigFile("/home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/cfg/neighbours.json")
    contractionJob["config"] = createConfigFile("/home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/cwl/jobs/contractionExample.json")
    inputs = []
    for f in allshpFiles:
        createFilterJob(join(pathToFiles, f),filterCfg,jobDir,script)
        inputs.append(createShpFile(f[:4]+"_filtered.shp"))
    neighboursJob["shpFiles"] = inputs
    contractionJob["shpFiles"] = inputs
    contractionJob["output"] = inputs
    writeJobToPath(os.join(pathToFiles,))

    with open("/home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/cwl/jobs/split/neighbours.json","w") as f:
        json.dump(neighboursJob,f)
        script.append(f"cwltool ../neighbours.cwl {f}")




    completeScript = ""
    for line in script:
        completeScript += line +"\n"
    with open("runSplit.sh","w") as bashFile:
        bashFile.write(completeScript)

if __name__ == "__main__":
    main()


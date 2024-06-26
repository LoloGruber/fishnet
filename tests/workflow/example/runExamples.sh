#!/bin/bash
rm -rf workingDir
mkdir workingDir
cd workingDir
cwltool /home/lolo/Documents/fishnet/prod/cwl/filter.cwl ../filterExample.json
cwltool /home/lolo/Documents/fishnet/prod/cwl/neighbours.cwl ../neighboursExample.json
cwltool /home/lolo/Documents/fishnet/prod/cwl/contraction.cwl ../contractionExample.json
cwltool /home/lolo/Documents/fishnet/prod/cwl/analysis.cwl ../analysisExample.json 

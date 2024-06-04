#!/bin/bash
rm -rf workingDir
mkdir workingDir
cd workingDir
cwltool /home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/cwl/filter.cwl ../filterExample.json
cwltool /home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/cwl/neighbours.cwl ../neighboursExample.json
cwltool /home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/cwl/contraction.cwl ../contractionExample.json
cwltool /home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/cwl/analysis.cwl ../analysisExample.json 

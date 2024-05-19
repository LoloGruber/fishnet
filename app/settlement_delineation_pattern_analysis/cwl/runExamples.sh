#!/bin/bash
mkdir workingDir
cd workingDir
cwltool ../filter.cwl ../jobs/filterExample.json
cwltool ../neighbours.cwl ../jobs/neighboursExample.json
cwltool ../contract.cwl ../jobs/contractionExample.json

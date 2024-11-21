cwlVersion: v1.2
class: CommandLineTool
baseCommand: [SettlementDelineationComponents]
inputs:
  config:
    type: File
    inputBinding:
      position: 1
      prefix: -c
    doc: "Path to configuration for the filters"
  jobDirectory:
    type: string
    inputBinding:
      position: 2
      prefix: -j
    doc: "Path to directory where created jobs will be stored"
  workflowID:
    type: long?
    inputBinding: 
      position: 3
      prefix: -w
    doc: "Workflow id for isolated database sessions"

outputs:
  standardOut:
    type: stdout
  errorOut:
    type: stderr

stdout: ComponentsTask_stdout.log
stderr: ComponentsTask_stderr.log
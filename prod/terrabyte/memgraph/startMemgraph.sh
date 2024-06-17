module use /dss/dsstbyfs01/pn56su/pn56su-dss-0020/usr/share/modules/files/
module load charliecloud
ch-run --write --home -b ~/memgraph/logs:/var/log/memgraph memgraph%memgraph-mage+latest -- /usr/lib/memgraph/memgraph --log-level=TRACE --query-execution-timeout-sec=0
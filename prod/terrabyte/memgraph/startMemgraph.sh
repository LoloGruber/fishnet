module use /dss/dsstbyfs01/pn56su/pn56su-dss-0020/usr/share/modules/files/
module load charliecloud
# ch-run --write --home -b ~/memgraph-volume/logs:/var/log/memgraph memgraph%memgraph+latest.sqfs -- /usr/lib/memgraph/memgraph # add params here

# Max virtual memory areas vm.max_map_count 65530 is too low, increase to at least 262144 -> cannot increase using sysctl... :(
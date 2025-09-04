SDA_CONTAINER_PATH=~/sda-workflow/logru%sda+latest
CH_CONTAINER_PATH=${SDA_CONTAINER_PATH}
module use /dss/dsstbyfs01/pn56su/pn56su-dss-0020/usr/share/modules/files/
module load charliecloud
ch-run --home -b ~/data:/home/ ${CH_CONTAINER_PATH} -- /bin/bash

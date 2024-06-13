module use /dss/dsstbyfs01/pn56su/pn56su-dss-0020/usr/share/modules/files/
module load charliecloud
USER_ID=dis35pof
IMG_NAME=memgraph
ch-image build --force -t ${IMG_NAME} -f . .
mv /var/tmp/${USER_ID}/img/${IMG_NAME}/ ~/database

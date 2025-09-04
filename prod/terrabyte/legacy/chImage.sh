USER_ID=di35pof
DOCKER_IMAGE=logru/sda:latest
INSTALL_REL_DIR=sda-workflow-container
# load the charliecloud module
module use /dss/dsstbyfs01/pn56su/pn56su-dss-0020/usr/share/modules/files/
module load charliecloud
# write intermediate files to local SSD for performance
export CH_IMAGE_STORAGE=/tmp/$USER_ID/chConvert
# and pull image from Registry
ch-image pull -s $CH_IMAGE_STORAGE $DOCKER_IMAGE

# convert the docker-Name syntax to the one used by charliecloud,
# allows us to keep the information on the registry image and version
CH_IMAGE_NAME=$(echo $DOCKER_IMAGE | sed 's#/#%#g' | sed 's#:#+#g')

# create empty folders for potential mountpoints
for f in lrz dss gpfs
do
    mkdir ${CH_IMAGE_STORAGE}/img/${CH_IMAGE_NAME}/$f
done
mv ${CH_IMAGE_STORAGE}/img/${CH_IMAGE_NAME} ~/${INSTALL_REL_DIR}

rm -r -f $CH_IMAGE_STORAGE
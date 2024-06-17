USER_ID=di35pof
IMG_NAME=memgraph/memgraph-mage:latest
INSTALL_DIR=~/${IMG_NAME}
# load the charliecloud module
module use /dss/dsstbyfs01/pn56su/pn56su-dss-0020/usr/share/modules/files/
module load charliecloud
# write intermediate files to local SSD for performance
export CH_IMAGE_STORAGE=/tmp/$USER_ID/chConvert
# and pull image from Registry
ch-image pull -s $CH_IMAGE_STORAGE $IMG_NAME

# convert the docker-Name syntax to the one used by charliecloud,
# allows us to keep the information on the registry image and version
chimageName=$(echo $IMG_NAME | sed 's#/#%#g' | sed 's#:#+#g')

# create empty folders for potential mountpoints
for f in lrz dss gpfs
do
    mkdir ${CH_IMAGE_STORAGE}/img/${chimageName}/$f
done
mv ${CH_IMAGE_STORAGE}/img/${chimageName} ~/memgraph

rm -r -f $CH_IMAGE_STORAGE
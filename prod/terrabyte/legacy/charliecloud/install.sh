# This command has to be executed on terrabyte / the HPC system
module use /dss/dsstbyfs01/pn56su/pn56su-dss-0020/usr/share/modules/files/
module load charliecloud
INSTALL_PATH=/dss/dsshome1/0D/di35pof/.fishnet/fishnet-container
# Untar the tarball
# rm -rf $INSTALL_PATH
ch-convert -s ~/.ch fishnet-image.tar.gz $INSTALL_PATH
cp ../start.sh $INSTALL_PATH
chmod +x $INSTALL_PATH/start.sh
# export PATH=/opt/venv/bin:$PATH
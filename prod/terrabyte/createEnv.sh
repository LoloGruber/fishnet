module use /dss/dsstbyfs01/pn56su/pn56su-dss-0020/usr/share/modules/files/
module load micromamba
eval "$(micromamba shell hook --shell bash)"
micromamba shell init --shell bash --root-prefix=~/micromamba
micromamba create -n fishnet
micromamba activate fishnet
micromamba install -y gdal=3.4.1
micromamba install -y gcc=13.2
micromamba install -y gxx=13.2
micromamba install -y cmake=3.22 
# link from include/gdal back to include -> allows import using #include <gdal/gdal.h>
cd /dss/dsshome1/0D/di35pof/micromamba/envs/fishnet/include/ 
ln -s . gdal
cd -

#custom ssl include dir -> use env variable
# SET(OPENSSL_INCLUDE_DIR /dss/dsshome1/0D/di35pof/micromamba/include)
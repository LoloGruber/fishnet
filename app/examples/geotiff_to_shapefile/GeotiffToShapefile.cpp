#include <fishnet/GISFactory.hpp>
#include <fishnet/VectorIO.hpp>
#include <fishnet/PathHelper.h>
#include <fishnet/Polygon.hpp>

using namespace fishnet;
int main(int argc, char * argv[]) {
    std::filesystem::path src {util::PathHelper::projectDirectory().append("data/WSF/2019/Bolivia/Bolivia_WSF2019population_WSF3D_binary.tif")};
    auto shpFileExp = GISFactory::asShapefile(src);
    if(not shpFileExp)
        throw std::runtime_error(shpFileExp.error());
    auto layer = VectorIO::readPolygonLayer(shpFileExp.value());
    VectorIO::overwrite(layer,src.replace_extension(".shp"));
    return 0;
}
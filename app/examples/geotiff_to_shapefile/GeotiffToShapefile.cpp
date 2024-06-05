#include <fishnet/GISFactory.hpp>
#include <fishnet/VectorLayer.hpp>
#include <fishnet/PathHelper.h>
#include <fishnet/Polygon.hpp>

using namespace fishnet;
int main(int argc, char * argv[]) {
    std::filesystem::path src {util::PathHelper::projectDirectory().append("data/WSF/2019/Casablanca/WesternCasablanca.tif")};
    auto shpFileExp = GISFactory::asShapefile(src);
    if(not shpFileExp)
        throw std::runtime_error(shpFileExp.error());
    auto layer = VectorLayer<geometry::Polygon<double>>::read(shpFileExp.value());
    layer.overwrite(src.replace_extension(".shp"));
    return 0;
}
#include <future>
#include <CLI/CLI.hpp>
#include <fishnet/Fishnet.hpp>

template<fishnet::geometry::GeometryObject G>
class ShapefileMerger {
public:
    static fishnet::Shapefile operator()(const fishnet::util::range_of<fishnet::Shapefile> auto& inputs, std::filesystem::path && outputPath){
        std::vector<std::future<fishnet::VectorLayer<G>>> futures;
        for(size_t i = 1; i < inputs.size();i++){
            futures.push_back(std::async(std::launch::async,[&inputs,i](){return fishnet::VectorIO::read<G>(inputs[i]);}));
        }
        auto firstLayer = fishnet::VectorIO::read<G>(inputs.front());
        auto outputLayer = fishnet::VectorIO::emptyCopy<G>(firstLayer);
        for(auto && feature: firstLayer.getFeatures()){
            outputLayer.addFeature(std::move(feature));
        }
        for(auto & future:futures){
            auto layer = future.get();
            layer.copyFields(outputLayer);
            for(auto && feature: layer.getFeatures()){
                outputLayer.addFeature(std::move(feature));
            }
        }
        fishnet::Shapefile output {outputPath};
        fishnet::VectorIO::overwrite(outputLayer, output);
        return output;
    }
};

int main(int argc, char * argv[]){
    using GeometryType = fishnet::geometry::MultiPolygon<fishnet::geometry::Polygon<double>>;
    CLI::App app {"FishnetShapefileMerger"};
    std::vector<std::string> inputFilenames;
    std::string outputFilename;
    app.add_option("-i,--inputs",inputFilenames,"Input Shapefiles storing the settlements and their attributes")->required()->each([](const std::string & str){
        try{
            auto file = fishnet::Shapefile(str);
            if(not file.exists())
                throw std::invalid_argument("File "+ file.getPath().string() + " does not exist");
        }catch(std::invalid_argument & error){
            throw CLI::ValidationError(error.what());
        }
    });
    app.add_option("-o,--output",outputFilename,"Output file location")->required()->check([](const std::string & str){
        try{
            auto file = fishnet::Shapefile(std::filesystem::path(str).filename()); //TODO use current working directory / filename.shp to prevent cwl error in readonly containers
            // std::filesystem::path parentPath = std::filesystem::path(str).parent_path();
            // if(not std::filesystem::exists(parentPath)){
            //     std::filesystem::create_directories(parentPath);
            // }
            return std::string();
        }catch(std::invalid_argument & error){
            return std::string("Invalid output path:\n"+str+"\n")+ error.what();
        }catch(std::filesystem::filesystem_error & fsError){
            return std::string("Filesystem error when checking/creating output path:\n"+str+"\n")+ fsError.what();
        }
    });
    CLI11_PARSE(app,argc,argv);
    if(inputFilenames.empty()){
        throw std::runtime_error("No input files provided");
    }
    std::vector<fishnet::Shapefile> inputs;
    for(auto && inputFilename: inputFilenames) {
        inputs.emplace_back(std::move(inputFilename));
    }
    ShapefileMerger<GeometryType>()(inputs, std::filesystem::path{outputFilename});
    return 0;
}
#include <future>
#include <fishnet/VectorLayer.hpp>
#include <fishnet/GeometryObject.hpp>
#include "Task.hpp"
#include "SettlementPolygon.hpp"

template<fishnet::geometry::Shape ShapeType>
class MergeShapefilesTask: public Task {
private:
    std::vector<fishnet::Shapefile> inputs;
    fishnet::Shapefile output;
    std::string_view taskName;
public:
    MergeShapefilesTask(std::string_view taskName,std::vector<fishnet::Shapefile> && inputs, fishnet::Shapefile output):inputs(std::move(inputs)),output(std::move(output)),taskName(taskName){
        this->desc["type"]=taskName;
        std::vector<std::string> inputStrings;
        std::ranges::for_each(this->inputs,[&inputStrings](auto const & file){inputStrings.push_back(file.getPath().filename().string());});
        this->desc["inputs"]=inputStrings;
        this->desc["output"]=this->output.getPath().string();
    }

    static fishnet::VectorLayer<ShapeType> readSingleInput(const fishnet::Shapefile & input) {
        return fishnet::VectorLayer<ShapeType>::read(input);
    }

    void run() override {
        fishnet::GDALInitializer::init();
        std::vector<std::future<fishnet::VectorLayer<ShapeType>>> futures;
        for(size_t i = 1; i < inputs.size();i++){
            futures.push_back(std::async(std::launch::async,[this,i](){return readSingleInput(inputs[i]);}));
        }
        auto firstLayer = readSingleInput(inputs.front());
        auto outputLayer = fishnet::VectorLayer<ShapeType>::empty(firstLayer);
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
        outputLayer.overwrite(output);
    }
};
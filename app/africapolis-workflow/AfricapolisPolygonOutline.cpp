#include <fishnet/Fishnet.hpp>
#include <CLI/CLI.hpp>
using MultiPolygon_t = fishnet::geometry::MultiPolygon<fishnet::geometry::Polygon<double>>;
using Polygon_t = fishnet::geometry::SimplePolygon<double>;
using Number_t= typename MultiPolygon_t::numeric_type;

template<fishnet::math::Number T>
struct DistanceFromReference{
    fishnet::geometry::Vec2D<T> referencePoint;
    constexpr DistanceFromReference(const fishnet::geometry::Vec2D<T> & referencePoint) noexcept : referencePoint(referencePoint) {}
    constexpr T operator()(const fishnet::geometry::Vec2D<T> & point) const noexcept {
        return point.distance(referencePoint);
    }
};

Polygon_t toBoundaryPolygon(const MultiPolygon_t & multiPolygon) {
    using PointType = fishnet::geometry::Vec2D<typename MultiPolygon_t::numeric_type>;
    auto graph = fishnet::graph::GraphFactory::DirectedGraph<PointType>();
    auto segmentsView = multiPolygon.getPolygons()
            |std::views::transform([](const auto & polygon){return polygon.getBoundary().getSegments();}) 
            | std::views::join;
    std::vector<fishnet::geometry::Segment<Number_t>> segments;
    for (auto&& seg : segmentsView) {
        segments.push_back(std::move(seg)); 
    }
    for(const auto & segment : segments) {
        if(not segment.isValid()) continue; // skip invalid segments
        std::vector<PointType> segmentIntersections;
        segmentIntersections.push_back(segment.p());

        for(const auto & other: segments){
            if(segment == other || not other.isValid()) continue; // skip self
            auto intersection = segment.intersection(other.toLine()); // find intersection and infinite line of other segment
            if(intersection) {
                auto intersectionPoint = intersection.value();
                segmentIntersections.push_back(intersectionPoint); // add intersection point to the current segment intersections
                auto endPointOnOtherSegment = other.p().distance(intersectionPoint) < other.q().distance(intersectionPoint) ? other.p() : other.q();
                graph.addEdge(intersectionPoint, endPointOnOtherSegment); // add edge to the graph for the intersection point and the closest endpoint of the other segment
                graph.addEdge(endPointOnOtherSegment, intersectionPoint); // add edge to the graph for the intersection point and the closest endpoint of the other segment
            }
        }
        segmentIntersections.push_back(segment.q());
        DistanceFromReference<Number_t> distanceFromReference(segment.p());
        // Sort the intersections by distance to the start point of the segment
        std::ranges::sort(segmentIntersections,std::less<Number_t>(),distanceFromReference);
        for(size_t i = 0; i < segmentIntersections.size() - 1; ++i) {
            graph.addEdge(segmentIntersections[i], segmentIntersections[i + 1]);
        }
    }
    PointType topLeft = *std::ranges::max_element(graph.getNodes(),[](const auto & lhs, const auto & rhs) {
        return (lhs.y < rhs.y) || (lhs.y == rhs.y && lhs.x > rhs.x);
    });
    std::vector<PointType> outlinePoints;
    outlinePoints.push_back(topLeft); // start with the top left point
    auto filteredNeighbours = graph.getNeighbours(topLeft) | std::views::filter([&topLeft](const auto & neighbour){
        return neighbour.x > topLeft.x; // filter out neighbours that are not to the right of the top left point
    });
    PointType current = *std::ranges::min_element(
        filteredNeighbours,
        [&topLeft](const auto & lhs, const auto & rhs) {
            return (lhs - topLeft).y < (rhs - topLeft).y; // find neighbour with smallest slope to the top left point 
        }
    );
    PointType prev = topLeft;
    

    while(current != topLeft) {
        outlinePoints.push_back(current);
        auto neighbours = graph.getNeighbours(current) | std::views::filter([previous=prev,currentPoint=current](const auto & neighbour) {
            // auto angle = neighbour.angle(previous) - currentPoint.angle(previous); // Calculate the angle of the neighbour relative to the current point
            // return currentPoint != previous && angle.getAngleValue() < fishnet::math::PI; // filter out the previous point to avoid going back
            return neighbour != previous && neighbour != currentPoint;
        });
        auto edge = current - prev;
        auto angleOffset = current.angle(prev);
        prev = current;
        current = *std::ranges::max_element(neighbours, [&prev, &edge,&angleOffset](const auto& lhs, const auto& rhs) {
            auto crossL = edge.cross(lhs - prev); // Calculate the cross product of the edge and the left neighbour
            auto crossR = edge.cross(rhs - prev); // Calculate the cross product of the edge and the right neighbour
            if(crossL == crossR){
                auto angleL = lhs.angle(prev) - angleOffset ; // Calculate the angle of the left neighbour relative to the previous point
                auto angleR = rhs.angle(prev) - angleOffset; // Calculate the angle of the right neighbour relative to the previous point
                return not (angleL < angleR);
            }
            return crossL < crossR; // Otherwise, sort by cross product
        });

    }
    return Polygon_t{fishnet::geometry::Ring<Number_t>{std::move(outlinePoints)}}; // Create a polygon from the outline points

}

int main(int argc, char* argv[]) {
    using namespace fishnet::geometry;
    CLI::App app{"AfricapolisPolygonOutline"};
    std::string inputFilename = "/home/lolo/Desktop/africapolis/sample/test.shp";
    app.add_option("-i,--input", inputFilename, "Path to input shape file")->required()->check(CLI::ExistingFile);
    // CLI11_PARSE(app, argc, argv);
    fishnet::geometry::Polygon<double> r1 {Ring{Vec2DStd{0,0},Vec2DStd{0,2},Vec2DStd{2,2},Vec2DStd{2,0}}};
    fishnet::geometry::Polygon<double> r2 {Ring{Vec2DStd{3,1},Vec2DStd{4,1},Vec2DStd{4,-1},Vec2DStd{3,-1}}};
    fishnet::geometry::MultiPolygon<fishnet::geometry::Polygon<double>> multiPolygon {r1,r2};
    auto boundaryPolygon = toBoundaryPolygon(multiPolygon);
    std::cout << "Boundary Polygon: " << boundaryPolygon.toString() << std::endl;

    // fishnet::Shapefile inputFile(inputFilename);
    // auto inputLayer = fishnet::VectorIO::read<MultiPolygon_t>(inputFile);
    // auto outputLayer = fishnet::VectorIO::emptyCopy<Polygon_t>(inputLayer);
    // for(const auto & multiPolygonFeature: inputLayer.getFeatures()) {
    //     auto feature = fishnet::Feature<Polygon_t>(toBoundaryPolygon(multiPolygonFeature.getGeometry()));
    //     feature.copyAttributes(multiPolygonFeature);
    //     outputLayer.addFeature(std::move(feature));
    // }
    // fishnet::Shapefile outputFile = {fishnet::util::PathHelper::appendToFilename(inputFile.getPath(), "_outline")};
    // fishnet::VectorIO::overwrite(outputLayer, outputFile);
    // return 0;
}
#include <fishnet/TestUtil.hpp>
#include <fishnet/Polygon.hpp>
#include <fishnet/VectorIO.hpp>
#include <fishnet/PathHelper.h>

#include <functional>
#include <unordered_set>

// Tested class
#include <fishnet/SettlementShape.hpp>

const static std::filesystem::path testFile = fishnet::util::PathHelper::projectDirectory() / "data" / "testing" / "regions" / "Corvara_Small_Preprocessed.shp";
constexpr static size_t TEST_FILE_FEATURE_COUNT = 87;
using geometry_type = fishnet::geometry::OGRPolygonAdapter;
using settlement_type = SettlementShape<geometry_type>;

struct CountingFileRefMapper {
    size_t count = 0;
    FileReference operator()(const auto & file) {
        return FileReference{count++};
    }
};

/**
 * @brief Shapefile reader, which allows to modify the read layer based on the number of previous calls.
 * Used to simulate empty layers or layers with a different spatial reference without additional files on disk.
 */
template<fishnet::geometry::Geometry G>
class TransformingShapefileReader {
public:
    using geometry_type = G;
    using file_type = fishnet::Shapefile;
    using transform_type = std::function<void(size_t, fishnet::VectorLayer<G> &)>;
private:
    transform_type transform;
    size_t calls = 0;
public:
    explicit TransformingShapefileReader(transform_type transform):transform(std::move(transform)){}

    fishnet::Either<fishnet::VectorLayer<G>,std::string> operator()(const fishnet::Shapefile & shapefile) {
        auto layer = fishnet::VectorIO::tryRead(fishnet::ShapefileReader<G>{},shapefile);
        if(layer)
            transform(calls, layer.value());
        calls++;
        return layer;
    }
};

class SettlementShapeTest: public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        referenceLayer = fishnet::VectorIO::read(fishnet::ShapefileReader<geometry_type>{},fishnet::Shapefile(testFile));
        settlements = settlement_type::read(fishnet::Shapefile(testFile),fishnet::ShapefileReader<geometry_type>{},CountingFileRefMapper{});
    }

    static std::vector<geometry_type> referenceGeometries() {
        return fishnet::util::toVector(referenceLayer.getGeometries());
    }

    static inline fishnet::VectorLayer<geometry_type> referenceLayer;
    static inline std::vector<settlement_type> settlements;
};

TEST_F(SettlementShapeTest, ReadSingleFile_ReadsAllFeatures) {
    ASSERT_EQ(referenceLayer.size(), TEST_FILE_FEATURE_COUNT);
    fishnet::test::EXPECT_SIZE(settlements, TEST_FILE_FEATURE_COUNT);
}

TEST_F(SettlementShapeTest, ReadSingleFile_KeysMatchFishnetIdAttribute) {
    auto idField = referenceLayer.getSizeField(Task::FISHNET_ID_FIELD).value_or_throw();
    auto features = fishnet::util::toVector(referenceLayer.getFeatures());
    ASSERT_EQ(settlements.size(), features.size());
    for(size_t i = 0; i < settlements.size(); i++) {
        auto expectedId = features[i].getAttribute(idField);
        ASSERT_TRUE(expectedId.has_value());
        EXPECT_EQ(settlements[i].key(), expectedId.value());
    }
}

TEST_F(SettlementShapeTest, ReadSingleFile_KeysAreUnique) {
    std::unordered_set<size_t> keys;
    for(const auto & settlement : settlements) {
        keys.insert(settlement.key());
    }
    EXPECT_EQ(keys.size(), settlements.size());
}

TEST_F(SettlementShapeTest, ReadSingleFile_GeometryMatchesSource) {
    static_assert(std::same_as<decltype(settlements.front().geometry()), geometry_type>);
    auto geometries = referenceGeometries();
    ASSERT_EQ(settlements.size(), geometries.size());
    for(size_t i = 0; i < settlements.size(); i++) {
        EXPECT_EQ(settlements[i].geometry(), geometries[i]);
    }
}

TEST_F(SettlementShapeTest, ReadSingleFile_FileReferenceFromMapper) {
    size_t mapperCalls = 0;
    auto mapper = [&mapperCalls](const fishnet::Shapefile &) {
        mapperCalls++;
        return FileReference{7};
    };
    auto result = settlement_type::read(fishnet::Shapefile(testFile),fishnet::ShapefileReader<geometry_type>{},mapper);
    EXPECT_EQ(mapperCalls, 1);
    fishnet::test::EXPECT_SIZE(result, TEST_FILE_FEATURE_COUNT);
    for(const auto & settlement : result) {
        EXPECT_EQ(settlement.file(), FileReference{7});
    }
    for(const auto & settlement : settlements) {
        EXPECT_EQ(settlement.file(), FileReference{0});
    }
}

TEST_F(SettlementShapeTest, Constructor_ForwardsGeometryArgs) {
    auto polygon = referenceGeometries().front();
    auto settlement = settlement_type(42, FileReference{7}, polygon);
    EXPECT_EQ(settlement.key(), 42);
    EXPECT_EQ(settlement.hash(), 42);
    EXPECT_EQ(settlement.file(), FileReference{7});
    EXPECT_EQ(settlement.geometry(), polygon);
}

TEST_F(SettlementShapeTest, Equality_IsByKeyOnly) {
    auto geometries = referenceGeometries();
    const auto & first = geometries[0];
    const auto & second = geometries[1];
    ASSERT_NE(first, second);
    EXPECT_EQ(settlement_type(1, FileReference{0}, first), settlement_type(1, FileReference{1}, second));
    EXPECT_NE(settlement_type(1, FileReference{0}, first), settlement_type(2, FileReference{0}, first));
}

TEST_F(SettlementShapeTest, Hash_EqualsKey) {
    for(const auto & settlement : settlements) {
        EXPECT_EQ(settlement.hash(), settlement.key());
    }
}

TEST_F(SettlementShapeTest, Filter_OnlyKeepsMatchingGeometries) {
    auto areas = fishnet::util::toVector(referenceLayer.getGeometries() | std::views::transform([](const auto & g){ return g.area(); }));
    std::ranges::nth_element(areas, areas.begin() + areas.size() / 2);
    const double medianArea = areas[areas.size() / 2];
    auto isLarge = [medianArea](const geometry_type & polygon) { return polygon.area() > medianArea; };

    auto result = settlement_type::read(fishnet::Shapefile(testFile),fishnet::ShapefileReader<geometry_type>{},CountingFileRefMapper{},isLarge);
    auto expectedCount = std::ranges::count_if(referenceGeometries(), isLarge);
    EXPECT_GT(expectedCount, 0);
    EXPECT_LT(expectedCount, TEST_FILE_FEATURE_COUNT);
    fishnet::test::EXPECT_SIZE(result, expectedCount);
    for(const auto & settlement : result) {
        EXPECT_TRUE(isLarge(settlement.geometry()));
    }
}

TEST_F(SettlementShapeTest, Filter_RejectAll_ReturnsEmpty) {
    auto rejectAll = [](const geometry_type &) { return false; };
    auto result = settlement_type::read(fishnet::Shapefile(testFile),fishnet::ShapefileReader<geometry_type>{},CountingFileRefMapper{},rejectAll);
    fishnet::test::EXPECT_EMPTY(result);
}

TEST_F(SettlementShapeTest, ReadRange_MultipleFiles_ConcatenatesAndAssignsFileRefs) {
    auto files = std::vector<fishnet::Shapefile>{fishnet::Shapefile(testFile), fishnet::Shapefile(testFile)};
    auto result = settlement_type::read<fishnet::Shapefile>(files,fishnet::ShapefileReader<geometry_type>{},CountingFileRefMapper{});
    ASSERT_EQ(result.size(), 2 * TEST_FILE_FEATURE_COUNT);
    for(size_t i = 0; i < TEST_FILE_FEATURE_COUNT; i++) {
        const auto & fromFirst = result[i];
        const auto & fromSecond = result[i + TEST_FILE_FEATURE_COUNT];
        EXPECT_EQ(fromFirst.file(), FileReference{0});
        EXPECT_EQ(fromSecond.file(), FileReference{1});
        EXPECT_EQ(fromFirst.key(), fromSecond.key());
        EXPECT_EQ(fromFirst.key(), settlements[i].key());
    }
}

TEST_F(SettlementShapeTest, ReadRange_SkipsEmptyLayers) {
    auto reader = TransformingShapefileReader<geometry_type>{[](size_t call, fishnet::VectorLayer<geometry_type> & layer){
        if(call == 0)
            layer = fishnet::VectorLayer<geometry_type>(layer.getSpatialReference()); // no features and no fields
    }};
    auto files = std::vector<fishnet::Shapefile>{fishnet::Shapefile(testFile), fishnet::Shapefile(testFile)};
    auto result = settlement_type::read<fishnet::Shapefile>(files,reader,CountingFileRefMapper{});
    fishnet::test::EXPECT_SIZE(result, TEST_FILE_FEATURE_COUNT);
    for(const auto & settlement : result) {
        // file reference mapper is not invoked for the skipped empty layer
        EXPECT_EQ(settlement.file(), FileReference{0});
    }
}

TEST_F(SettlementShapeTest, ReadRange_MismatchingSpatialReference_Throws) {
    auto reader = TransformingShapefileReader<geometry_type>{[](size_t call, fishnet::VectorLayer<geometry_type> & layer){
        if(call == 1) {
            OGRSpatialReference webMercator;
            ASSERT_EQ(webMercator.importFromEPSG(3857), OGRERR_NONE);
            layer.setSpatialReference(webMercator);
        }
    }};
    auto files = std::vector<fishnet::Shapefile>{fishnet::Shapefile(testFile), fishnet::Shapefile(testFile)};
    EXPECT_THROW(settlement_type::read<fishnet::Shapefile>(files,reader,CountingFileRefMapper{}), std::runtime_error);
}

TEST_F(SettlementShapeTest, Read_MissingIdField_Throws) {
    EXPECT_THROW(
        settlement_type::read(fishnet::Shapefile(testFile),fishnet::ShapefileReader<geometry_type>{},CountingFileRefMapper{},fishnet::util::TruePredicate{},"NO_FIELD"),
        std::runtime_error
    );
}

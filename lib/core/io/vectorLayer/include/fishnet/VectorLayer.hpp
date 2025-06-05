#pragma once
#include <ranges>
#include <utility>
#include <vector>
#include <algorithm>
#include <expected>
#include <iostream>

#include <fishnet/GeometryObject.hpp>
#include <fishnet/CollectionConcepts.hpp>
#include <fishnet/FunctionalConcepts.hpp>
#include <fishnet/Either.hpp>

#include "FieldType.hpp"
#include "Feature.hpp"

#include <gdal/ogr_spatialref.h>


namespace fishnet {
/**
 * @brief Stores the geometries, wrapped in features (which hold the field values / attributes)
 * Keeps track of the fields available for the features
 * Stores a SpatialReference
 * @tparam G
 */
template<geometry::GeometryObject G>
class VectorLayer{
private:
    OGRSpatialReference spatialRef;
    std::vector<Feature<G>> features;

    /**
     * Fields are stored in a Variant, to be able to use a single container. 
     * This implementation detail does not leak to outside users, but requires std::visit and Variant-Visitors within the code
     */
    std::unordered_map<std::string,FieldDefinitionVariant> fields;

    using error_type = std::string; // error type for std::expected

    /**
     * @brief Helper function to remove certain features. 
     * 
     * @param predicate emits true for Features to be deleted
     */
    constexpr void remove(util::Predicate<Feature<G>> auto && predicate)noexcept {
        const auto removed = std::ranges::remove_if(this->features,predicate);
        this->features.erase(removed.begin(),removed.end());
    }

    template<geometry::GeometryObject T>
    friend class VectorLayer; // allow other VectorLayers to access the private members

public: 
    using geometry_type = G;
    using feature_type = Feature<G>;

    /**
     * @brief Construct a new incomplete Vector Layer object
     * 
     */
    VectorLayer() = default;

    /**
     * @brief Construct a new empty Vector Layer object
     * 
     * @param spatialReference spatial reference which is required to write a GIS-Shapefile to the disk
     */
    explicit VectorLayer(OGRSpatialReference spatialReference):spatialRef(std::move(spatialReference)){}

    constexpr size_t size() const noexcept {
        return this->features.size();
    }

    constexpr bool isEmpty() const noexcept {
        return this->size() == 0;
    }

    constexpr util::view_of<G> auto getGeometries() const noexcept {
        return std::views::all(features) | std::views::transform([](const auto & feature){return feature.getGeometry();});
    }

    constexpr util::view_of<Feature<G>> auto getFeatures() const noexcept {
        return std::views::all(features);
    }

    constexpr util::view_of<Feature<G>> auto getFeatures()  noexcept {
        return std::views::all(features);
    }

    constexpr const OGRSpatialReference & getSpatialReference() const noexcept {
        return this->spatialRef;
    }

    constexpr void setSpatialReference(const OGRSpatialReference & spatialReference) noexcept {
        this->spatialRef = spatialReference;
    }

    constexpr void addGeometry(const G & g) noexcept {
        addFeature(Feature<G>(g));
    }

    constexpr void addGeometry(G && g) noexcept {
        addFeature(Feature<G>(std::move(g)));
    }

    constexpr void addAllGeometry(util::forward_range_of<G> auto && geometries) noexcept {
        std::ranges::for_each(geometries,[this](const auto & g){ addGeometry(g);});
    }

    /**
     * @brief checks if any geometry is equal to the passed geometry
     * WARNING: this method might be slow, since it might compare all geometries of the layer with the passed geometry
     * @param g 
     * @return true 
     * @return false 
     */
    constexpr bool containsGeometry(const G & g) const noexcept {
        return std::ranges::find_if(this->features,[&g](const auto & feature){return feature.getGeometry() == g;}) != std::ranges::end(this->features);
    }


    constexpr void removeGeometry(const G & g) noexcept {
        remove( [&g](const auto & feature){return feature.getGeometry() == g;});
    }

    constexpr void addFeature(Feature<G> && feature) noexcept {
        features.push_back(std::forward<Feature<G>>(feature));
    }

    constexpr void addFeature(const Feature<G> & feature) noexcept {
        features.push_back(feature);
    }

    constexpr bool containsFeature(const Feature<G> & feature) noexcept {
        return std::ranges::contains(this->features, feature);
    }

    constexpr void removeFeature(const Feature<G> & feature) noexcept {
        remove([&feature](const auto & f){return f==feature;});
    }

    /**
     * @brief Add a field of generic type T to the layer. A field is referenced exclusively by its name.
     * 
     * @tparam T value type stored in the field
     * @param fieldName identifier of the field, must not exceed 10 characters
     * @param fieldID optional field ID, if not provided, a unique ID is generated. Use with care!
     * @return constexpr util::Either<FieldDefinition<T>,error_type> 
     */
    template<FieldValueType T>
    [[nodiscard]] constexpr util::Either<FieldDefinition<T>,error_type> addField(const std::string & fieldName, const std::optional<int> & fieldID = std::nullopt) noexcept {
        if(fieldName.length() > 10){
            return std::unexpected("Field name \""+ fieldName+"\" must not exceed a length of 10 characters");
        }
        if(this->fields.contains(fieldName))
            return std::unexpected("Field \"" + fieldName + "\" already exists");
        
        FieldDefinition<T> field = fieldID.transform([&fieldName](int id) { return FieldDefinition<T>(fieldName, id); }).value_or(FieldDefinition<T>(fieldName));
        this->fields.emplace(fieldName, field);
        return field;
    }

    constexpr util::Either<FieldDefinition<int>,error_type> addIntegerField(const std::string & fieldName, const std::optional<int> & fieldID = std::nullopt) noexcept {
        return addField<int>(fieldName,fieldID);
    }

    constexpr util::Either<FieldDefinition<double>,error_type> addDoubleField(const std::string & fieldName, const std::optional<int> & fieldID = std::nullopt) noexcept {
        return addField<double>(fieldName,fieldID);
    }

    constexpr util::Either<FieldDefinition<std::string>,error_type> addTextField(const std::string & fieldName, const std::optional<int> & fieldID = std::nullopt) noexcept {
        return addField<std::string>(fieldName,fieldID);
    }

    constexpr util::Either<FieldDefinition<size_t>,error_type> addSizeField(const std::string & fieldName, const std::optional<int> & fieldID = std::nullopt) noexcept {
        return addField<size_t>(fieldName,fieldID);
    }

    constexpr bool hasField(const std::string & fieldName) const noexcept {
        return this->fields.contains(fieldName);
    }

    constexpr void removeField(const std::string & fieldName) noexcept {
        this->fields.erase(fieldName);
    }

    constexpr void clearFields() noexcept {
        fields.clear();
    }

    template<geometry::GeometryObject T>
    constexpr void copyFields(VectorLayer<T> & other) const noexcept {
        for(auto [fieldName,fieldVariant]:fields){
            other.fields.try_emplace(fieldName,fieldVariant);
        }
    }

    template<FieldValueType T>
    constexpr std::optional<FieldDefinition<T>> getField(const std::string & fieldName) const noexcept {
        std::optional<FieldDefinition<T>> optFieldDef = std::nullopt;
        if(not fields.contains(fieldName)) {
            return std::nullopt;
        }
        // test whether the FieldVariant with name == fieldName has the correct value_type
        std::visit([&optFieldDef](auto && fieldVariant){
            using U = typename  std::decay_t<decltype(fieldVariant)>::value_type;
            if constexpr(std::same_as<T,U>){
                optFieldDef = fieldVariant;
            }
        },fields.at(fieldName));
        return optFieldDef;
    }

    constexpr std::optional<FieldDefinition<double>> getDoubleField(const std::string  & fieldName) const noexcept {
        return getField<double>(fieldName);
    }

    constexpr std::optional<FieldDefinition<int>> getIntegerField(const std::string  & fieldName) const noexcept {
        return getField<int>(fieldName);
    }

    constexpr std::optional<FieldDefinition<size_t>> getSizeField(const std::string & fieldName) const noexcept  {
        return getField<size_t>(fieldName);
    }

    constexpr std::optional<FieldDefinition<std::string>> getTextField(const std::string  & fieldName) const noexcept {
        return getField<std::string>(fieldName);
    }

    constexpr const std::unordered_map<std::string,FieldDefinitionVariant> & getFieldsMap() const noexcept {
        return this->fields;
    }
};
}
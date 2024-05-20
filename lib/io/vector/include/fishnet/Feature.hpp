#pragma once
#include <optional>
#include <fishnet/GeometryObject.hpp>
#include "FieldType.hpp"
#include "FieldDefinition.hpp"

namespace fishnet {

template<typename From, typename To>
concept convertible_without_loss = requires(From f){To{f};};

/**
 * @brief Feature implementation storing a geometry and associated attributes
 * 
 * @tparam G geometry type
 */
template<geometry::GeometryObject G>
class Feature {
private:
    /**
     * @brief Helper class to store an attribute for a field 
     * 
     */
    class FieldValue {
    private:
        FieldType value;
        int fieldID;
    public:
        template<FieldValueType T>
        FieldValue(T value,int fieldID):value(value),fieldID(fieldID) {}

        template<FieldValueType T>
        T getValue() const {
            return std::get<T>(value);
        }

        [[nodiscard]]constexpr FieldType getVariant() const noexcept {
            return value;
        }

        [[nodiscard]]constexpr int getFieldID() const noexcept{
            return fieldID;
        }

        constexpr bool operator==(const FieldValue & other) const noexcept{
            return fieldID==other.fieldID && value == other.value;
        }
    };

    G geometry;
    std::vector<FieldValue> attributes;

    template<FieldValueType T>
    constexpr auto getItPosOfAttribute(const FieldDefinition<T> & fieldDefinition) const noexcept{
        return std::ranges::find_if(attributes,[&fieldDefinition](const auto & value){
            return fieldDefinition.getFieldID() == value.getFieldID();
        });
    }

    template<FieldValueType T>
    constexpr auto getItPosOfAttribute(const FieldDefinition<T> & fieldDefinition) noexcept{
        return std::ranges::find_if(attributes,[&fieldDefinition](const auto & value){
            return fieldDefinition.getFieldID() == value.getFieldID();
        });
    }

public:
    constexpr explicit Feature(const G & geometry):geometry(geometry){}
    constexpr explicit Feature(G && geometry):geometry(std::move(geometry)){}

    template<IFieldDefinition FieldDef>
    constexpr bool addAttribute(const FieldDef & fieldDefinition, convertible_without_loss<typename FieldDef::value_type> auto value) noexcept {
        if (hasAttribute(fieldDefinition))
            return false;
        attributes.emplace_back(static_cast<typename FieldDef::value_type>(value), fieldDefinition.getFieldID());
        return true;
    }

    constexpr bool hasAttribute(const IFieldDefinition auto & fieldDefinition) const noexcept {
        return std::ranges::any_of(attributes,[&fieldDefinition](const auto & fieldValue){
            return fieldDefinition.getFieldID() == fieldValue.getFieldID();
        });
    }

    template<IFieldDefinition FieldDef>
    constexpr std::optional<typename FieldDef::value_type> getAttribute(const FieldDef & fieldDefinition) const noexcept {
        using T = typename FieldDef::value_type;
        auto valueIt = getItPosOfAttribute(fieldDefinition);
        if(valueIt == std::ranges::end(attributes))
            return std::nullopt;
        return std::make_optional<T>(valueIt->template getValue<T>());
    }

    template<IFieldDefinition FieldDef>
    constexpr void setAttribute(const FieldDef & fieldDefinition, std::convertible_to<typename FieldDef::value_type> auto value) noexcept {
        if(not addAttribute(fieldDefinition,value)) {
            *getItPosOfAttribute(fieldDefinition) = FieldValue(static_cast<typename FieldDef::value_type>(value), fieldDefinition.getFieldID());
        }
    }

    constexpr void removeAttribute(const IFieldDefinition auto fieldDefinition) noexcept {
        std::erase_if(attributes,[&fieldDefinition](const auto & value){return value.getFieldID() == fieldDefinition.getFieldID();});
    }

    constexpr const G & getGeometry() const noexcept{
        return this->geometry;
    }

    constexpr auto getGeometry() noexcept {
        return this->geometry;
    }

    constexpr bool operator==(const Feature<G> & feature) const noexcept {
        return this->geometry == feature.getGeometry() &&
               std::ranges::is_permutation(this->attributes, feature.attributes);
    }
};
}
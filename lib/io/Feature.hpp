#pragma once
#include "GeometryObject.hpp"
#include "FieldType.hpp"
#include "FieldDefinition.hpp"

#include <optional>

namespace fishnet {

template<geometry::GeometryObject G>
class Feature {
private:

    class FieldValue {
    private:
        FieldType value;
        size_t fieldID;
    public:
        template<FieldValueType T>
        FieldValue(T value,size_t fieldID):value(value),fieldID(fieldID) {}

        template<FieldValueType T>
        T getValue() const {
            return std::get<T>(value);
        }

        size_t getFieldID() const {
            return fieldID;
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
    constexpr explicit Feature(G & geometry):geometry(geometry){}
    constexpr explicit Feature(G && geometry):geometry(std::move(geometry)){}

    template<FieldValueType T>
    constexpr bool addAttribute(const FieldDefinition<T> & fieldDefinition, std::convertible_to<T> auto value) noexcept {
        if (hasAttribute(fieldDefinition))
            return false;
        attributes.emplace_back(static_cast<T>(value), fieldDefinition.getFieldID());
        return true;
    }

    template<FieldValueType T>
    constexpr bool hasAttribute(const FieldDefinition<T> & fieldDefinition) const noexcept {
        return std::ranges::any_of(attributes,[&fieldDefinition](const auto & fieldValue){
            return fieldDefinition.getFieldID() == fieldValue.getFieldID();
        });
    }

    template<FieldValueType T>
    constexpr std::optional<T> getAttribute(const FieldDefinition<T> & fieldDefinition) const noexcept {
        auto valueIt = getItPosOfAttribute(fieldDefinition);
        if(valueIt == std::ranges::end(attributes))
            return std::nullopt;
        return std::make_optional<T>(valueIt->template getValue<T>());
    }

    template<FieldValueType T>
    constexpr void setAttribute(const FieldDefinition<T> fieldDefinition, std::convertible_to<T> auto value) noexcept {
        if(not addAttribute(fieldDefinition,value)) {
            *getItPosOfAttribute(fieldDefinition) = FieldValue(static_cast<T>(value), fieldDefinition.getFieldID());
        }
    }

    constexpr const G & getGeometry() const noexcept{
        return this->geometry;
    }
};
}

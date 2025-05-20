#pragma once
#include <string>
#include <variant>
#include "FieldType.hpp"
#include <fishnet/GeometryObject.hpp>

// Forward-Declaration needed for testing
template<fishnet::FieldValueType T>
class FieldDefinitionTestFactory;

namespace fishnet {

class FieldCounter{
private:
    static inline int id = 0;
    FieldCounter()= default;
public:
    [[nodiscard]] static int operator()() noexcept {
        return id++;
    }
};

/**
 * @brief Defines an Interface for FieldDefinitions, which can be used as generic parameter
 * 
 * @tparam T field value type
 */
template<class T>
concept IFieldDefinition = FieldValueType<typename T::value_type> && std::equality_comparable<T> && requires(const T & fieldDef){
    typename T::value_type;
    {fieldDef.getFieldName()} -> std::convertible_to<std::string_view>;
    {fieldDef.getFieldID()} -> std::integral;
};

/**
 * @brief Field definition implementation.
 * Defines name, id and data type of a field
 * @note constructors are private, only accessible by friends
 * @tparam T field value type
 */
template<FieldValueType T>
class FieldDefinition {
private:
    friend class FieldDefinitionTestFactory<T>;

    template<geometry::GeometryObject G>
    friend class VectorLayer; 

    std::string fieldName;
    int fieldID;

    constexpr explicit FieldDefinition(std::string fieldName) : fieldName(std::move(fieldName)), fieldID(FieldCounter::operator()()) {}
    constexpr explicit FieldDefinition(std::string fieldName,int id) : fieldName(std::move(fieldName)), fieldID(id) {}

public:
    using value_type = T;

    [[nodiscard]] constexpr const std::string &getFieldName() const noexcept {
        return this->fieldName;
    }

    [[nodiscard]] constexpr int getFieldID() const noexcept {
        return this->fieldID;
    }

    template<FieldValueType U>
    constexpr bool inline operator==(const FieldDefinition<U> & other )const noexcept {
        return this->fieldID == other.getFieldID();
    }
};
}

namespace fishnet::__impl {
template<typename Variant>
struct FieldDefinitionTypes;

template<typename... Types>
struct FieldDefinitionTypes<std::variant<Types...>> {
    using type = std::variant<FieldDefinition<Types>...>; // Construct FieldDefinitionVariant from FieldValueVariant
};
}

namespace fishnet{
using FieldDefinitionVariant = fishnet::__impl::FieldDefinitionTypes<FieldType>::type;
}
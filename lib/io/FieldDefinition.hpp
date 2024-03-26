#pragma once

#include <string>
#include <utility>
#include "FieldType.hpp"
template<fishnet::FieldValueType T>
class FieldDefinitionTestFactory;

namespace fishnet {

template<FieldValueType T>
class FieldDefinition {
private:
    friend class FieldDefinitionTestFactory<T>;

    static inline size_t fieldIDCounter = 0;
    std::string fieldName;
    size_t fieldID;

    constexpr explicit FieldDefinition(std::string fieldName) : fieldName(std::move(fieldName)), fieldID(fieldIDCounter++) {}

public:

    using value_type = T;

    [[nodiscard]] constexpr const std::string &getFieldName() const noexcept {
        return this->fieldName;
    }

    [[nodiscard]] constexpr size_t getFieldID() const noexcept {
        return this->fieldID;
    }
};
}
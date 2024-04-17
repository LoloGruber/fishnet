#pragma once

#include <fishnet/FieldDefinition.hpp>
template<fishnet::FieldValueType T>
class FieldDefinitionTestFactory{
public:
    static fishnet::FieldDefinition<T> createField(std::string fieldName){
        auto field = fishnet::FieldDefinition<T>(fieldName);
        return field;
    }
};
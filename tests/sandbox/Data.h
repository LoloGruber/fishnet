#include <iostream>

static void println(const auto & data){
    std::cout << data << std::endl;
}

template<typename T>
class Data{
private:
    T value;
public:
    Data(T value):value(std::move(value)){
        println("Default Constructor");
    }

    Data(const Data & data){
        this->value = data.value;
        println("Copy Constructor");
    }

    Data(Data && data){
        this->value = std::move(data.value);
        println("Move Constructor");
    }

    Data & operator=(const Data & data){
        this->value = data.value;
        println("Copy Assignment");
        return *this;
    }

    Data & operator=(Data && data){
        this->value = std::move(data.value);
        println("Move Assignment");
        return *this;
    }

    const T & get(){
        return value;
    }
};
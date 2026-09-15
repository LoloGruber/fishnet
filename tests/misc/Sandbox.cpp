#include <iostream>
#include <ranges>
#include <sstream>
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

template<auto C, typename T>
concept Satisfies = requires {
    C.template operator()<T>();
};

#define CONCEPT(TheConcept) \
  [] <typename T> () consteval { }


template<typename R, auto C>
concept range_over = std::ranges::range<R> && Satisfies<C,std::ranges::range_value_t<R>>;



struct Builder{
    std::ostringstream data {std::ios::ate};

    Builder(){
        println("Builder Constructor");
    }

    Builder(Builder && builder){
        this->data = std::move(builder.data);
        println("Builder Move Constructor");
    }

    // Builder(Builder const & builder){
    //     this->data = builder.data;
    //     println("Builder Copy Constructor");
    // }

    Builder & operator=(Builder && builder){
        this-> data = std::move(builder.data);
        println("Builder Move Assignment");
        return *this;
    }

    // Builder & operator=(Builder const & builder) {
    //     this->data = builder.data;
    //     println("Builder Copy Assignment");
    //     return *this;
    // }

    Builder & add(Data<int> && data) &{
        this->data << data.get() << " ";
        return *this;
    }

    Builder && add(Data<int> && data) &&{
        this->data << data.get() << " ";
        return std::move(*this);
    }
    
    std::string build(){
        return data.str();
    }
};



int main(){
    Builder builder = Builder().add(Data(1)).add(Data(2)).add(Data(3)); // Move Constructor of builder called
    std::cout << builder.build() << std::endl;
    std::cout << std::endl;
    auto result = Builder().add(Data(1)).add(Data(2)).add(Data(3)).build();
    std::cout << result << std::endl;
    std::cout << "Destroying Sandbox" << std::endl;
    return 0;
}


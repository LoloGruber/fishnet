#pragma once
#include <iostream>

template<class T>
class Base{
public:
    using node_type = int;
    int function()const{
        return static_cast<const T*>(this)->function();
    }
};

class Derived1 : public Base<Derived1>{
public:
    int function()const{
        return 42;
    }
};

class Derived2: public Base<Derived2>{
public:
    int function()const{
        return -1;
    }
};

template<class Impl>
static void print(const Base<Impl> & base){
    std::cout << base.function() << std::endl;
}


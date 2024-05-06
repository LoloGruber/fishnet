#pragma once
namespace fishnet::util{
template<typename T>
class Element
{
private:
    T e;
    bool present = false;
    constexpr Element(){};
public:

    Element(T && element):e(std::move(element)){present = true;}


    operator bool() const {
        return this->present;
    }

    const static inline Element POISON_PILL = Element();

    T get() const {
        if(present) {
            return e;
        }
        throw std::runtime_error("Value not present");
    }

    T operator->() const{
        return this->get();
    }

    T operator*() const {
        return this->get();
    }


    ~Element() = default;
};
}
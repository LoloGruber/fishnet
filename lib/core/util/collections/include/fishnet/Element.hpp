#pragma once
namespace fishnet::util{
/**
 * @brief Helper Class, wrapping elements in the blocking queue.
 * Additionally specifies a POISON_PILL, indicating that the queue will no longer be used
 * @tparam T 
 */
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

    const static inline Element POISON_PILL = Element(); // Element with present==false and no value

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
};
}
#ifndef TESTUTIL_H
#define TESTUTIL_H
#include <gtest/gtest.h>
#include <memory>
#include <concepts>
#include <algorithm>
#include <ranges>
#include <fishnet/CollectionConcepts.hpp>
#include <fishnet/NumericConcepts.hpp>
namespace testutil{

static void TODO() {
    FAIL() << "Test not implemented yet";
}

template<typename T>
static bool contains(const  util::input_range_of<std::unique_ptr<T>> auto & collection, const std::unique_ptr<T> & element) {
    return collection.end() != std::find_if(collection.begin(),collection.end(),[&element](const std::unique_ptr<T> & current){return *element == *current;});
}


template<typename T>
static bool contains(const util::input_range_of<std::shared_ptr<T>> auto & collection, const std::shared_ptr<T> & element) {
    return collection.end() != std::find_if(collection.begin(),collection.end(),[&element](const std::shared_ptr<T> & current){return *element == *current;});
}

template<typename T, typename U> requires std::derived_from<U,T>
static bool contains(const util::input_range_of<T> auto & collection, const U & element){
    for(auto & e : collection) {
        if(dynamic_cast<const U&>(e) == element){
            return true;
        }
    }
    return false;
}

template<typename T, typename U> requires std::derived_from<U,T>
static bool contains(const  util::input_range_of<std::unique_ptr<T>> auto & collection, const std::unique_ptr<U> & element){
    for(auto & e : collection) {
        if(dynamic_cast< U&>(*e) == *element){
            return true;
        }
    }
    return false;
}

template<typename T>
static bool contains(const util::input_range_of<T> auto & collection, const T & element){
    return collection.end() != std::ranges::find(collection,element);
}

template<typename T>
static bool containsView(util::view_of<T> auto view, const T & element){
    return view.end() != std::ranges::find(view,element);
}

static bool containsAll(const std::ranges::input_range auto & collection, const std::ranges::input_range auto & toBeContained) {
    return std::ranges::all_of(toBeContained.begin(),toBeContained.end(),[&collection](const auto & element){return contains(collection,element);});
}

static bool containsAllView(std::ranges::view auto view, const std::ranges::input_range auto & toBeContained){
    return std::ranges::all_of(toBeContained,[view](const auto & e){return containsView(view,e);});
}

// template<typename T>
// static bool containsAll(const  util::input_range_of<std::unique_ptr<T>> auto & collection, const  util::input_range_of<std::unique_ptr<T>> auto & toBeContained) {
//     return std::all_of(toBeContained.begin(),toBeContained.end(),[&collection](const std::unique_ptr<T> & element){return contains<T>(collection,element);});
// }

// template<typename T>
// static bool containsAll(const util::input_range_of<std::shared_ptr<T>> auto & collection, const util::input_range_of<std::shared_ptr<T>> auto & toBeContained) {
//     return std::all_of(toBeContained.begin(),toBeContained.end(),[&collection](const std::shared_ptr<T> & element){return contains<T>(collection,element);});
// }


// template<typename T, typename U> requires std::derived_from<U,T>
// static bool containsAll(const util::input_range_of<T> auto & collection, const util::input_range_of<U> auto & toBeContained){
//     return std::all_of(toBeContained.begin(),toBeContained.end(),[&collection](const U & element){return contains<T,U>(collection,element);});
// }

// template<typename T, typename U> requires std::derived_from<U,T>
// static bool containsAll(const  util::input_range_of<std::unique_ptr<T>> auto & collection, const util::input_range_of<std::unique_ptr<U>> auto & toBeContained){
//     return std::all_of(toBeContained.begin(),toBeContained.end(),[&collection](const std::unique_ptr<U> & element){return contains<T,U>(collection,element);});
// }





template<typename T>
void EXPECT_CONTAINS( util::input_range_of<T> auto & collection, const T & element){
    EXPECT_NE(std::find(collection.begin(), collection.end(), element), collection.end()) << "Expected Element not contained in Collection";
}

template<typename T>
void EXPECT_CONTAINS( util::input_range_of<T> auto && collection, const T & element){
    EXPECT_NE(std::find(collection.begin(), collection.end(), element), collection.end()) << "Expected Element not contained in Collection";
}

template<typename T>
void EXPECT_NOT_CONTAINS( util::input_range_of<T> auto & collection, const T & element){
    EXPECT_EQ(std::find(collection.begin(), collection.end(), element), collection.end());
}

template<typename T>
void EXPECT_NOT_CONTAINS( util::input_range_of<T> auto && collection, const T & element){
    EXPECT_EQ(std::find(collection.begin(), collection.end(), element), collection.end());
}



template<typename T> 
void EXPECT_CONTAINS(const  util::input_range_of<std::unique_ptr<T>> auto & collection, const std::unique_ptr<T> & element){
    bool result = contains<T>(collection,element);
    EXPECT_TRUE(result);
}

template<typename T> 
void EXPECT_NOT_CONTAINS(const  util::input_range_of<std::unique_ptr<T>> auto & collection, const std::unique_ptr<T> & element){
    bool result = contains<T>(collection,element);
    EXPECT_FALSE(result);
}

template<typename T>
void EXPECT_CONTAINS(const util::input_range_of<std::shared_ptr<T>> auto & collection, const std::shared_ptr<T> & element){
    bool result = contains<T>(collection,element);
    EXPECT_TRUE(result);
}


template<typename T>
void EXPECT_NOT_CONTAINS(const util::input_range_of<std::shared_ptr<T>> auto & collection, const std::shared_ptr<T> & element){
    bool result = contains<T>(collection,element);
    EXPECT_FALSE(result);
}


template<typename T, typename U> requires std::derived_from<U,T>
void EXPECT_CONTAINS(const util::input_range_of<T> auto & collection, const U & element){
    bool result = contains<T,U>(collection,element);
    EXPECT_TRUE(result);
}


template<typename T, typename U> requires std::derived_from<U,T>
void EXPECT_NOT_CONTAINS(const util::input_range_of<T> auto & collection, const U & element){
    bool result = contains<T,U>(collection,element);
    EXPECT_FALSE(result);
}

template<typename T, typename U> requires std::derived_from<U,T>
void EXPECT_CONTAINS(const  util::input_range_of<std::unique_ptr<T>> auto & collection, const std::unique_ptr<U> & element){
    bool result =  contains<T,U>(collection,element);
    EXPECT_TRUE(result);
}

template<typename T, typename U> requires std::derived_from<U,T>
void EXPECT_NOT_CONTAINS(const  util::input_range_of<std::unique_ptr<T>> auto & collection, const std::unique_ptr<U> & element){
    bool result =  contains<T,U>(collection,element);
    EXPECT_FALSE(result);
}


void EXPECT_CONTAINS_ALL(const std::ranges::input_range auto & collection, const std::ranges::input_range auto & toBeContained){
    bool result = containsAll(collection,toBeContained);
    EXPECT_TRUE(result);
}


void EXPECT_CONTAINS_ALL( std::ranges::view auto  view, std::ranges::range auto & toBeContained){
    bool result = containsAllView(view,toBeContained);
    EXPECT_TRUE(result);
}

template<typename T>
concept supportsIOAppend = requires(const T & t, std::ostream & os){
    {os << t} -> std::convertible_to<std::ostream>;
};

void EXPECT_UNSORTED_RANGE_EQ(std::ranges::forward_range auto const & actual, std::ranges::forward_range auto const & expected) {
    if(util::size(actual) != util::size(expected)){
        FAIL() << "Ranges have a different size!"<<"\nExpecting: "<<util::size(expected)<<" but was: " << util::size(actual);
        return;
    }
    for(const auto & expectedElement : expected){
        if constexpr (supportsIOAppend<decltype(expectedElement)>){
            EXPECT_NE(std::ranges::find(actual,expectedElement),std::ranges::end(actual)) <<"Actual does not contain " << expectedElement;
        }
        else {
            EXPECT_NE(std::ranges::find(actual, expectedElement), std::ranges::end(actual))
                                << "Actual does not contain expected element";
        }
    }
}

void EXPECT_RANGE_EQ(const std::ranges::input_range auto &  lhs, const std::ranges::input_range auto & rhs){
    auto actual = std::ranges::cbegin(lhs);
    auto expected = std::ranges::cbegin(rhs);
    while(actual != std::ranges::cend(lhs) and expected != std::ranges::cend(rhs)){
        EXPECT_EQ(*actual,*expected) << "Elements of input ranges are not equal";
        if (*actual != *expected) return;
        ++actual; ++expected;
    }
    if(actual != std::ranges::cend(lhs) or expected != std::ranges::cend(rhs)) FAIL() << "Ranges have a different size!";
}

// template<typename T>
// void EXPECT_CONTAINS_ALL(const  util::input_range_of<std::unique_ptr<T>> auto & collection, const  util::input_range_of<std::unique_ptr<T>> auto & toBeContained){
//     bool result = containsAll<T>(collection,toBeContained);
//     EXPECT_TRUE(result);
// }

// template<typename T>
// void EXPECT_CONTAINS_ALL(const util::input_range_of<std::shared_ptr<T>> auto & collection, const util::input_range_of<std::shared_ptr<T>> auto & toBeContained){
//     bool result = containsAll<T>(collection,toBeContained);
//     EXPECT_TRUE(result);
// }

template<typename T, typename U> requires std::derived_from<U,T>
void EXPECT_CONTAINS_ALL(const util::input_range_of<T> auto & collection, const util::input_range_of<U> auto & toBeContained){
    bool result = containsAll<T,U>(collection,toBeContained);
    EXPECT_TRUE(result);
}

template<typename T, typename U> requires std::derived_from<U,T>
void EXPECT_CONTAINS_ALL(const  util::input_range_of<std::unique_ptr<T>> auto & collection, const util::input_range_of<std::unique_ptr<U>> auto & toBeContained){
    bool result = containsAll<T,U>(collection,toBeContained);
    EXPECT_TRUE(result);
}


template<typename U, typename T>
void EXPECT_BAD_CAST( T & t){
    try{
        auto casted = (U&)(t);
        FAIL();
    }catch(std::bad_cast & e){
        SUCCEED();
    }
}

template<typename T,typename U> requires std::derived_from<U,T>
void EXPECT_REF_EQ(const  util::input_range_of<std::unique_ptr<T>> auto& expected, const util::input_range_of<std::unique_ptr<U>> auto & actual){
    if(expected.size() != actual.size()){
        FAIL();
    }
    for(int i = 0; i < expected.size(); i++) {
        EXPECT_EQ(*expected[i],*actual[i]);
    }
}

template<std::ranges::range R>
void EXPECT_SIZE(R & container, size_t size) {
    auto actual = 0UL;
    if constexpr(std::ranges::sized_range<R>){
        actual = std::ranges::distance(container);
    }else{
        actual = std::ranges::count_if(container,[](const auto & e){return true;});
    }
    EXPECT_EQ(actual,size);
}

template<std::ranges::range R>
void EXPECT_SIZE(R && container, size_t size) {
    EXPECT_EQ(util::size<decltype(container)>(container),size);
}

void EXPECT_EMPTY(std::ranges::range auto && container){
    EXPECT_SIZE(container,0);
}

void EXPECT_EMPTY(std::ranges::range auto & container){
    EXPECT_SIZE(container,0);
}

void EXPECT_IN_RANGE(fishnet::math::Number auto value, fishnet::math::Number auto lower, fishnet::math::Number auto upper){
    EXPECT_TRUE(value >= lower);
    EXPECT_TRUE(value < upper);
}

template<typename ExpectedType>
void EXPECT_TYPE(const auto & value){
    bool hasCorrectType = std::same_as<decltype(value),const ExpectedType &>;
    EXPECT_TRUE(hasCorrectType);
}

template<typename T>
concept OptionalOrExpected = requires(const T & t){
    {t.has_value()} -> std::convertible_to<bool>;
};

void EXPECT_VALUE(const OptionalOrExpected auto & opt) {
    EXPECT_TRUE(opt.has_value());
}

void EXPECT_EMPTY(const OptionalOrExpected auto & opt){
    EXPECT_FALSE(opt.has_value());
}

}

#endif
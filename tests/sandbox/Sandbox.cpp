#include "CRTPExample.h"
#include <iostream>
#include "Concepts.h"
#include <ranges>
#include <vector> 
#include <algorithm>
void concepts(){
    std::cout << cmp([](int a,int b){return 0;},1,1) << std::endl;
    std::cout << add([](int a, int b){return a+b;},1,1) << std::endl;
    TestGraph<int,int> g;
    int from = 1;
    int to = 2;
    addEdgeAndGet(g,from,to);
    // AbstractGraph<MyGraph<int, int>, int, int, Edge<int, int, std::hash<int>, std::equal_to<int> > > test;
    auto weightFunction = [](int x, int y){return x+y;};
    TestWeightedGraph<int,int,decltype(weightFunction)> wg;
    auto we = makeWeightedEdge(wg,1,2);
    auto wfrom = we.getFrom();
    auto weight = we.getWeight();
    addEdgeAndGet(wg,from,to);
    auto other = WeightedEdgeImpl<int,int,decltype(weightFunction)>(1,2, 10);
    std::cout << (we == other) << std::endl;
    // auto error = makeWeightedEdge(g,1,2);
}

// #include <iostream>
// #include <ranges>
// #include <vector>

// template <typename T>
// struct AppendSingleToView {
//     const T& value;

//     template <typename Rng>
//     constexpr auto operator()(Rng&& rng) const {
//         return std::views::transform(std::forward<Rng>(rng), [value = value](auto&&) { return value; });
//     }
// };

// template <typename T>
// constexpr auto appendSingleToView(const T& value) {
//     return AppendSingleToView<T>{value};
// }

// void ranges_append() {
//     std::vector<int> numbers = {1, 2, 3, 4, 5};

//     // Create a view of the original vector
//     auto numbersView = numbers | std::views::all;

//     // Create a new view by appending a single element
//     auto updatedView = appendSingleToView(6);

//     // Print the elements of the updated view
//     for (int num : updatedView) {
//         std::cout << num << " ";
//     }
//     std::cout << std::endl;
// }


void crtp(){
    Derived1 d1;
    Derived2 d2;
    Derived1::node_type x = 1;

    print(d1); // 42
    print(d2); // -1
}

#include <map>
void mapBinarySearch() {
    std::map<int,std::string> map;

    map.emplace(1,"one");
    map.emplace(2,"two");
    map.emplace(3,"three");
    map.emplace(5,"five");
    map.emplace(6,"six");
    map.emplace(8, "eight");
    int query = 5;
    for(auto it = map.lower_bound(query);it != map.end() && abs(query-it->first) <=10; --it ) {
        std::cout << it->first << ":"<<it->second << std::endl;
    }
}

void rangesCountIf(){
    std::vector<int> v{1, 2, 3, 4, 5};
    auto even = [](int x) { return x % 2 == 0; };
    auto filter = v | std::views::filter(even);
    auto n = std::ranges::count_if(filter,[](const auto & e){return true;});
    // n == 2
    std::cout << n<< std::endl;
}






int main(){
    //concepts();
    //crtp();
    //rangesCountIf();
    mapBinarySearch();


    std::cout << "Destroying Sandbox" << std::endl;
    return 0;
}


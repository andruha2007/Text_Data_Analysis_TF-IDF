#include <iostream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <unordered_map>
#include <set>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <cstdint>



int main(){

    std::unordered_map<std::string, int> test;

    test["a"]++;

    std::cout << test["a"];

        test["a"]++;

    std::cout << test["a"];
}

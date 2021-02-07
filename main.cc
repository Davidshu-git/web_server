/**
 * @brief 
 * Copyright (c) 2021, David Shu. All rights reserved.
 * 
 * Use of this source code is governed by a GPL license
 * @author David Shu (a294562476@gmail.com)
 */

#include <iostream>
#include <string>

int main() {
    
    std::cout << "no error" << std::endl;
    int a = 54;
    int b = 45;
    std::string test = std::to_string(a) + "." + std::to_string(b);
    std::cout << test << std::endl;
}

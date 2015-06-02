//
//  main.cpp
//  HoaBinauralMatrixCreator
//
//  Created by Guillot Pierre on 02/06/2015.
//  Copyright (c) 2015 PierreGuillot. All rights reserved.
//

#include <iostream>
#include "../../../HoaLibrary-PD/ThirdParty/HoaLibrary/Sources/Hoa.hpp"

using namespace hoa;


int main(int argc, const char * argv[])
{
    Decoder<Hoa2d, double>::Regular Decoder2D(5, 11);
    double harmonics_2D[5 * 2 + 1];
    
    
    
    std::cout << "Hello, World!\n";
    
    
    return 0;
}

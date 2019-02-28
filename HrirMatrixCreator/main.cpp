//
//  main.cpp
//  HoaBinauralMatrixCreator
//
//  Created by Guillot Pierre on 02/06/2015.
//  Copyright (c) 2015 PierreGuillot. All rights reserved.
//

#include "../Sources/HrirCreator.hpp"

using namespace hoa;

template<Dimension Dim>
void writeSubject(hoa::Subject<Dim>&& subject)
{
    subject.read();
    subject.writeForCPP();
}

int main(int argc, const char * argv[])
{
    cout << "Current folder : " << System::getCurrentFolder() << "\n";
    
    for(auto folder : System::getFolders("../ThirdParty/Listen"))
    {
        writeSubject<Hoa2d>({5, folder});
        writeSubject<Hoa3d>({3, folder});
    }

    return 0;
}

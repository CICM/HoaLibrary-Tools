//
//  main.cpp
//  HoaBinauralMatrixCreator
//
//  Created by Guillot Pierre on 02/06/2015.
//  Copyright (c) 2015 PierreGuillot. All rights reserved.
//

#include "../Sources/HrirCreator.hpp"
#include <climits>
#include <vector>

using namespace hoa;
using namespace std;

int main(int argc, const char * argv[])
{
    cout << "Current folder : " << System::getCurrentFolder() << "\n";
    vector< hoa::Subject< Hoa2d > > sujets2D;
    vector< hoa::Subject< Hoa3d > > sujets3D;
    
    vector<System::Folder> folders(System::getFolders("../ThirdParty/Listen"));
    for(auto it : folders)
    {
        sujets2D.push_back(hoa::Subject<Hoa2d>(5, it));
        sujets3D.push_back(hoa::Subject<Hoa3d>(3, it));
    }
    for(auto it : sujets2D)
    {
        it.read();
        it.writeForCPP();
    }
    for(auto it : sujets3D)
    {
        it.read();
        it.writeForCPP();
    }

    return 0;
}

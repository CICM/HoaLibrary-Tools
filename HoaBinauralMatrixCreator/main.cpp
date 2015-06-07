//
//  main.cpp
//  HoaBinauralMatrixCreator
//
//  Created by Guillot Pierre on 02/06/2015.
//  Copyright (c) 2015 PierreGuillot. All rights reserved.
//

#include "../Sources/Hrir.hpp"

using namespace hoa;
using namespace std;
typedef float sample;

int main(int argc, const char * argv[])
{
    Decoder<Hoa2d, double>::Regular Decoder2D(5, 11);
    vector<Subject<sample>> subject;
    vector<Folder> folders(Folder::get("/Users/Pierre/GitHub/HoaLibrary-Tools/ThirdParty/Listen/"));
    for(auto it : folders)
    {
        subject.push_back(Subject<sample>(it));
    }
    for(auto it : subject)
    {
        cout << it.getName() << "\n";
    }
    //double harmonics_2D[5 * 2 + 1];
    
    cout << "end \n";
    return 0;
}

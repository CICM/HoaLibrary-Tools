//
//  main.cpp
//  HoaBinauralMatrixCreator
//
//  Created by Guillot Pierre on 02/06/2015.
//  Copyright (c) 2015 PierreGuillot. All rights reserved.
//

#include "../Sources/Hrir.hpp"
#include <climits>
using namespace hoa;
using namespace std;
typedef float sample;
static const Dimension dim      = Hoa2d;
static const ulong     order    = 5;

int main(int argc, const char * argv[])
{
    cout << "Current folder : " << System::getCurrentFolder() << "\n";
    vector<Subject<dim, sample>> subject;
    
    vector<System::Folder> folders(System::getFolders("../ThirdParty/Listen"));
    for(auto it : folders)
    {
        subject.push_back(Subject<dim, sample>(order, it));
    }
    for(auto it : subject)
    {
        it.write("matrix.hpp");
    }
    /*
    //cout.setf(std::ios::scientific | std:: ios::showpoint);
    cout.precision(numeric_limits<long double>::digits10);
    cout << numeric_limits<sample>::digits << " "<< 1.999999000087543210123456l;
    //double harmonics_2D[5 * 2 + 1];
    cout << "end \n";
    
    typedef std::numeric_limits<long double > dbl;
    long double d = 3.141592653589790001l;
    cout.precision(dbl::digits10);
    cout << "Pi: " << fixed << d << endl;
     */
    return 0;
}

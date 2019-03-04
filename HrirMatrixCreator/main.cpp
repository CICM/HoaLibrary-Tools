// Copyright (c) 2012-2019 CICM - Universite Paris 8 - Labex Arts H2H.
// Authors :
// 2012: Pierre Guillot, Eliott Paris & Julien Colafrancesco.
// 2012-2015: Pierre Guillot & Eliott Paris.
// 2015: Pierre Guillot & Eliott Paris & Thomas Le Meur (Light version)
// 2016-2017: Pierre Guillot.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

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
    std::cout << "Current folder : " << System::getCurrentFolder() << "\n";
    
    for(auto folder : System::getFolders("../ThirdParty/Listen"))
    {
        std::cout << "reading " << folder.getName() << " folder" << '\n';
        writeSubject<Hoa2d>({5, folder});
        writeSubject<Hoa3d>({3, folder});
    }

    return 0;
}

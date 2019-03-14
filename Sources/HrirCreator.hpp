// Copyright (c) 2012-2019 CICM - Universite Paris 8 - Labex Arts H2H.
// Authors :
// 2012: Pierre Guillot, Eliott Paris & Julien Colafrancesco.
// 2012-2015: Pierre Guillot & Eliott Paris.
// 2015: Pierre Guillot & Eliott Paris & Thomas Le Meur (Light version)
// 2016-2017: Pierre Guillot.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

#pragma once

#include "Config.hpp"
#include "System.hpp"
#include "Subject.hpp"

namespace hoa::hrir_matrix_creator
{
    template<Dimension Dim>
    void writeSubject(Subject<Dim>&& subject)
    {
        subject.read();
        subject.writeForCPP();
    }
    
    void writeCppFileForConfig(Config& config);
    void writeCppFileForConfig(Config& config)
    {
        switch(config.dimension)
        {
            case hoa::Hoa2d : { writeSubject<Hoa2d>({config}); break;}
            case hoa::Hoa3d : { writeSubject<Hoa3d>({config}); break;}
        }
    }
}

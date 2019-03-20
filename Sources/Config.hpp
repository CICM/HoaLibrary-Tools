// Copyright (c) 2012-2019 CICM - Universite Paris 8 - Labex Arts H2H.
// Authors :
// 2012: Pierre Guillot, Eliott Paris & Julien Colafrancesco.
// 2012-2015: Pierre Guillot & Eliott Paris.
// 2015: Pierre Guillot & Eliott Paris & Thomas Le Meur (Light version)
// 2016-2017: Pierre Guillot.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

#pragma once

#include "System.hpp"
#include "../ThirdParty/HoaLibrary/Sources/Hoa.hpp"

#include <set>

namespace hoa::hrir_matrix_creator
{
    enum class HrirDatabase
    {
        Listen = 0,
        Sadie
    };
    
    struct Config
    {
        size_t order = 0;                           //! required
        System::Folder wave_folder {};              //! required
        std::string classname {};                   //! required
        Dimension dimension = Dimension::Hoa2d;     //! required
        HrirDatabase database_type = {};            //! required
        
        std::string filename_prefix = "Hoa_Hrir_";  //! optional
        std::string file_extension = ".hpp";        //! optional
        std::string output_directory = "./";        //! optional
        std::set<std::string> wave_files = {};      //! optional
        std::string notes {};                       //! optional
    };
}

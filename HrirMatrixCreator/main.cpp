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
using namespace hrir_matrix_creator;

int main(int argc, const char * argv[])
{
    std::cout << "Current folder : " << System::getCurrentFolder() << "\n";
    
    using namespace std::string_literals;
    
    const auto database_path = "../ThirdParty/HrirDatabase"s;
    const auto Sadie_database_path = database_path + "/Sadie";
    const auto Listen_database_path = database_path + "/Listen";
    
    std::vector<Config> configs;
    
    Config main_config {};
    
    main_config.output_directory = "../Results/";
    
    // Listen 1002C subject config
    {
        Config listen_1002C = main_config;
        
        listen_1002C.database_type = HrirDatabase::Listen;
        listen_1002C.classname = "Listen_1002C";
        listen_1002C.wave_folder = {Listen_database_path, "IRC_1002_C"};
        
        listen_1002C.notes = "link: ftp://ftp.ircam.fr/pub/IRCAM/equipes/salles/listen/archive/SUBJECTS/IRC_1002.zip";
        
        // 2D config
        {
            Config config = listen_1002C;
            config.dimension = Dimension::Hoa2d;
            config.order = 5;
            configs.emplace_back(std::move(config));
        }
        
        // 3D config
        {
            Config config = listen_1002C;
            config.dimension = Dimension::Hoa3d;
            config.order = 3;
            configs.emplace_back(std::move(config));
        }
    }
    
    // Sadie D2 subject config
    {
        Config sadie_d2 = main_config;
        
        sadie_d2.database_type = HrirDatabase::Sadie;
        sadie_d2.classname = "Sadie_D2";
        sadie_d2.wave_folder = {Sadie_database_path + "/D2_HRIR_WAV", "44K_16bit"};
        
        sadie_d2.notes = "link: https://www.york.ac.uk/sadie-project/Resources/SADIEIIDatabase/D2/D2_HRIR_WAV.zip";
        
        // 2D config
        {
            Config config = sadie_d2;
            config.dimension = Dimension::Hoa2d;
            config.order = 5;
            configs.emplace_back(std::move(config));
        }
        
        // 3D config
        {
            Config config = sadie_d2;
            config.dimension = Dimension::Hoa3d;
            config.order = 3;
            configs.emplace_back(std::move(config));
        }
    }
    
    for(auto& config : configs)
    {
        writeCppFileForConfig(config);
    }

    return 0;
}

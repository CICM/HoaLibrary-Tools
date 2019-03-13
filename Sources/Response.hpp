// Copyright (c) 2012-2019 CICM - Universite Paris 8 - Labex Arts H2H.
// Authors :
// 2012: Pierre Guillot, Eliott Paris & Julien Colafrancesco.
// 2012-2015: Pierre Guillot & Eliott Paris.
// 2015: Pierre Guillot & Eliott Paris & Thomas Le Meur (Light version)
// 2016-2017: Pierre Guillot.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

#pragma once

#include "../ThirdParty/LibSndFile/src/sndfile.hh"

#include "Config.hpp"
#include <type_traits>

namespace hoa::hrir_matrix_creator
{
    // ================================================================================ //
    // Response
    // ================================================================================ //
    
    //! @brief The response class owns the informations of an impulse response wav file.
    class Response
    : public System::File
    {
    public:
        
        Response(System::File const& file, HrirDatabase const& database_type)
        : System::File(file)
        {
            switch(database_type)
            {
                case HrirDatabase::Listen : { parseListenFile(); break;}
                case HrirDatabase::Sadie : { parseSadieFile(); break;}
            }
        }
        
        void read()
        {
            SndfileHandle file(getFullName());
            if(file && file.channels() == 2)
            {
                try
                {
                    m_values.resize(size_t(file.channels() * file.frames()));
                }
                catch(std::exception& e)
                {
                    std::cout << e.what();
                    m_values.clear();
                    return;
                }
                size_t count = (size_t)file.read(m_values.data(), sf_count_t(file.channels() * file.frames()));
                
                if(count != m_values.size())
                {
                    m_values.clear();
                    std::cerr << count << "[!] error " << '\n';
                }
            }
            else
            {
                m_values.clear();
                std::cerr << "can't load wav file : " << getFullName() << "\n";
            }
        }
        
        ~Response() = default;
        
        double getRadius() const
        {
            return m_radius;
        }
        
        double getAzimuth() const
        {
            return m_azimuth;
        }
        
        double getElevation() const
        {
            return m_elevation;
        }
        
        size_t getNumberOfSamplesPerChannel() const
        {
            return m_values.size() / 2;
        }
        
        double getSample(size_t channel, size_t index) const
        {
            if(channel < 2 && index < getNumberOfSamplesPerChannel())
            {
                return m_values[index * 2 + channel];
            }
            
            return 0;
        }
        
        bool isValid() const override
        {
            return m_valid;
        }
        
    private:
        
        void parseSadieFile()
        {
            // ex: "azi_13,0_ele_-64,8.wav"
            
            std::string name = getName();
            std::string delimiter = "_";
            
            size_t pos = 0;
            std::vector<std::string> tokens;
            while ((pos = name.find(delimiter)) != std::string::npos)
            {
                tokens.emplace_back(name.substr(0, pos));
                name.erase(0, pos + delimiter.length());
            }
            tokens.emplace_back(name);
            
            if(tokens.size() == 4 && tokens[0] == "azi" && tokens[2] == "ele")
            {
                std::replace(tokens[1].begin(), tokens[1].end(), ',', '.');
                std::replace(tokens[3].begin(), tokens[3].end(), ',', '.');
                
                m_azimuth = std::stod(tokens[1]) / 360. * HOA_2PI;
                m_elevation = std::stod(tokens[3]) / 360. * HOA_2PI;
                m_valid = true;
            }
        }
        
        void parseListenFile()
        {
            // ex: IRC_1002_C_R0195_T180_P060.wav
            
            std::string name = getName();
            std::string::size_type pos = name.find("_T");
            
            if(pos != std::string::npos && pos < name.size() - 2)
            {
                if(std::isdigit(name[pos+2]))
                {
                    name.erase(name.begin(), name.begin()+long(pos)+2);
                    m_azimuth = stod(name) / 360. * HOA_2PI;
                    pos = name.find("_P");
                    if(pos != std::string::npos && pos < name.size() - 2)
                    {
                        if(std::isdigit(name[pos+2]))
                        {
                            name.erase(name.begin(), name.begin()+long(pos)+2);
                            m_elevation = stod(name) / 360. * HOA_2PI;
                            m_valid = true;
                        }
                    }
                }
            }
        }
        
        std::vector<double> m_values {};
        double              m_radius = 1.;
        double              m_azimuth = 0.;
        double              m_elevation = 0.;
        size_t              m_size = 0;
        bool                m_valid = false;
    };
}

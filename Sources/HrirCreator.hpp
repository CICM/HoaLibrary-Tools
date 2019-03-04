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
#include "../ThirdParty/LibSndFile/src/sndfile.hh"

namespace hoa
{
    // ================================================================================ //
    // Response
    // ================================================================================ //
    
    //! @brief The response class owns the informations of an impulse response.
    class Response
    : public System::File
    {
    public:
        
        Response(System::File const& file)
        : System::File(file)
        {
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
        
        inline ~Response() noexcept {m_values.clear();}
        inline double getRadius() const noexcept {return m_radius;}
        inline double getAzimuth() const noexcept {return m_azimuth;}
        inline double getElevation() const noexcept {return m_elevation;}
        inline size_t getNumberOfSamplesPerChannel() const noexcept {return m_values.size() / 2;}
        inline double getSample(size_t channel, size_t index) const noexcept {
            return (channel < 2 && index < getNumberOfSamplesPerChannel()) ?  m_values[index * 2 + channel] : 0;}
        bool isValid() const override {return m_valid;}
        
    private:
        
        std::vector<double> m_values {};
        double              m_radius = 1.;
        double              m_azimuth = 0.;
        double              m_elevation = 0.;
        size_t              m_size = 0;
        bool                m_valid = false;
    };
    
    static char const* const get_cpp_file_header_text()
    {
        static char const* const text = "// Copyright (c) 2012-2019 CICM - Universite Paris 8 - Labex Arts H2H.\n"
        "// Authors :\n"
        "// 2012: Pierre Guillot, Eliott Paris & Julien Colafrancesco.\n"
        "// 2012-2015: Pierre Guillot & Eliott Paris.\n"
        "// 2015: Pierre Guillot & Eliott Paris & Thomas Le Meur (Light version)\n"
        "// 2016-2017: Pierre Guillot.\n"
        "// For information on usage and redistribution, and for a DISCLAIMER OF ALL\n"
        "// WARRANTIES, see the file, \"LICENSE.txt,\" in this distribution.\n\n";
        
        return text;
    }
    
    // ================================================================================ //
    // Subject
    // ================================================================================ //
    
    template<Dimension D> class Subject {};
    
    // ================================================================================ //
    // Subject 2D
    // ================================================================================ //
    
    //! @brief The subject owns the impulse response of a subject.
    template <>
    class Subject<Hoa2d>
    : private Processor<Hoa2d, double>::Harmonics
    {
    public:
        
        using processor_t = Processor<Hoa2d, double>::Harmonics;
        
        Subject(const size_t order, const System::Folder& folder)
        : processor_t(order)
        , m_folder(folder)
        {}
        
        ~Subject() = default;
        
        std::string const& getName() const
        {
            return m_folder.getName();
        }
        
        size_t getNumberOfResponses() const noexcept
        {
            return m_responses.size();
        }
        
        size_t getResponsesSize() const noexcept
        {
            return m_size;
        }
        
        size_t getDecompositionOrder() const noexcept
        {
            return processor_t::getDecompositionOrder();
        }
        
        size_t getNumberOfHarmonics() const noexcept
        {
            return processor_t::getNumberOfHarmonics();
        }
        
        inline size_t getMatricesSize() const noexcept
        {
            return getResponsesSize() * getNumberOfHarmonics();
        }
        
        void read()
        {
            for(auto file : m_folder.getFiles(".wav"))
            {
                Response temp(file);
                if(temp.isValid() && temp.getElevation() == 0)
                {
                    m_responses.push_back(temp);
                    m_responses[m_responses.size()-1].read();
                    const auto size = m_responses[m_responses.size()-1].getNumberOfSamplesPerChannel();
                    if(m_size < size)
                    {
                        m_size = size;
                    }
                }
            }
            
            m_left.resize(getMatricesSize());
            fill(m_left.begin(), m_left.end(), 0.);
            m_right.resize(getMatricesSize());
            fill(m_right.begin(), m_right.end(), 0.);
            
            std::vector<double> harmonics (getNumberOfHarmonics(), 0.);
            const auto order = getDecompositionOrder();
            Encoder<Hoa2d, double>::Basic encoder(order);
            
            for(auto& response : m_responses)
            {
                encoder.setAzimuth(response.getAzimuth());
                
                for(size_t j = 0; j < getResponsesSize(); j++)
                {
                    const double left = response.getSample(0, j) / double(order + 1.);
                    encoder.process(&left, harmonics.data());
                    harmonics[0] *= 0.5;
                    Signal<double>::add(harmonics.size(), harmonics.data(),
                                        m_left.data() + j * harmonics.size());
                    
                    const double right = response.getSample(1, j) / double(order + 1.);
                    encoder.process(&right, harmonics.data());
                    harmonics[0] *= 0.5;
                    Signal<double>::add(harmonics.size(), harmonics.data(),
                                        m_right.data() + j * harmonics.size());
                }
            }
        }

        void writeForCPP()
        {
            std::string name = getName();
            size_t size = name.size();
            for(size_t i = 1; i < size; i++)
            {
                if(name[i] == '_')
                {
                    name.erase(name.begin() + long(i));
                    size--;
                }
                else
                {
                    name[i] = (char)tolower(name[i]);
                }
            }
            
            const auto filename = "../Results/Hoa_Hrir" + name + "2D.hpp";
            std::ofstream file(filename);
            if(file.is_open())
            {
                file << get_cpp_file_header_text();
                
                file << "#pragma once\n\n";
                
                file << "// Order of Decomposition : " + std::to_string(getDecompositionOrder()) + "\n";
                file << "// Number of Harmonics    : " + std::to_string(getNumberOfHarmonics()) + "\n";
                file << "// Size of the Responses  : " + std::to_string(getResponsesSize()) + "\n";
                file << "// Size of the matrices   : " + std::to_string(getMatricesSize()) + "\n\n";
                
                file << "namespace hoa\n{\n";
                file.precision(std::numeric_limits<float>::digits10);
                
                file << "    static const float " + name + "_float_2d_left[] = {";
                
                for(size_t i = 0; i < m_left.size() - 1; i++)
                {
                    file << static_cast<float>(m_left[i]) << "f, ";
                }
                file << static_cast<float>(m_left[m_right.size()-1]) << "f};\n";
                
                file << '\n';
                
                file << "    static const float " + name + "_float_2d_right[] = {";
                for(size_t i = 0; i < m_right.size() - 1; i++)
                {
                    file << static_cast<float>(m_right[i]) << "f, ";
                }
                file << static_cast<float>(m_right[m_right.size()-1]) << "f};\n";
                
                file.precision(std::numeric_limits<double>::digits10);
                
                file << '\n';
                file << "\n    static const double " + name + "_double_2d_left[] = {";
                for(size_t i = 0; i < m_left.size() - 1; i++)
                {
                    file << m_left[i] << ", ";
                }
                file << m_left[m_left.size()-1] << "};\n";
                
                file << "    static const double " + name + "_double_2d_right[] = {";
                for(size_t i = 0; i < m_right.size() - 1; i++)
                {
                    file << m_right[i] << ", ";
                }
                file << m_right[m_right.size()-1] << "};\n";
                
                file <<"\n}\n";
                file.close();
                
                std::cout << name << " 2d response written" << "\n";
            }
            else
            {
                std::cerr << "[!] error - can't read " << filename << '\n';
            }
            
        }
        
    private:
        
        const System::Folder    m_folder;
        std::vector<Response>   m_responses = {};
        size_t                  m_size = 0;
        std::vector<double>     m_left = {};
        std::vector<double>     m_right = {};
    };
    
    // ================================================================================ //
    // Subject 3D
    // ================================================================================ //
    
    //! @brief The subject owns the impulse response of a subject.
    template <>
    class Subject<Hoa3d>
    : private Processor<Hoa3d, double>::Harmonics
    {
    public:
        
        using processor_t = Processor<Hoa3d, double>::Harmonics;
        
        Subject(const size_t order, System::Folder const& folder)
        : processor_t(order)
        , m_folder(folder)
        {}
        
        ~Subject() = default;
        
        std::string const& getName() const {return m_folder.getName();};
        
        inline size_t getNumberOfResponses() const noexcept
        {
            return m_responses.size();
        }
        
        inline size_t getResponsesSize() const noexcept
        {
            return m_size;
        }
        
        inline size_t getDecompositionOrder() const noexcept
        {
            return processor_t::getDecompositionOrder();
        }
        
        inline size_t getNumberOfHarmonics() const noexcept
        {
            return processor_t::getNumberOfHarmonics();
        }
        
        inline size_t getMatricesSize() const noexcept
        {
            return getResponsesSize() * getNumberOfHarmonics();
        }
        
        void read()
        {
            for(auto file : m_folder.getFiles(".wav"))
            {
                Response temp(file);
                
                if(temp.isValid())
                {
                    m_responses.push_back(temp);
                    m_responses[m_responses.size()-1].read();
                    const auto size = m_responses[m_responses.size()-1].getNumberOfSamplesPerChannel();
                    if(m_size < size)
                    {
                        m_size = size;
                    }
                }
            }
            
            m_left.resize(getMatricesSize());
            fill(m_left.begin(), m_left.end(), 0.);
            m_right.resize(getMatricesSize());
            fill(m_right.begin(), m_right.end(), 0.);
            
            std::vector<double> harmonics (getNumberOfHarmonics(), 0.);
            
            Encoder<Hoa3d, double>::Basic encoder(getDecompositionOrder());
            for(auto& response : m_responses)
            {
                encoder.setAzimuth(response.getAzimuth());
                encoder.setElevation(response.getElevation());
                for(size_t j = 0; j < getResponsesSize(); j++)
                {
                    const double left  = response.getSample(0, j) / double(getNumberOfResponses());
                    encoder.process(&left, harmonics.data());
                    
                    for(size_t k = 0; k < harmonics.size(); k++)
                    {
                        const size_t l = encoder.getHarmonicDegree(k);
                        if(encoder.getHarmonicOrder(k) == 0)
                        {
                            harmonics[k] *= (2. * l + 1.);
                        }
                        else
                        {
                            harmonics[k] *= double(2. * l + 1.) * 4. * HOA_PI;
                        }
                    }
                    
                    Signal<double>::add(harmonics.size(), harmonics.data(),
                                        m_left.data() + j * harmonics.size());
                    
                    const double right = response.getSample(1, j) / double(getNumberOfResponses());
                    encoder.process(&right, harmonics.data());
                    
                    for(size_t k = 0; k < harmonics.size(); k++)
                    {
                        const size_t l = encoder.getHarmonicDegree(k);
                        if(encoder.getHarmonicOrder(k) == 0)
                        {
                            harmonics[k] *= (2. * l + 1.);
                        }
                        else
                        {
                            harmonics[k] *= double(2. * l + 1.) * 4. * HOA_PI;
                        }
                    }
                    
                    Signal<double>::add(harmonics.size(), harmonics.data(),
                                        m_right.data() + j * harmonics.size());
                }
            }
        }
        
        void writeForCPP()
        {
            std::string name = getName();
            size_t size = name.size();
            for(size_t i = 1; i < size; i++)
            {
                if(name[i] == '_')
                {
                    name.erase(name.begin() + long(i));
                    size--;
                }
                else
                {
                    name[i] = (char)tolower(name[i]);
                }
            }
            
            const auto filename = "../Results/Hoa_Hrir" + name + "3D.hpp";
            std::ofstream file(filename);
            if(file.is_open())
            {
                file << get_cpp_file_header_text();
                
                file << "#pragma once\n\n";
                
                file << "// Order of Decomposition : " + std::to_string(getDecompositionOrder()) + "\n";
                file << "// Number of Harmonics    : " + std::to_string(getNumberOfHarmonics()) + "\n";
                file << "// Size of the Responses  : " + std::to_string(getResponsesSize()) + "\n";
                file << "// Size of the matrices   : " + std::to_string(getMatricesSize()) + "\n\n";
                
                file << "namespace hoa\n{\n";
                file.precision(std::numeric_limits<float>::digits10);
                
                file << "    static const float " + name + "_float_3d_left[] = {";
                
                for(size_t i = 0; i < m_left.size() - 1; i++)
                {
                    file << static_cast<float>(m_left[i]) << "f, ";
                }
                file << static_cast<float>(m_left[m_left.size()-1]) << "f};\n";
                
                file << '\n';
                
                file << "    static const float " + name + "_float_3d_right[] = {";
                for(size_t i = 0; i < m_right.size() - 1; i++)
                {
                    file << static_cast<float>(m_right[i]) << "f, ";
                }
                file << static_cast<float>(m_right[m_right.size()-1]) << "f};\n";
                
                file.precision(std::numeric_limits<double>::digits10);
                
                file << '\n';
                file << "\n    static const double " + name + "_double_3d_left[] = {";
                for(size_t i = 0; i < m_left.size() - 1; i++)
                {
                    file << m_left[i] << ", ";
                }
                file << m_left[m_left.size()-1] << "};\n";
                
                file << "    static const double " + name + "_double_3d_right[] = {";
                for(size_t i = 0; i < m_right.size() - 1; i++)
                {
                    file << m_right[i] << ", ";
                }
                file << m_right[m_right.size()-1] << "};\n";
                
                file <<"\n}\n";
                
                file.close();
                std::cout << name << " 3d response written" << "\n";
            }
            else
            {
                std::cerr << "[!] error - can't read " << filename << '\n';
            }
        }
        
    private:
        
        const System::Folder    m_folder;
        std::vector<Response>   m_responses = {};
        size_t                  m_size = 0;
        std::vector<double>     m_left = {};
        std::vector<double>     m_right = {};
    };
}

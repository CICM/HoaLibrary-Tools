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
        
        std::vector<double> m_values {};
        double              m_radius = 1.;
        double              m_azimuth = 0.;
        double              m_elevation = 0.;
        size_t              m_size = 0;
        bool                m_valid = false;
    };
    
    // ================================================================================ //
    // Subject
    // ================================================================================ //
    
    template<Dimension Dim>
    class Subject
    {
    public:
        
        enum BinauralSide
        {
            Left = 0,
            Right
        };
        
        using processor_t = ProcessorHarmonics<Dim, double>;
        using encoder_t = Encoder<Dim, double>;
        
        Subject(const size_t order, System::Folder const& folder)
        : m_processor(order)
        , m_folder(folder)
        {}
        
        ~Subject() = default;
        
        std::string const& getName() const
        {
            return m_folder.getName();
        }
        
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
            return m_processor.getDecompositionOrder();
        }
        
        inline size_t getNumberOfHarmonics() const noexcept
        {
            return m_processor.getNumberOfHarmonics();
        }
        
        inline size_t getMatricesSize() const noexcept
        {
            return getResponsesSize() * getNumberOfHarmonics();
        }
        
        void read();
        
        void writeForCPP()
        {
            const auto filepath_base = "../Results/Hoa_Hrir";
            const std::string name = getFormattedName();
            const auto dim_str = (Dim == Hoa2d) ? "2D" : "3D";
            const auto extension = ".hpp";
            const auto filename = filepath_base + name + dim_str + extension;
            
            std::ofstream file(filename);
            if(!file.is_open())
            {
                std::cerr << "[!] error - can't read " << filename << '\n';
                return;
            }
            
            // --- write data to file --- //
            
            file << get_cpp_file_header_text();
            
            file << "#pragma once\n\n";
            
            file << "// Order of Decomposition : " + std::to_string(getDecompositionOrder()) + "\n";
            file << "// Number of Harmonics    : " + std::to_string(getNumberOfHarmonics()) + "\n";
            file << "// Size of the Responses  : " + std::to_string(getResponsesSize()) + "\n";
            file << "// Size of the matrices   : " + std::to_string(getMatricesSize()) + "\n\n";
            
            file << "namespace hoa\n{\n";
            
            writeData<float, BinauralSide::Left>(file, name, m_left);
            writeData<float, BinauralSide::Right>(file, name, m_right);
            writeData<double, BinauralSide::Left>(file, name, m_left);
            writeData<double, BinauralSide::Right>(file, name, m_right);
            
            file << "}\n"; // end of hoa namespace
            
            file.close();
            std::cout << name << " " << dim_str << " response written" << "\n";
        }
        
    private: // methods
        
        static char const* const get_cpp_file_header_text()
        {
            static char const* const text = "// Copyright (c) 2012-2019 CICM - Universite Paris 8 - Labex Arts H2H.\n"
            "// Authors :\n"
            "// 2012: Pierre Guillot, Eliott Paris & Julien Colafrancesco.\n"
            "// 2012-2015: Pierre Guillot & Eliott Paris.\n"
            "// 2015: Pierre Guillot & Eliott Paris & Thomas Le Meur (Light version)\n"
            "// 2016-2017: Pierre Guillot.\n"
            "// For information on usage and redistribution, and for a DISCLAIMER OF ALL\n"
            "// WARRANTIES, see the file, \"LICENSE.txt,\" in this distribution.\n\n"
            "// This file has been generated by https://github.com/CICM/HoaLibrary-Tools \n\n";
            
            return text;
        }
        
        std::string getFormattedName()
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
            
            return name;
        }
        
        template<typename FloatType, BinauralSide Side>
        void writeData(std::ofstream& file, std::string const& name, std::vector<double>& data)
        {
            file.precision(std::numeric_limits<FloatType>::digits10);
            
            const auto dim_str = (Dim == Hoa2d) ? "2d" : "3d";
            const auto float_type_str = std::is_same<FloatType, float>::value ? "float" : "double";
            const auto float_type_suffix = std::is_same<FloatType, float>::value ? "f" : "";
            const auto side_str = Side == BinauralSide::Left ? "left" : "right";
            const auto tab = "    ";
            
            file << tab << "static const " << float_type_str << " "
            << name << "_" << float_type_str << "_" << dim_str << "_" << side_str << "[] = {";
            
            auto* f = data.data();
            for(size_t i = 0; i < data.size(); ++f, ++i)
            {
                file << static_cast<FloatType>(*f) << float_type_suffix;
                
                // don't add comma for the last value
                if(i < data.size() - 1)
                {
                    file << ", ";
                }
            }
            
            file << "};\n\n";
        }
        
    private: // variables
        
        const processor_t       m_processor;
        const System::Folder    m_folder;
        std::vector<Response>   m_responses = {};
        size_t                  m_size = 0;
        std::vector<double>     m_left = {};
        std::vector<double>     m_right = {};
    };
    
    // ================================================================================ //
    // Subject 2D read
    // ================================================================================ //

    template<>
    void Subject<Hoa2d>::read()
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
        encoder_t encoder(order);
        
        auto processSample = [&] (double input_sample, double* output_buffer) {
            
            encoder.process(&input_sample, harmonics.data());
            harmonics[0] *= 0.5;
            Signal<double>::add(harmonics.size(), harmonics.data(), output_buffer);
        };
        
        for(auto& response : m_responses)
        {
            encoder.setAzimuth(response.getAzimuth());
            
            for(size_t j = 0; j < getResponsesSize(); j++)
            {
                const auto index = j * harmonics.size();
                
                const double left = response.getSample(0, j) / double(order + 1.);
                processSample(left, m_left.data() + index);
                
                const double right = response.getSample(1, j) / double(order + 1.);
                processSample(right, m_right.data() + index);
            }
        }
    }
    
    // ================================================================================ //
    // Subject 3D read
    // ================================================================================ //
    
    template<>
    void Subject<Hoa3d>::read()
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
        const auto order = getDecompositionOrder();
        encoder_t encoder(order);
        
        auto processSample = [&] (double input_sample, double* output_buffer) {
            
            encoder.process(&input_sample, harmonics.data());
            
            for(size_t k = 0; k < harmonics.size(); k++)
            {
                const size_t l = encoder.getHarmonicDegree(k);
                harmonics[k] *= double(2. * l + 1.);
            }
            
            Signal<double>::add(harmonics.size(), harmonics.data(), output_buffer);
        };
        
        const double number_of_responses = getNumberOfResponses();
        
        for(auto const& response : m_responses)
        {
            encoder.setAzimuth(response.getAzimuth());
            encoder.setElevation(response.getElevation());
            
            for(size_t j = 0; j < getResponsesSize(); j++)
            {
                const auto index = j * harmonics.size();
                
                const double left = response.getSample(0, j) / number_of_responses;
                processSample(left, m_left.data() + index);
                
                const double right = response.getSample(1, j) / number_of_responses;
                processSample(right, m_right.data() + index);
            }
        }
    }
}

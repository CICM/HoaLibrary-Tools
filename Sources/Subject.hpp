// Copyright (c) 2012-2019 CICM - Universite Paris 8 - Labex Arts H2H.
// Authors :
// 2012: Pierre Guillot, Eliott Paris & Julien Colafrancesco.
// 2012-2015: Pierre Guillot & Eliott Paris.
// 2015: Pierre Guillot & Eliott Paris & Thomas Le Meur (Light version)
// 2016-2017: Pierre Guillot.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

#pragma once

#include "Response.hpp"

#include <limits>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <iostream>

namespace hoa::hrir_matrix_creator
{
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
        
        Subject(Config& config)
        : m_config(config)
        , m_processor(config.order)
        , m_folder(config.wave_folder)
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
            const auto filepath_base = m_config.output_directory;
            const auto filename_prefix = m_config.filename_prefix;;
            const auto dim_str = (Dim == Hoa2d) ? "2D" : "3D";
            const auto classname = m_config.classname + "_" + dim_str;
            const auto extension = m_config.file_extension;
            const auto filename = filepath_base + filename_prefix + classname + extension;
            
            std::ofstream file(filename);
            if(!file.is_open())
            {
                std::cerr << "[!] error - can't read " << filename << '\n';
                return;
            }
            
            // --- write data to file --- //
            
            const auto newline = "\n";
            const auto tab = "    ";
            
            file << get_cpp_file_header_text();
            
            if(!m_config.notes.empty())
            {
                file << newline << "/* Notes:\n";
                file << m_config.notes;
                file << newline << "*/" << newline;
            }
            
            const auto hoa_dim_str = (Dim == Hoa2d) ? "Dimension::Hoa2d" : "Dimension::Hoa3d";
            
            file << newline << "#pragma once" << newline << newline;
            
            file << "namespace hoa { namespace hrir " << newline << "{" << newline;
            
            file << tab << "struct " << classname << newline;
            file << tab << "{" << newline;
            file << tab << tab << "static const Dimension dimension = " << hoa_dim_str << ";\n";
            file << tab << tab << "static const size_t order = " << getDecompositionOrder() << ";\n";
            file << tab << tab << "static const size_t number_of_harmonics = " << getNumberOfHarmonics() << ";\n";
            file << tab << tab << "static const size_t responses_size = " << getResponsesSize() << ";\n";
            file << tab << tab << "static const size_t matrices_size = " << getMatricesSize() << ";\n";
            
            file << newline;
            
            writeData<float, BinauralSide::Left>(file, m_left);
            writeData<float, BinauralSide::Right>(file, m_right);
            writeData<double, BinauralSide::Left>(file, m_left);
            writeData<double, BinauralSide::Right>(file, m_right);
            
            file << tab << "};\n\n"; // end of struct
            
            file << "}}\n"; // end of hoa::hrir namespace
            
            file.close();
            
            std::cout << classname << " response written" << "\n";
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
            "// This file has been generated by https://github.com/CICM/HoaLibrary-Tools \n";
            
            return text;
        }
        
        template<typename FloatType>
        std::string convertFloatingPointNumberToString(FloatType value)
        {
            constexpr bool is_float = std::is_same<FloatType, float>::value;
            
            if(value == static_cast<FloatType>(0))
            {
                return is_float ? "0.f" : "0.";
            }
            
            std::stringstream ss;
            ss << std::setprecision(std::numeric_limits<FloatType>::max_digits10);
            ss << value;
            
            if constexpr (is_float)
            {
                ss << 'f';
            }
            
            return ss.str();
        }
        
        template<typename FloatType, BinauralSide Side>
        void writeData(std::ofstream& file, std::vector<double>& data)
        {
            file.precision(std::numeric_limits<FloatType>::digits10);
            
            const auto float_type_str = std::is_same<FloatType, float>::value ? "float" : "double";
            const auto side_str = Side == BinauralSide::Left ? "left" : "right";
            
            const auto tab = "    ";
            
            file << tab << tab << "static " << float_type_str << " const* get_" << float_type_str << "_" << side_str << "()\n";
            file << tab << tab << "{\n";
            
            file << tab << tab << tab << "static const " << float_type_str << " data[] = {";
            
            auto* f = data.data();
            for(size_t i = 0; i < data.size(); ++f, ++i)
            {
                // don't add comma for the last value
                const auto separator = (i < data.size() - 1) ? ", " : "";
                file << convertFloatingPointNumberToString(static_cast<FloatType>(*f)) << separator;
            }
            
            file << "};\n\n";
            
            file << tab << tab << tab << "return data;\n";
            file << tab << tab << "}\n\n";
        }
        
    private: // variables
        
        const Config            m_config;
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
            Response temp(file, m_config.database_type);
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
            Response temp(file, m_config.database_type);
            
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

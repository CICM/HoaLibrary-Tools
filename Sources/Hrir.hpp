/*
// Copyright (c) 2012-2015 Eliott Paris, Julien Colafrancesco, Thomas Le Meur & Pierre Guillot, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#ifndef DEF_HOA_HRIR_LIGHT
#define DEF_HOA_HRIR_LIGHT

#include "System.hpp"

using namespace std;

namespace hoa
{
    //! The response class owns the informations of an impulse response.
    /** The response class owns the informations of an impulse response.
     */
    template<typename T> class Response : public System::File::Wave
    {
    private:
        T          m_radius;
        T          m_azimuth;
        T          m_elevation;
        bool       m_valid;
    public:
        inline Response(System::File const& file) : System::File::Wave(file.getPath(), file.getName())
        {
            m_valid = false;
            if(System::File::Wave::isValid())
            {
                string name = getName();
                string::size_type pos = name.find("_T");
                if(pos != string::npos && pos < name.size() - 2)
                {
                    if(isdigit(name[pos+2]))
                    {
                        name.erase(name.begin(), name.begin()+pos+2);
                        m_azimuth = double(stol(name)) / 360. * HOA_2PI;
                        pos = name.find("_P");
                        if(pos != string::npos && pos < name.size() - 2)
                        {
                            if(isdigit(name[pos+2]))
                            {
                                name.erase(name.begin(), name.begin()+pos+2);
                                m_elevation = double(stol(name)) / 360. * HOA_2PI;
                                m_valid = true;
                            }
                            
                        }
                    }
                    
                }
            }
        }
        
        inline ~Response() noexcept {};
        inline T getRadius() const noexcept {return m_radius;}
        inline T getAzimuth() const noexcept {return m_azimuth;}
        inline T getElevation() const noexcept {return m_azimuth;}
        virtual inline bool isValid() const noexcept override {return m_valid;}
    };
    
    //! The subject owns the impulse response of a subject.
    /** The subject owns the impulse response of a subject.
     */
    template<Dimension D, typename T> class Subject : private Processor<D, T>::Harmonics
    {
    private:
        const System::Folder m_folder;
        vector<Response<T>>  m_responses;
        ulong                m_size;
        vector<T>            m_left;
        vector<T>            m_right;
    public:
        
        Subject(const ulong order, const System::Folder& folder) noexcept : Processor<D, T>::Harmonics(order),
        m_folder(folder), m_size(0ul)
        {
            vector<System::File> files(m_folder.getFiles(".wav"));
            for(auto it : files)
            {
                Response<T> temp(it);
                if(temp.isValid())
                {
                    m_responses.push_back(temp);
                    m_responses[m_responses.size()-1].read();
                    if(m_size < m_responses[m_responses.size()-1].getNumberOfSamplesPerChannel())
                    {
                        m_size = m_responses[m_responses.size()-1].getNumberOfSamplesPerChannel();
                    }
                }
            }
            m_left.resize(getResponsesSize() * getNumberOfHarmonics());
            fill(m_left.begin(), m_left.end(), T(0));
            m_right.resize(getResponsesSize() * getNumberOfHarmonics());
            fill(m_right.begin(), m_right.end(), T(0));
            Encoder<Hoa2d, float>::Basic encoder(getDecompositionOrder());
            for(ulong i = 0; i< getNumberOfResponses(); i++)
            {
                encoder.setAzimuth(m_responses[i].getAzimuth());
                for(ulong j = 0; j < getResponsesSize(); j++)
                {
                    const T left  = m_responses[i].getSample(0, j);
                    const T right = m_responses[i].getSample(1, j);
                    encoder.processAdd(&left, m_left.data()+j*getNumberOfHarmonics());
                    encoder.processAdd(&right, m_right.data()+j*getNumberOfHarmonics());
                }
            }
        }
        
        inline ~Subject() noexcept
        {
            m_responses.clear();
            m_left.clear();
            m_right.clear();
        }
        inline string getName() const noexcept {return m_folder.getName();};
        inline ulong getNumberOfResponses() const noexcept{return m_responses.size();}
        inline ulong getResponsesSize() const noexcept{return m_size;}
        inline ulong getDecompositionOrder() const noexcept {return Processor<D, T>::Harmonics::getDecompositionOrder();}
        inline ulong getNumberOfHarmonics() const noexcept {return Processor<D, T>::Harmonics::getNumberOfHarmonics();}
        inline ulong getMatricesSize() const noexcept {return getResponsesSize() * getNumberOfHarmonics();}
        
        void write(const string& name)
        {
            ofstream file("../Results/"+ getName() + ".hpp");
            if(file.is_open())
            {
                file << "/*\n// Copyright (c) 2012-2015 Eliott Paris, Julien Colafrancesco & Pierre Guillot, CICM, Universite Paris 8.\n// For information on usage and redistribution, and for a DISCLAIMER OF ALL\n// WARRANTIES, see the file, \"LICENSE.txt,\" in this distribution.\n*/\n\n";
                file << "#ifndef DEF_HOA_HRTF_" + m_folder.getName() + "_LIGHT\n";
                file << "#define DEF_HOA_HRTF_" + m_folder.getName() + "_LIGHT\n\n";
                file << "namespace hoa\n{\n";
                
                file.precision(numeric_limits<long double>::digits10);
                file << "    static const float " + m_folder.getName() + "_RIGHT[] = {";
                for(ulong i = 0; i < m_left.size() - 1; i++)
                {
                    file << m_left[i] << ", ";
                }
                file << m_left[m_left.size()-1] << "};\n";
                
                file << "    static const float " + m_folder.getName() + "_RIGHT[] = {";
                for(ulong i = 0; i < m_right.size() - 1; i++)
                {
                    file << m_right[i] << ", ";
                }
                file << m_right[m_right.size()-1] << "};\n";
                file <<"\n}\n#endif\n\n";
            }
            else
            {
                cout << "error";
            }
            
        }
    };
}

#endif

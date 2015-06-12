/*
// Copyright (c) 2012-2015 Eliott Paris, Julien Colafrancesco, Thomas Le Meur & Pierre Guillot, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#ifndef DEF_HOA_HRIR_LIGHT
#define DEF_HOA_HRIR_LIGHT

#include "System.hpp"
#include "../ThirdParty/LibSndFile/src/sndfile.hh"

using namespace std;

namespace hoa
{
    //! The response class owns the informations of an impulse response.
    /** The response class owns the informations of an impulse response.
     */
    class Response : public System::File
    {
    private:
        double  m_radius;
        double  m_azimuth;
        double  m_elevation;
        vector<double> m_values;
        ulong   m_size;
        bool    m_valid;
    public:
        inline Response(System::File const& file) : System::File(file), m_size(0), m_valid(false)
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
        
        void read()
        {
            SndfileHandle file(getFullName());
            if(file && file.channels() == 2)
            {
                try
                {
                    m_values.resize(file.channels() * file.frames());
                }
                catch(exception& e)
                {
                    cout << e.what();
                    m_values.clear();
                    return;
                }
                sf_count_t count = file.read(m_values.data(), file.channels() * file.frames());
                if(count != m_values.size())
                {
                    m_values.clear();
                    cout << count << "error \n";
                }
            }
            else
            {
                m_values.clear();
                cerr << "can't load wav file : " << getFullName() << "\n";
            }
        }
        
        inline ~Response() noexcept
        {
            m_values.clear();
        }
        inline double getRadius() const noexcept {return m_radius;}
        inline double getAzimuth() const noexcept {return m_azimuth;}
        inline double getElevation() const noexcept {return m_elevation;}
        inline ulong getNumberOfSamplesPerChannel() const noexcept {return m_values.size() / 2;}
        inline double getSample(ulong channel, ulong index) const noexcept
        {
            if(channel < 2 && index < getNumberOfSamplesPerChannel())
            {
                return m_values[index * 2 + channel];
            }
            return 0.;
        }
        inline bool isValid() const noexcept {return m_valid;}
    };
    
    //! The subject owns the impulse response of a subject.
    /** The subject owns the impulse response of a subject.
     */
    template<Dimension D> class Subject : private Processor<D, double>::Harmonics
    {
    private:
        const System::Folder m_folder;
        vector<Response>     m_responses;
        ulong                m_size;
        vector<double>       m_left;
        vector<double>       m_right;
    public:
        
        Subject(const ulong order, const System::Folder& folder) noexcept : Processor<D, double>::Harmonics(order),
        m_folder(folder), m_size(0ul)
        {
            ;
        }
        
        inline ~Subject() noexcept
        {
            m_responses.clear();
            m_left.clear();
            m_right.clear();
        }
        
        inline void read()
        {
            vector<System::File> files(m_folder.getFiles(".wav"));
            for(auto it : files)
            {
                Response temp(it);
                if(temp.isValid() && temp.getElevation() == 0)
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
            fill(m_left.begin(), m_left.end(), double(0));
            m_right.resize(getResponsesSize() * getNumberOfHarmonics());
            fill(m_right.begin(), m_right.end(), double(0));
            
            double* harmonics = new double[getNumberOfHarmonics()];
            if(harmonics)
            {
                Encoder<Hoa2d, double>::Basic encoder(getDecompositionOrder());
                for(ulong i = 0; i < getNumberOfResponses(); i++)
                {
                    const double azimuth = m_responses[i].getAzimuth();
                    encoder.setAzimuth(azimuth);
                    for(ulong j = 0; j < getResponsesSize(); j++)
                    {
                        const double left  = m_responses[i].getSample(0, j) / double(getDecompositionOrder() + 1.);
                        const double right = m_responses[i].getSample(1, j) / double(getDecompositionOrder() + 1.);
                        encoder.process(&left, harmonics);
                        harmonics[0] *= 0.5;
                        Signal<double>::add(getNumberOfHarmonics(), harmonics, m_left.data()+j*getNumberOfHarmonics());
                        encoder.process(&right, harmonics);
                        harmonics[0] *= 0.5;
                        Signal<double>::add(getNumberOfHarmonics(), harmonics, m_right.data()+j*getNumberOfHarmonics());
                    }
                }
                delete [] harmonics;
            }
            else
            {
                cout << "can't allocate memory";
            }
        }
        
        inline string getName() const noexcept {return m_folder.getName();};
        inline ulong getNumberOfResponses() const noexcept{return m_responses.size();}
        inline ulong getResponsesSize() const noexcept{return m_size;}
        inline ulong getDecompositionOrder() const noexcept{return Processor<D, double>::Harmonics::getDecompositionOrder();}
        inline ulong getNumberOfHarmonics() const noexcept {return Processor<D, double>::Harmonics::getNumberOfHarmonics();}
        inline ulong getMatricesSize() const noexcept {return getResponsesSize() * getNumberOfHarmonics();}
        
        void writeForPD()
        {
            for(ulong i = 0; i < getNumberOfHarmonics(); i++)
            {
                long order = Harmonic<Hoa2d, double>::getOrder(i);
                {
                    ofstream file("../Results/PD/ir"+ to_string(order) + "l.txt");
                    file.precision(numeric_limits<double>::digits10);
                    if(file.is_open())
                    {
                        for(ulong j = 0; j < getResponsesSize(); j++)
                        {
                            file << m_left[j*getNumberOfHarmonics()+i] << "\n";
                        }
                        file.close();
                    }
                    else
                    {
                        cout << "error";
                    }
                }
                {
                    ofstream file("../Results/PD/ir"+ to_string(order) + "r.txt");
                    file.precision(numeric_limits<double>::digits10);
                    if(file.is_open())
                    {
                        for(ulong j = 0; j < getResponsesSize(); j++)
                        {
                            file << m_right[j*getNumberOfHarmonics()+i] << "\n";
                        }
                        file.close();
                    }
                    else
                    {
                        cout << "error";
                    }
                }
                
            }
            
            
        }
        void writeForCPP()
        {
            string name = getName();
            size_t size = name.size();
            for(size_t i = 1; i < size; i++)
            {
                if(name[i] == '_')
                {
                    name.erase(name.begin() + i);
                    size--;
                }
                else
                {
                    name[i] = tolower(name[i]);
                }
            }
            ofstream file("../Results/Hrir"+ name + ".hpp");
            if(file.is_open())
            {
                file << "/*\n// Copyright (c) 2012-2015 Eliott Paris, Julien Colafrancesco & Pierre Guillot, CICM, Universite Paris 8.\n// For information on usage and redistribution, and for a DISCLAIMER OF ALL\n// WARRANTIES, see the file, \"LICENSE.txt,\" in this distribution.\n*/\n\n";
                file << "#ifndef DEF_HOA_HRIR_" + m_folder.getName() + "_LIGHT\n";
                file << "#define DEF_HOA_HRIR_" + m_folder.getName() + "_LIGHT\n\n";
                file << "// Order of Decompistion : " + to_string(getDecompositionOrder()) + "\n";
                file << "// Number of Harmonics   : " + to_string(getNumberOfHarmonics()) + "\n";
                file << "// Size of the Responses : " + to_string(getResponsesSize()) + "\n";
                file << "// Size of the matrics   : " + to_string(getMatricesSize()) + "\n\n";
                
                file << "namespace hoa\n{\n\n";
                file.precision(numeric_limits<double>::digits10);
                
                file << "\n    static const float " + name + "_left[] = {";
                for(ulong i = 0; i < m_left.size() - 1; i++)
                {
                    file << m_left[i] << ", ";
                }
                file << m_left[m_left.size()-1] << "};\n";
                
                file << "    static const float " + name + "_right[] = {";
                for(ulong i = 0; i < m_right.size() - 1; i++)
                {
                    file << m_right[i] << ", ";
                }
                file << m_right[m_right.size()-1] << "};\n";
                
                file <<"\n}\n#endif\n\n";
                file.close();
                cout << name << "\n";
            }
            else
            {
                cout << "error";
            }
            
        }
    };
}

#endif

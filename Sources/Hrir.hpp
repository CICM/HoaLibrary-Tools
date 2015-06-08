/*
// Copyright (c) 2012-2015 Eliott Paris, Julien Colafrancesco, Thomas Le Meur & Pierre Guillot, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#ifndef DEF_HOA_HRIR_LIGHT
#define DEF_HOA_HRIR_LIGHT

#include "Wave.hpp"

using namespace std;

namespace hoa
{
    //! The response class owns the informations of an impulse response.
    /** The response class owns the informations of an impulse response.
     */
    template<typename T> class Response
    {
    private:
        T          m_radius;
        T          m_azimuth;
        T          m_elevation;
        vector<T>  m_datas;
        
    public:
        inline Response(System::File file)
        {
            if(file.getType() == ".wav")
            {
                //WaveFile<T> wave;
                //wave.read(file);
                /*
                if(wave.isLoaded() && wave.getNumberOfChannels() == 2)
                {
                    wave.swap(m_datas);
                }*/
            }
        }
        
        inline Response(Response const& other) noexcept
        {
            m_radius = other.m_radius;
            m_azimuth = other.m_azimuth;
            m_elevation = other.m_elevation;
            m_datas = other.m_datas;
        }
        
        inline Response(Response&& other) noexcept
        {
            swap(m_radius, other.m_radius);
            swap(m_azimuth, other.m_azimuth);
            swap(m_elevation, other.m_elevation);
            m_datas.swap(other.m_datas);
        }
        
        inline ~Response() noexcept {m_datas.clear();};
        inline T getRadius() const noexcept {return m_radius;}
        inline T getAzimuth() const noexcept {return m_azimuth;}
        inline T getElevation() const noexcept {return m_azimuth;}
        inline ulong getSize() const noexcept {return m_datas.size() / 2;}
        inline T getSampleLeft(ulong const index) const noexcept {return (index < getSize()) ? m_datas[index * 2] : 0;}
        inline T getSampleRight(ulong const index) const noexcept {return (index < getSize()) ? m_datas[index * 2 + 1] : 0;}
        inline T* getSamplesLeft(ulong const index) noexcept {return (index < getSize()) ? m_datas.data()+index*2 : nullptr;}
        inline T* getSamplesRight(ulong const index) noexcept {return (index < getSize()) ? m_datas.data()+index*2+1 : nullptr;}
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
                m_responses.push_back(Response<T>(it));
                if(m_size < m_responses[m_responses.size()-1].getSize())
                {
                    m_size = m_responses[m_responses.size()-1].getSize();
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
                    encoder.processAdd(m_responses[i].getSamplesLeft(j), m_left.data()+j*getNumberOfHarmonics());
                    encoder.processAdd(m_responses[i].getSamplesLeft(j), m_right.data()+j*getNumberOfHarmonics());
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
    };
}

#endif

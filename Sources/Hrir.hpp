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
        inline Response(string const& file)
        {
            WaveFile<T> wave;
            wave.read(file);
            if(wave.isLoaded() && wave.getNumberOfChannels() == 2)
            {
                wave.swap(m_datas);
            }
            else
            {
                cout << file + " can't be loaded\n";
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
        inline T left(ulong const index) const noexcept {return index < getSize() ? m_datas[index * 2] : 0;}
        inline T right(ulong const index) const noexcept {return index < getSize() ? m_datas[index * 2 + 1] : 0;}
    };
    
    //! The subject owns the impulse response of a subject.
    /** The subject owns the impulse response of a subject.
     */
    template<typename T> class Subject
    {
    private:
        const Folder        m_folder;
        vector<Response<T>> m_responses;
        ulong               m_size;
    public:
        Subject(const Folder& folder) noexcept : m_folder(folder), m_size(0ul)
        {
            vector<string> waves(m_folder.getWaves());
            for(auto it : waves)
            {
                m_responses.push_back(Response<T>(it));
                if(m_size < m_responses[m_responses.size()-1].getSize())
                {
                    m_size = m_responses[m_responses.size()-1].getSize();
                }
            }
        }
        
        inline ~Subject() noexcept {m_responses.clear();}
        
        inline string getName() const noexcept {return m_folder.getName();};
        inline ulong getNumberOfResponses() const noexcept{return m_responses.size();}
        
        T* getHarmonicsMatrix() const noexcept
        {
            return nullptr;
        }
    };
}

#endif

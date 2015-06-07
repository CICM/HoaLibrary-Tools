/*
// Copyright (c) 2012-2015 Eliott Paris, Julien Colafrancesco, Thomas Le Meur & Pierre Guillot, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#ifndef DEF_HOA_WAVE_LIGHT
#define DEF_HOA_WAVE_LIGHT

#include "System.hpp"

namespace hoa
{
    //! The wave file class loads and stores the samples of a wave file.
    /** The wave file class loads and stores the samples of a wave file.
     */
    template<typename T> class WaveFile
    {
    private:
        char  m_chunk_id[4];
        ulong m_chunk_size;
        char  m_format[4];
        char  m_subchunk_id[4];
        ulong m_subchunk_size;
        short m_compressed;
        short m_nchannels;
        ulong m_sample_rate;
        ulong m_bytes_per_second;
        short m_bytes_per_samples;
        short m_bits_per_samples;
        char  m_subchunk_id2[4];
        ulong m_subchunk_size2;
        vector<T> m_datas;
        bool  m_loaded;
    public:
        inline WaveFile() noexcept : m_loaded(false)
        {
            
        }
        inline ~WaveFile() noexcept {m_datas.clear();}
        inline ulong getNumberOfChannels() const noexcept {return m_nchannels;}
        inline ulong getSampleRate() const noexcept {return m_sample_rate;}
        inline ulong getNumberOfSamples() const noexcept {return ulong(m_subchunk_size2/m_bytes_per_samples);}
        inline ulong getNumberOfSamplesPerChannel() const noexcept {return getNumberOfSamples()/getNumberOfChannels();}
        inline bool isCompressed() const noexcept {return m_compressed != 1;}
        inline bool isLoaded() const noexcept {return m_loaded;}
        inline void swap(vector<T>& datas) noexcept {datas.swap(m_datas); m_loaded = false;}
        inline void read(string const& path) noexcept
        {
            m_loaded = false;
            FILE* file;
            file = fopen(path.c_str(), "rb");
            if(file)
            {
                fread(&m_chunk_id, 4, 1, file);
                fread(&m_chunk_size, 4, 1, file);
                fread(&m_format, 4, 1, file);
                fread(&m_subchunk_id, 4, 1, file);
                fread(&m_subchunk_size, 4, 1, file);
                fread(&m_compressed, 2, 1, file);
                fread(&m_nchannels, 2, 1, file);
                fread(&m_sample_rate, 4, 1, file);
                fread(&m_bytes_per_second, 4, 1, file);
                fread(&m_bytes_per_samples, 2, 1, file);
                fread(&m_bits_per_samples, 2, 1, file);
                fread(&m_subchunk_id2, 4, 1, file);
                fread(&m_subchunk_size2, 4, 1, file);
                m_loaded = true;
                
                char* sample = new char[m_bytes_per_samples];
                m_datas.resize(getNumberOfSamples());
                for(ulong i = 0; i < getNumberOfSamples(); i++)
                {
                    fread(sample, m_bytes_per_samples, 1, file);
                    m_datas[i]  = T(sample[0]) / pow(2. ,15);
                }
                delete [] sample;
                m_loaded = true;
            }
        }
        
    };
}

#endif

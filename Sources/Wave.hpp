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
    class System::File::Wave : public System::File
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
        vector<double> m_datas;
        
        void read() noexcept
        {
            if(isValid())
            {
                FILE* file = getPtr();
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
                
                char* sample = new char[m_bytes_per_samples];
                m_datas.resize(getNumberOfSamples());
                for(ulong i = 0; i < getNumberOfSamples(); i++)
                {
                    fread(sample, m_bytes_per_samples, 1, file);
                    m_datas[i]  = double(sample[0]) / pow(2. ,15);
                }
                delete [] sample;
            }
        }
    public:
        inline Wave(const string& path, const string& name) noexcept : System::File(path, name, ".wav")
        {
            read();
        }
        inline ~Wave() noexcept {m_datas.clear();}
        inline ulong getNumberOfChannels() const noexcept {return m_nchannels;}
        inline ulong getSampleRate() const noexcept {return m_sample_rate;}
        inline ulong getNumberOfSamples() const noexcept {return ulong(m_subchunk_size2/m_bytes_per_samples);}
        inline ulong getNumberOfSamplesPerChannel() const noexcept {return getNumberOfSamples()/getNumberOfChannels();}
        inline bool isCompressed() const noexcept {return m_compressed != 1;}
    };
}

#endif

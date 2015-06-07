/*
// Copyright (c) 2012-2015 Eliott Paris, Julien Colafrancesco, Thomas Le Meur & Pierre Guillot, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#ifndef DEF_HOA_WAVE_LIGHT
#define DEF_HOA_WAVE_LIGHT

#include <iostream>
#include <dirent.h>
#include "../ThirdParty/HoaLibrary/Sources/Hoa.hpp"

using namespace std;

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
    
    class Folder
    {
    private:
        string m_name;
        string m_path;
        static inline bool isFolder(struct dirent const& ent) noexcept {return strchr(ent.d_name, '.') == NULL;}
        static inline bool isWave(struct dirent const& ent) noexcept {return strstr(ent.d_name, ".wav") != NULL;}
        
    public:
        inline Folder(const string& path, const string& name) noexcept
        {
#ifdef _WIN32
            m_path = path[path.size()-1] != '\\' ? path + "\\" : path;
            m_name = (!name.empty() && name[0] == '\\') ? string(name.c_str()+1) : name;
#else
            m_path = path[path.size()-1] != '/' ? path + "/" : path;
            m_name = (!name.empty() && name[0] == '/') ? string(name.c_str()+1) : name;
#endif
        }
        inline Folder(const Folder& other) noexcept
        {
            m_name = other.getName();
            m_path = other.getPath();
        }
        inline Folder(Folder&& other) noexcept
        {
            m_name.swap(other.m_name);
            m_path.swap(other.m_path);
        }
        inline string getName() const noexcept {return m_name;}
        inline string getPath() const noexcept {return m_path;}
        inline string getFullPath() const noexcept {return m_path + m_name;}

        inline vector<string> getWaveFiles() const noexcept
        {
            vector<string> files;
            DIR *dir;
            if((dir = opendir(getFullPath().c_str())) != NULL)
            {
                struct dirent *ent;
                while((ent = readdir(dir)) != NULL)
                {
                    if(isWave(*ent))
                    {
#ifdef _WIN32
                        files.push_back(getFullPath() + "\\" + ent->d_name);
#else
                        files.push_back(getFullPath() + "/" + ent->d_name);
#endif
                    }
                }
                closedir(dir);
            }
            return files;
        }
            
        static inline vector<Folder> get(string const& path) noexcept
        {
            vector<Folder> folders;
            DIR *dir;
            if((dir = opendir(path.c_str())) != NULL)
            {
                struct dirent *ent;
                while((ent = readdir(dir)) != NULL)
                {
                    if(isFolder(*ent))
                    {
                        folders.push_back(Folder(path, ent->d_name));
                    }
                }
                closedir(dir);
            }
            else
            {
                cout << "No such folder " + path + "\n";
            }
            return folders;
        }
    };
    
    static inline ostream& operator<<(ostream& os, Folder const& folder)
    {
        os << folder.getFullPath();
        return os;
    }
}

#endif

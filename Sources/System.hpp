/*
// Copyright (c) 2012-2015 Eliott Paris, Julien Colafrancesco, Thomas Le Meur & Pierre Guillot, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#ifndef DEF_HOA_SYSTEM_LIGHT
#define DEF_HOA_SYSTEM_LIGHT

#include <iostream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include "../ThirdParty/HoaLibrary/Sources/Hoa.hpp"

using namespace std;

namespace hoa
{
    class System
    {
    private:
#ifdef _WIN32
        static const char sepator = '\\';
#else
        static const char sepator = '/';
#endif
        static inline string formatName(const string& name) noexcept
        {
            string ntxt = name;
            string::size_type pos = ntxt.find_first_of('.');
            if(pos != string::npos)
            {
                ntxt.erase(ntxt.begin()+pos, ntxt.end());
            }
            pos = ntxt.find_last_of(sepator);
            if(pos != string::npos)
            {
                ntxt.erase(ntxt.begin(), ntxt.begin()+min(pos + 1, ntxt.size()));
            }
            return ntxt;
        }
        
        static inline string formatType(const string& type) noexcept
        {
            string ntxt = type;
            string::size_type pos = ntxt.find_last_of('.');
            if(pos != string::npos)
            {
                ntxt.erase(ntxt.begin(), ntxt.begin()+pos);
            }
            return ntxt;
        }
        
        static inline string formatPath(const string& path) noexcept
        {
            string ntxt = path;
            if(!path.empty() && path[path.size()-1] != sepator)
            {
                ntxt += sepator;
            }
            return ntxt;
        }
        
        static inline bool isType(const string& name, const string& type) noexcept
        {
            return type.empty() || formatType(name) == type;
        }
        
        static inline bool isFolder(const string& name) noexcept
        {
            return name.find('.') == string::npos;
        }
        
        static inline bool isValid(const string& name) noexcept
        {
            struct stat buffer;
            return (stat(name.c_str(), &buffer) == 0);
        }
        
    public:
        class File
        {
        private:
            string m_name;
            string m_type;
            string m_path;
        public:
            inline File(const string& path, const string& name, const string& type) noexcept :
            m_name(formatName(name)), m_path(formatPath(path)), m_type(formatType(type)) {}
            inline File(const File& other) noexcept :
            m_name(other.getName()), m_path(other.getPath()), m_type(other.getType()) {}
            inline File(File&& other) noexcept {
                m_name.swap(other.m_name); m_path.swap(other.m_path); m_type.swap(other.m_type);}
            virtual ~File() noexcept {}
            inline string getName() const noexcept {return m_name;}
            inline string getPath() const noexcept {return m_path;}
            inline string getType() const noexcept {return m_type;}
            inline string getFullName() const noexcept {return m_path + m_name + m_type;}
            virtual inline bool isValid() const noexcept {return System::isValid(getFullName());}
            static inline string getExtension() noexcept {return "";}
            class Wave;
        };
            
        class Folder
        {
        private:
            string m_name;
            string m_path;
        public:
            inline Folder(const string& path, const string& name) noexcept :
            m_name(formatName(name)), m_path(formatPath(path)) {}
            inline Folder(const Folder& other) noexcept : m_name(other.m_name), m_path(other.m_path) {}
            inline Folder(Folder&& other) noexcept {m_name.swap(other.m_name); m_path.swap(other.m_path);}
            inline ~Folder(){}
            inline string getName() const noexcept {return m_name;}
            inline string getPath() const noexcept {return m_path;}
            inline string getFullName() const noexcept {return m_path + m_name;}
            inline bool isValid() const noexcept {return System::isValid(getFullName());}
            inline vector<File> getFiles(const string& type) const noexcept
            {
                DIR* dir; vector<File> files;
                if((dir = opendir(getFullName().c_str())) != NULL)
                {
                    struct dirent *ent;
                    while((ent = readdir(dir)) != NULL)
                    {
                        if(!isFolder(ent->d_name) && isType(ent->d_name, type))
                        {
                            files.push_back(File(getFullName(), ent->d_name, type));
                        }
                    }
                    closedir(dir);
                }
                return files;
            }
        };
    
        static inline string getCurrentFolder() noexcept
        {
            char cwd[1024];
            if(getcwd(cwd, sizeof(cwd)) != NULL)
            {
               return cwd;
            }
            else
            {
                 return "";
            }
        }
        
        static inline vector<Folder> getFolders(string const& path) noexcept
        {
            vector<Folder> folders; DIR *dir;
            if((dir = opendir(path.c_str())) != NULL)
            {
                struct dirent *ent;
                while((ent = readdir(dir)) != NULL)
                {
                    if(isFolder(ent->d_name))
                    {
                        folders.push_back(Folder(path, ent->d_name));
                    }
                }
                closedir(dir);
            }
            else
            {
                //cerr << "No such folder " + path + "\n";
            }
            return folders;
        }
    };
                
    static inline ostream& operator<<(ostream& os, System::File const& file)
    {
        os << file.getFullName();
        return os;
    }
                            
    static inline ostream& operator<<(ostream& os, System::Folder const& folder)
    {
        os << folder.getFullName();
        return os;
    }
    
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
        bool m_valid;

    public:
        inline Wave(const string& path, const string& name) noexcept : System::File(path, name, ".wav"),
        m_nchannels(0),
        m_sample_rate(0),
        m_subchunk_size2(0),
        m_bytes_per_samples(0),
        m_compressed(0)
        {}
        inline ~Wave() noexcept {m_datas.clear();}
        inline ulong getNumberOfChannels() const noexcept {return m_nchannels;}
        inline ulong getSampleRate() const noexcept {return m_sample_rate;}
        inline ulong getNumberOfSamples() const noexcept
        {
            return (m_bytes_per_samples) ? ulong(m_subchunk_size2 / m_bytes_per_samples) : 0;
        }
        inline ulong getNumberOfSamplesPerChannel() const noexcept
        {
            return (getNumberOfChannels()) ? (getNumberOfSamples() / getNumberOfChannels()) : 0;
        }
        inline bool isCompressed() const noexcept {return m_compressed != 1;}
        inline double getSample(ulong const channel, ulong const index) const noexcept
        {
            return (index < getNumberOfSamplesPerChannel() && channel < getNumberOfChannels()) ? m_datas[index * getNumberOfChannels() + channel] : 0;
        }
        void read() noexcept
        {
            m_valid = false;
            FILE* file = fopen(getFullName().c_str(), "r");
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
                
                char* sample = new char[m_bytes_per_samples];
                m_datas.resize(getNumberOfSamples());
                for(ulong i = 0; i < getNumberOfSamples(); i++)
                {
                    fread(sample, m_bytes_per_samples, 1, file);
                    m_datas[i]  = double(sample[0]) / pow(2. ,15);
                }
                delete [] sample;
                m_valid = true;
            }
        }
    };
                
}

#endif

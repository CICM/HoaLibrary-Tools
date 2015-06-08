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
        
    public:
        class File
        {
        private:
            FILE*  m_file;
            string m_name;
            string m_type;
            string m_path;
        protected:
            inline FILE* getPtr() const noexcept {return m_file;};
        public:
            inline File(const string& path, const string& name, const string& type) noexcept :
            m_name(formatName(name)), m_path(formatPath(path)), m_type(formatType(type))
            {
                m_file = fopen(getFullName().c_str(), "r");
            }
            inline File(const File& other) noexcept :
            m_name(other.getName()), m_path(other.getPath()), m_type(other.getType())
            {
                m_file = fopen(getFullName().c_str(), "r");
            }
            inline File(File&& other) noexcept : m_file(nullptr)
            {
                m_name.swap(other.m_name); m_path.swap(other.m_path); m_type.swap(other.m_type), swap(m_file, other.m_file);
            }
            virtual ~File() noexcept {}
            inline string getName() const noexcept {return m_name;}
            inline string getPath() const noexcept {return m_path;}
            inline string getType() const noexcept {return m_type;}
            inline string getFullName() const noexcept {return m_path + m_name + m_type;}
            virtual inline bool isValid() const noexcept {return bool(m_file);}
            static inline string getExtension() noexcept {return "";}
            class Wave;
        };
            
        class Folder
        {
        private:
            DIR*   m_dir;
            string m_name;
            string m_path;
        public:
            inline Folder(const string& path, const string& name) noexcept :
            m_name(formatName(name)), m_path(formatPath(path))
            {
                m_dir = opendir(getFullName().c_str());
            }
            inline Folder(const Folder& other) noexcept :
            m_name(other.getName()), m_path(other.getPath())
            {
                m_dir = opendir(getFullName().c_str());
            }
            inline Folder(Folder&& other) noexcept : m_dir(nullptr)
            {
                m_name.swap(other.m_name); m_path.swap(other.m_path); swap(m_dir, other.m_dir);
            }
            inline ~Folder(){if(m_dir){closedir(m_dir);};}
            inline string getName() const noexcept {return m_name;}
            inline string getPath() const noexcept {return m_path;}
            inline string getFullName() const noexcept {return m_path + m_name;}
            inline bool isValid() const noexcept {return bool(m_dir);}
            inline vector<File> getFiles(const string& type) const noexcept
            {
                vector<File> files;
                if(isValid())
                {
                    struct dirent *ent;
                    while((ent = readdir(m_dir)) != NULL)
                    {
                        if(!isFolder(ent->d_name) && isType(ent->d_name, type))
                        {
                            files.push_back(File(getFullName(), ent->d_name, type));
                        }
                    }
                    rewinddir(m_dir);
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
                cerr << "No such folder " + path + "\n";
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
}

#endif

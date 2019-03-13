// Copyright (c) 2012-2019 CICM - Universite Paris 8 - Labex Arts H2H.
// Authors :
// 2012: Pierre Guillot, Eliott Paris & Julien Colafrancesco.
// 2012-2015: Pierre Guillot & Eliott Paris.
// 2015: Pierre Guillot & Eliott Paris & Thomas Le Meur (Light version)
// 2016-2017: Pierre Guillot.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

#pragma once

#include <iostream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace hoa
{
    class System
    {
    private:
#ifdef _WIN32
        static const char separator = '\\';
#else
        static const char separator = '/';
#endif
        static inline std::string formatName(std::string const& name)
        {
            std::string ntxt = name;
            auto pos = ntxt.find_first_of('.');
            if(pos != std::string::npos)
            {
                ntxt.erase(ntxt.begin()+long(pos), ntxt.end());
            }
            pos = ntxt.find_last_of(separator);
            if(pos != std::string::npos)
            {
                ntxt.erase(ntxt.begin(), ntxt.begin()+long(std::min(pos + 1, ntxt.size())));
            }
            return ntxt;
        }
        
        static inline std::string formatType(std::string const& type)
        {
            std::string ntxt = type;
            auto pos = ntxt.find_last_of('.');
            if(pos != std::string::npos)
            {
                ntxt.erase(ntxt.begin(), ntxt.begin()+long(pos));
            }
            return ntxt;
        }
        
        static inline std::string formatPath(std::string const& path)
        {
            std::string ntxt = path;
            if(!path.empty() && path[path.size()-1] != separator)
            {
                ntxt += separator;
            }
            return ntxt;
        }
        
        static inline bool isType(std::string const& name, std::string const& type) noexcept
        {
            return type.empty() || formatType(name) == type;
        }
        
        static inline bool isFolder(std::string const& name) noexcept
        {
            return name.find('.') == std::string::npos;
        }
        
        static inline bool isValid(std::string const& name) noexcept
        {
            struct stat buffer;
            return (stat(name.c_str(), &buffer) == 0);
        }
        
    public:
        
        class File
        {
        public:
            
            File(std::string const& path,
                 std::string const& name,
                 std::string const& type)
            : m_name(formatName(name))
            , m_path(formatPath(path))
            , m_type(formatType(type))
            {}
            
            inline File(const File& other)
            : m_name(other.getName())
            , m_path(other.getPath())
            , m_type(other.getType())
            {}
            
            inline File(File&& other) noexcept
            {
                m_name.swap(other.m_name);
                m_path.swap(other.m_path);
                m_type.swap(other.m_type);
            }
            
            virtual ~File() = default;
            inline std::string getName() const {return m_name;}
            inline std::string getPath() const {return m_path;}
            inline std::string getType() const {return m_type;}
            inline std::string getFullName() const {return m_path + m_name + m_type;}
            virtual bool isValid() const {return System::isValid(getFullName());}
            static std::string getExtension() {return "";}
            
        private:
            
            std::string m_name {};
            std::string m_type {};
            std::string m_path {};
        };
            
        class Folder
        {
        public:
            
            Folder() = default;
            
            Folder(std::string const& path,
                   std::string const& name)
            : m_name(formatName(name))
            , m_path(formatPath(path))
            {}
            
            Folder(Folder const& other)
            : m_name(other.m_name)
            , m_path(other.m_path)
            {}
            
            Folder(Folder&& other)
            {
                m_name.swap(other.m_name);
                m_path.swap(other.m_path);
            }
            
            Folder& operator=(Folder other)
            {
                std::swap(m_name, other.m_name);
                std::swap(m_path, other.m_path);
                return *this;
            }
            
            ~Folder() = default;
            
            std::string const& getName() const noexcept {return m_name;}
            std::string const& getPath() const noexcept {return m_path;}
            std::string getFullName() const {return m_path + m_name;}
            
            bool isValid() const noexcept
            {
                return (!m_name.empty() && !m_path.empty()
                        && System::isValid(getFullName()));
            }
            
            std::vector<File> getFiles(const std::string& type) const
            {
                DIR* dir;
                std::vector<File> files;
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
            
        private:
            
            std::string m_name {};
            std::string m_path {};
        };
    
        static inline std::string getCurrentFolder() noexcept
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
        
        static inline std::vector<Folder> getFolders(std::string const& path) noexcept
        {
            DIR *dir;
            std::vector<Folder> folders;
            if((dir = opendir(path.c_str())) != NULL)
            {
                struct dirent *ent;
                while((ent = readdir(dir)) != NULL)
                {
                    if(isFolder(ent->d_name))
                    {
                        folders.emplace_back(path, ent->d_name);
                    }
                }
                
                closedir(dir);
            }
            else
            {
                std::cerr << "No such folder " + path + "\n";
            }
            return folders;
        }
    };
                
    static inline std::ostream& operator<<(std::ostream& os, System::File const& file)
    {
        os << file.getFullName();
        return os;
    }
                            
    static inline std::ostream& operator<<(std::ostream& os, System::Folder const& folder)
    {
        os << folder.getFullName();
        return os;
    }
                
}

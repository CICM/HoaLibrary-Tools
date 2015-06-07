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
    class Response
    {
    private:
        double          m_azimuth;
        double          m_elevation;
        vector<double>  m_left;
        vector<double>  m_right;
        
    public:
        inline Response(string const& file) {};
        inline Response(Response const& other)
        {
            ;
        };
        
        inline Response(Response&& other)
        {
            ;
        };
        
        inline ~Response() noexcept {m_left.clear(); m_right.clear();};
    };
    
    //! The subject owns the impulse response of a subject.
    /** The subject owns the impulse response of a subject.
     */
    class Subject
    {
    private:
        const Folder     m_folder;
        vector<Response> m_responses;
    public:
        Subject(const Folder& folder) noexcept : m_folder(folder)
        {
            vector<string> waves(m_folder.getWaves());
            for(auto it : waves)
            {
                m_responses.push_back(Response(it));
            }
        }
        inline ~Subject() noexcept {m_responses.clear();}
        inline string getName() const noexcept {return m_folder.getName();};
    };
}

#endif

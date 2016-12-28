#ifndef _SETTINGS_H
#define _SETTINGS_H

#pragma once

struct ProjectSettings
{
    wxString m_author;
    wxString m_copyright;
};

struct Settings
{
    ProjectSettings project;
    bool isProjectLoaded;
};

#endif
// -*- c-basic-offset: 4 -*-

/** @file huginApp.cpp
 *
 *  @brief implementation of huginApp Class
 *
 *  @author Pablo d'Angelo <pablo.dangelo@web.de>
 *
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <wx/wxprec.h>
#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// For all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWindows headers)
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/config.h>              // wx config classes for all systems
#include "wx/image.h"               // wxImage
#include "wx/xrc/xmlres.h"          // XRC XML resouces

#include "hugin/MainFrame.h"

#include "hugin/huginApp.h"

// make wxwindows use this class as the main application
IMPLEMENT_APP(huginApp)

huginApp::huginApp()
{
}

huginApp::~huginApp()
{
}


bool huginApp::OnInit()
{
    printf( "%s\n", GetAppName().c_str());

    // here goes and comes configuration
    wxConfigBase *config = new wxConfig ( GetAppName().c_str(),
			"hugin Team", ".huginrc", "huginrc",
			 wxCONFIG_USE_LOCAL_FILE );
    // set as global config, so that other parts of hugin and wxWindows
    // controls can use it easily
    wxConfigBase::Set(config);
    config->SetRecordDefaults(TRUE);
    printf( "GetPath(): %s\n",config->GetPath().c_str());
    printf( "GetVendorName(): %s\n",config->GetVendorName().c_str() );
    config->SetRecordDefaults(TRUE);
    
    printf ("entries in config:  %i\n", config->GetNumberOfEntries() );
    if ( config->IsRecordingDefaults() )
      printf ("writes in config:  %i\n", config->GetNumberOfEntries() );
    config->Flush();

    // initialize i18n
    locale.Init(wxLANGUAGE_DEFAULT);

    // add local Path
    locale.AddCatalogLookupPathPrefix(wxT("po"));
    // add path from config file
    if (config->HasEntry(wxT("locale_path"))){
        locale.AddCatalogLookupPathPrefix(  config->Read("locale_path").c_str() );
    }

    // set the name of locale recource to look for
    locale.AddCatalog(wxT("hugin"));

    // initialize image handlers
    wxInitAllImageHandlers();

    // Initialize all the XRC handlers.
    wxXmlResource::Get()->InitAllHandlers();

    // load all XRC files.
#ifdef _INCLUDE_UI_RESOURCES
    InitXmlResource();
#else
    // try local xrc files first
    wxString xrcPrefix;
    wxFile testfile("xrc/main_frame.xrc");
    if (testfile.IsOpened()) {
        std::cerr << "using local xrc files" << std::endl;
        xrcPrefix = "";
    } else {
        std::cerr << "using xrc prefix from config" << std::endl;
        xrcPrefix = config->Read("xrc_path") + wxT("/");
    }
    wxXmlResource::Get()->Load(xrcPrefix + wxT("xrc/main_frame.xrc"));
//    wxXmlResource::Get()->Load(wxT("xrc/main_menubar.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("xrc/cp_editor_panel.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("xrc/main_menu.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("xrc/main_tool.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("xrc/edit_text.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("xrc/about.xrc"));
#endif

    // create main frame
    MainFrame *frame = new MainFrame();

    // show the frame.
    frame->Show(TRUE);

    return true;
}


// -*- c-basic-offset: 4 -*-

/** @file Batch.h
 *
 *  @brief Batch processor for Hugin
 *
 *  @author Marko Kuder <marko.kuder@gmail.com>
 *
 *  $Id: Batch.h 3322 2008-08-18 1:10:07Z mkuder $
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

#ifndef BATCH_H
#define BATCH_H

#include <string>
#include "ProjectArray.h"
#include <wx/dir.h>
#ifndef __WXMSW__
	#include <sys/wait.h>
#endif
//#include <wx/wfstream.h>
//#include <wx/event.h>
#include "RunStitchFrame.h"
//#include <hugin_config.h>

#ifndef FRAMEARRAY
#define FRAMEARRAY
	WX_DEFINE_ARRAY_PTR(RunStitchFrame*,FrameArray);
#endif

using namespace std;

class Batch : public wxFrame
{
public:
	bool parallel;
	bool deleteFiles;
	bool shutdown;
	bool overwrite;
	bool verbose;
	bool gui;

	//Main constructor
	Batch(wxFrame* parent, wxString path, bool bgui);

	//Adds an application entry in the batch list
	void  AddAppToBatch(wxString app);
	//Adds a project entry in the batch list
	void  AddProjectToBatch(wxString projectFile, wxString outputFile = _T(""));	
	//Returns true if there are no more projects pending execution
	bool  AllDone();
	//Appends projects from file to batch list
	void  AppendBatchFile(wxString file);
	//Stops batch run, failing projects in progress
	void  CancelBatch();
	//Cancels project at index in batch, failing it
	void  CancelProject(int index);
	//Changes output prefix for project at index
	void  ChangePrefix(int index, wxString newPrefix);
	//Called internally in console mode. Waits for running projects to complete and then runs next in batch
	bool  CheckProjectExistence();
	//Clears batch list and returns 0 if succesful
	int   ClearBatch();
	//Compares two project at indexes in both lists and returns true if they have identical project ids
	bool  CompareProjectsInLists(int stitchListIndex, int batchListIndex);
	//Returns index of first waiting project in batch
	int   GetFirstAvailable();
	//Returns index of project with selected id
	int   GetIndex(int id);
	//Returns project at index
	Project* GetProject(int index);
	//Returns number of projects in batch list
	int   GetProjectCount();
	//Returns number of projects in batch list with the input file path
	int   GetProjectCountByPath(wxString path);
	//Returns number of projects currently in progress
	int   GetRunningCount();
	//Returns current status of project at index
	Project::Status GetStatus(int index);
	//Returns true if batch execution is currently paused
	bool  IsPaused();
	//Used in console mode. Prints out all projects and their statuses to the console
	void  ListBatch();
	//Clears current batch list and loads projects from batch file
	int  LoadBatchFile(wxString file);
	//Loads temporary batch file
	int   LoadTemp();
	//Returns true if there are no failed projects in batch
	bool  NoErrors();
	//Called internally when all running processes have completed and need to be removed from running list
	void  OnProcessTerminate(wxProcessEvent & event);
	//Called to start stitch of project with input scriptFile
	bool  OnStitch(wxString scriptFile, wxString outname, int id);
	//Pauses and continues batch execution
	void  PauseBatch();
	//Removes project with id from batch list
	void  RemoveProject(int id);
	//Removes project at index from batch list
	void  RemoveProjectAtIndex(int selIndex);
	//Starts batch execution
	void  RunBatch();
	//Starts execution of next waiting project in batch
	void  RunNextInBatch();
	//Saves batch list to file
	void  SaveBatchFile(wxString file);
	//Saves batch list to temporary file
	void  SaveTemp();
	//Used internally to set status of selected project
	void  SetStatus(int index,Project::Status status);
	//Swaps position in batch of project at index with project at index+1
	void  SwapProject(int index);	

private:
	//environment config objects
	wxConfigBase* m_config;
	wxLocale      m_locale;
	//internal list of projects in batch
	ProjectArray  m_projList;
	//list of projects in progress
	FrameArray    m_stitchFrames;
	
	//batch state flags
	bool m_cancelled;
	bool m_paused;
	bool m_running;
	bool m_clearedInProgress;
	
	//external program config
	PTPrograms progs;

	DECLARE_EVENT_TABLE()
};

#endif //BATCH_H
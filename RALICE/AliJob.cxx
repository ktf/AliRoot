/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

// $Id$

///////////////////////////////////////////////////////////////////////////
// Class AliJob
// Base class for the top level processor class in a task based procedure.
// It allows stepwise invokation of various sub-tasks by the derived
// (user defined) top level processor, based on looping over a certain
// main object structure (e.g. AliEvent for event-by-event processing).
// The main object structure (if needed) may be specified by the derived
// top level processor class and will be stored automatically in the
// working environment (see below).  
// This base class provides a working environment for the derived
// (user defined) processor class and all of its subtasks.
//
// The working environment consists of (a.o.) :
// * A folder which may serve as a whiteboard for transferring pointers to
//   objects which are posted there by the top level processor or any
//   of its subtasks.
//   Objects can be posted in the job folder via the AddObject() facility
//   of this AliJob class.
//   Access to the job folder is obtained via the GetFolder() memberfunction
//   and from this the various objects can be accessed via the usual TFolder
//   FindObject and/or iterator facilities.
// * An array containing pointers to the objects which are stored
//   via the AddObject() facility of this AliJob class.
//   From this storage the objects can be (more directly) accessed via the
//   GetObject() and GetObjects() memberfunctions.
// * A pointer to the main object structure during job processing.
//   This pointer can be initiated/updated only by the derived top level
//   processor via the SetMainObject() facility but all sub-tasks can access
//   it via the above folder/array facilities or the GetMainObject() memberfunction.
//   The latter provides faster access to the main object structure than
//   the GetObject() or TFolder search based procedures.
//
// Notes :
// -------
// 1) This AliJob class is derived from TTask, which implies that every
//    (user defined) top level processor class is itself also a TTask.
//    This allows sub-tasks to be introduced to the top level processor
//    using the standard TTask facilities.
//
// 2) Only references to the various introduced objects are stored.
//    It is the user's responsibility to delete all introduced objects,
//    either in the destructor of the derived top level processor class
//    or via Clear() facility as provided by the TTask machinery.
//
// 3) The top level processor instance is entered into the standard ROOT
//    ListOfTasks under the name which was provided by the user in the
//    constructor of the top level processor.
//    The name of the top level processor is passed automatically as the
//    opt argument to the Exec(Option_t* opt) memberfunctions of the 
//    various sub-tasks by the ExecuteJob() memberfunction (see below).
//    This allows all sub-tasks to obtain the pointer to the top level
//    processor instance from its name via the statement :
//
//      AliJob* parent=(AliJob*)gROOT->GetListOfTasks()->FindObject(opt)
//
// 4) The job-specific folder will be created in the generic folder called
//    "AliJob-folders" as a sub-folder under the same name as the one
//    introduced in the constructor of the derived top level processor class.
//    The folder will only be created when the first object is posted via
//    the AddObject() or SetMainObject() facilities.
//
// Execution of the (user defined) top level processor has to be invoked via
// the memberfunction ExecuteJob() of this AliJob base class.
// This will invoke the (user written) Exec() memberfunction of the top level
// processor class with as argument the name of the top level processor instance
// as specified by the user in the top level processor constructor.
// This will allow stepwise (e.g. event-by-event) execution of the various sub-tasks.
//
// It is the user's responsibility to invoke the sub-tasks via the
// ExecuteTasks() statement at the appropriate location in the top level
// processor class. 
//
//--- Author: Nick van Eijndhoven 07-may-2005 Utrecht University
//- Modified: NvE $Date$ Utrecht University
///////////////////////////////////////////////////////////////////////////
 
#include "AliJob.h"
#include "Riostream.h"

ClassImp(AliJob) // Class implementation to enable ROOT I/O

AliJob::AliJob(const char* name,const char* title) : TTask(name,title)
{
// Default constructor.
// Initialise the working environment for general data access
// by the derived task and its subtasks.

 fMainObject=0;
 fFolder=0;
 fObjects=0;
 fSelect=0;

 // Introduce this AliJob based instance into the ROOT task list
 TSeqCollection* tasks=gROOT->GetListOfTasks();
 if (tasks) tasks->Add(this);
}
///////////////////////////////////////////////////////////////////////////
AliJob::~AliJob()
{
// Default destructor.
// Note : The objects belonging to the various pointers in the folder
//        and the main processing object are NOT deleted by this base class.

 if (fObjects)
 {
  delete fObjects;
  fObjects=0;
 }
 if (fFolder)
 {
  delete fFolder;
  fFolder=0;
 }
 if (fSelect)
 {
  delete fSelect;
  fSelect=0;
 }
}
///////////////////////////////////////////////////////////////////////////
void AliJob::ListEnvironment()
{
// Provide listing of the job environment. 

 cout << " ***" << endl;
 cout << " *** Environment of job " << GetName() << " ***" << endl;
 cout << " ***" << endl;
 cout << " === Available (sub)tasks : " << endl;
 ls();
 cout << " === Current job-folder contents : " << endl;
 fFolder->ls();
 cout << endl;
}
///////////////////////////////////////////////////////////////////////////
void AliJob::ExecuteJob()
{
// Invokation of the top level processor via its Exec() memberfunction.
 Exec(GetName());
}
///////////////////////////////////////////////////////////////////////////
TFolder* AliJob::GetFolder() const
{
// Provide pointer to the whiteboard folder.
 return fFolder;
}
///////////////////////////////////////////////////////////////////////////
TObject* AliJob::GetMainObject() const
{
// Provide pointer to the main object structure.
 return fMainObject;
}
///////////////////////////////////////////////////////////////////////////
void AliJob::SetMainObject(TObject* obj)
{
// Store pointer to the main object structure.
 if (obj)
 {
  fMainObject=obj;
  AddObject(obj);
 }
}
///////////////////////////////////////////////////////////////////////////
void AliJob::AddObject(TObject* obj)
{
// Store pointer of specified object in the working environment.

 if (!obj) return;

 if (!fObjects) fObjects=new TObjArray();

 if (!fFolder)
 {
  // Create the top level environment folder for all AliJobs if needed 
  TList* list=gROOT->GetListOfBrowsables();
  if (list)
  {
   TFolder* top=(TFolder*)list->FindObject("AliJob-folders");
   if (!top)
   {
    top=new TFolder("AliJob-folders","Environment for all AliJob derived tasks");
    list->Add(top,"AliJob-folders");
   }
   // Create the task-specific folder as a sub-folder in the top folder 
   fFolder=top->AddFolder(GetName(),GetTitle());
  }
 }

 // Add object pointer to array and folder if it doesn't already exist 
 Int_t exist=0;
 for (Int_t i=0; i<fObjects->GetEntries(); i++)
 {
  if (obj==fObjects->At(i))
  {
   exist=1;
   break;
  }
 }
 if (!exist)
 {
  fObjects->Add(obj);
  fFolder->Add(obj);
 }
}
///////////////////////////////////////////////////////////////////////////
void AliJob::AddObjects(TObjArray* arr)
{
// Store pointers of all the array objects individually in the working environment.

 if (!arr) return;

 TObject* obj=0;
 for (Int_t i=0; i<arr->GetSize(); i++)
 {
  obj=arr->At(i);
  if (obj) AddObject(obj);
 }
}
///////////////////////////////////////////////////////////////////////////
void AliJob::RemoveObject(TObject* obj)
{
// Remove pointer of specified object from the working environment.

 if (!obj) return;

 if (fObjects)
 {
  TObject* test=fObjects->Remove(obj);
  if (test) fObjects->Compress();
 }

 if (fFolder) fFolder->Remove(obj);
}
///////////////////////////////////////////////////////////////////////////
void AliJob::RemoveObjects(const char* classname)
{
// Remove all stored objects inheriting from classname.

 if (!fObjects) return;

 Int_t remove=0;
 for (Int_t i=0; i<fObjects->GetEntries(); i++)
 {
  TObject* obj=fObjects->At(i);
  if (obj->InheritsFrom(classname))
  {
   TObject* test=fObjects->Remove(obj);
   if (test) remove=1;
   if (fFolder) fFolder->Remove(obj);
  }
 }
 if (remove) fObjects->Compress();
}
///////////////////////////////////////////////////////////////////////////
TObject* AliJob::GetObject(Int_t j) const
{
// Provide pointer to j-th stored object.
// Note : j=1 indicates the first object.

 if (!fObjects || j<1) return 0;

 TObject* obj=0;

 if (j<=fObjects->GetEntries()) obj=fObjects->At(j-1);
 return obj;
}
///////////////////////////////////////////////////////////////////////////
TObject* AliJob::GetObject(const char* classname) const
{
// Provide pointer to the first stored object which inherits from classname.

 if (!fObjects) return 0;

 TObject* obj=0;
 for (Int_t i=0; i<fObjects->GetEntries(); i++)
 {
  TObject* obx=fObjects->At(i);
  if (obx->InheritsFrom(classname))
  {
   obj=obx;
   break;
  }
 }
 return obj;
}
///////////////////////////////////////////////////////////////////////////
TObjArray* AliJob::GetObjects() const
{
// Provide pointers of all the stored objects.
 return fObjects;
}
///////////////////////////////////////////////////////////////////////////
TObjArray* AliJob::GetObjects(const char* classname)
{
// Provide pointers to all stored objects inheriting from classname.

 if (!fObjects) return 0;

 if (fSelect)
 {
  fSelect->Clear();
 }
 else
 {
  fSelect=new TObjArray();
 }

 for (Int_t i=0; i<fObjects->GetEntries(); i++)
 {
  TObject* obj=fObjects->At(i);
  if (obj->InheritsFrom(classname)) fSelect->Add(obj);
 }
 return fSelect;
}
///////////////////////////////////////////////////////////////////////////

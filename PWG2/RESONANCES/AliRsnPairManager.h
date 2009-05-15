//
// Class AliRsnPairManager
//
// A collection of pairs for an analysis.
// The function of this collection is just for purposes of well-sorting
// the analyzed pairs into upper-level groups, in the case of a wide
// analysis containing many resonances at once, or different settings for the same one.
//
// Each PairManager will result in a separate list of histograms, which
// can be seen as a folder in the output file, named after this object.
//
// This object inherits from AliRsnVManager, and "forces" the type of the
// stored objects to be of the "AliRsnPair" class.
// No further data members are added.
//
// author: M. Vala (email: martin.vala@cern.ch)
//

#ifndef AliRsnPairManager_H
#define AliRsnPairManager_H

#include <TList.h>

#include "AliRsnVManager.h"
#include "AliRsnPair.h"

class AliRsnPairManager : public AliRsnVManager
{
  public:

    AliRsnPairManager(const char *name = "defaultPairMgr");

    //virtual void   Add(AliRsnPair *pair);
    virtual void   Add(TObject *pair);
    virtual void   AddPair(AliRsnPair *pair) {Add(pair);}
    virtual void   PrintArray() const;
    virtual void   Print(Option_t *option = "") const;

            TList* InitAllPairs();
            void   ProcessAllPairs(AliRsnPIDIndex *pidIndexes1, AliRsnEvent *ev1, AliRsnPIDIndex *pidIndexes2 = 0, AliRsnEvent *ev2 = 0);

  private:

    ClassDef(AliRsnPairManager, 1)
};

#endif

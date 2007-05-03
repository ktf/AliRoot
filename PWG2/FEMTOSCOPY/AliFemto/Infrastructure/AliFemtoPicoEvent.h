/***************************************************************************
 *
 * $Id$
 *
 * Author: Mike Lisa, Ohio State, lisa@mps.ohio-state.edu
 ***************************************************************************
 *
 * Description: part of STAR HBT Framework: AliFemtoMaker package
 *  PicoEvents are last-step ultra-compressed "events" just containing
 *  bare information about the particles of interest.  They have already
 *  gone through Event and Track cuts, so only Pair cuts are left.
 *  PicoEvents are *internal* to the code, and are stored in the
 *  Event-mixing buffers.
 *           
 *
 ***************************************************************************
 *
 * $Log$
 * Revision 1.1.1.1  2007/04/25 15:38:41  panos
 * Importing the HBT code dir
 *
 * Revision 1.1.1.1  2007/03/07 10:14:49  mchojnacki
 * First version on CVS
 *
 * Revision 1.2  2000/03/17 17:23:05  laue
 * Roberts new three particle correlations implemented.
 *
 * Revision 1.1.1.1  1999/06/29 16:02:57  lisa
 * Installation of AliFemtoMaker
 *
 **************************************************************************/

#ifndef AliFemtoPicoEvent_hh
#define AliFemtoPicoEvent_hh

#include "Infrastructure/AliFemtoParticleCollection.h"

class AliFemtoPicoEvent{
public:
  AliFemtoPicoEvent();
  AliFemtoPicoEvent(const AliFemtoPicoEvent& aPicoEvent);
  ~AliFemtoPicoEvent();

  AliFemtoPicoEvent& operator=(AliFemtoPicoEvent& aPicoEvent);

  /* may want to have other stuff in here, like where is primary vertex */

  AliFemtoParticleCollection* FirstParticleCollection();
  AliFemtoParticleCollection* SecondParticleCollection();
  AliFemtoParticleCollection* ThirdParticleCollection();

private:
  AliFemtoParticleCollection* fFirstParticleCollection;
  AliFemtoParticleCollection* fSecondParticleCollection;
  AliFemtoParticleCollection* fThirdParticleCollection;
};

inline AliFemtoParticleCollection* AliFemtoPicoEvent::FirstParticleCollection(){return fFirstParticleCollection;}
inline AliFemtoParticleCollection* AliFemtoPicoEvent::SecondParticleCollection(){return fSecondParticleCollection;}
inline AliFemtoParticleCollection* AliFemtoPicoEvent::ThirdParticleCollection(){return fThirdParticleCollection;}

#endif

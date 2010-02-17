/**************************************************************************
 * This file is property of and copyright by the Experimental Nuclear     *
 * Physics Group, Dep. of Physics                                         *
 * University of Oslo, Norway, 2007                                       *
 *                                                                        *
 * Author: Per Thomas Hille <perthomas.hille@yale.edu>                    *
 * for the ALICE HLT Project.                                             * 
 * Contributors are mentioned in the code where appropriate.              *
 * Please report bugs to perthi@fys.uio.no                                *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

// Evaluation of peak position
// and amplitude using Neural Networks (NN)
// ------------------
// ------------------
// ------------------


#include "AliCaloRawAnalyzerNN.h"
#include "AliCaloNeuralFit.h"
#include "AliCaloFitResults.h"
#include "AliCaloBunchInfo.h"

#include <iostream>

using namespace std;

ClassImp( AliCaloRawAnalyzerNN )

AliCaloRawAnalyzerNN::AliCaloRawAnalyzerNN() : AliCaloRawAnalyzer("Neural Network"), fNeuralNet(0)
{
  // Comment

  fNeuralNet = new AliCaloNeuralFit();

  for(int i=0; i < 5 ; i++)
    {
      fNNInput[i]  = 0;
    }

}


AliCaloRawAnalyzerNN::~AliCaloRawAnalyzerNN()
{
  delete fNeuralNet;
}


AliCaloFitResults 
AliCaloRawAnalyzerNN::Evaluate( const vector<AliCaloBunchInfo> &bunchvector, 
				       const UInt_t altrocfg1,  const UInt_t altrocfg2 )
{
  // The eveluation of  Peak position and amplitude using the Neural Network
  if( bunchvector.size()  <=  0 )
    {
      return AliCaloFitResults(9999, 9999, 9999, 9999 , 9999, 9999, 9999 );
    } 
 
  short maxindex;
  short maxamp;

  int bindex = SelectBunch( bunchvector, &maxindex , &maxamp ) ;
  
  if( bindex   < 0 )
    {
      return AliCaloFitResults(9999, 9999, 9999, 9999 , 9999, 9999, 9999 );
    }
  
  int first = 0;
  int last = 0;
 
  Float_t ped = ReverseAndSubtractPed( &(bunchvector.at( bindex ) )  ,  altrocfg1, altrocfg2, fReversed  );
  
  short maxrev = maxindex  -  bunchvector.at(bindex).GetStartBin();
  short timebinOffset = maxindex - (bunchvector.at(bindex).GetLength()-1);
  
  SelectSubarray( fReversed,  bunchvector.at(bindex).GetLength(),  maxrev , &first, &last);

  if(maxrev  < 1000 )
    {
      if (  ( maxrev   - first) < 2  &&  (last -   maxrev ) < 2)
	{
	  return AliCaloFitResults(9999, 9999, 9999, 9999 , 9999, 9999, 9999 );
	}
      else
	{

	  for(int i=0; i < 5 ; i++)
	    {
	      fNNInput[i]  = fReversed[maxrev-2 +i]/(maxamp -ped);
	    } 

	  	  
	  double amp = (maxamp - ped)*fNeuralNet->Value( 0,  fNNInput[0],  fNNInput[1], fNNInput[2], fNNInput[3], fNNInput[4]);
	  double tof = (fNeuralNet->Value( 1,  fNNInput[0],  fNNInput[1], fNNInput[2], fNNInput[3], fNNInput[4]) + timebinOffset ) ;

	  return AliCaloFitResults( maxamp, ped , -1, amp , tof, -2, -3 ); 

	}
    }
  return AliCaloFitResults(9999, 9999, 9999, 9999 , 9999, 9999, 9999 );
}



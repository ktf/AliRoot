#ifndef ROOT_TKDTree
#define ROOT_TKDTree

#ifndef ROOT_TObject
#include "TObject.h"
#endif

#include "TMath.h"


template <typename Index, typename Value> class TKDTree : public TObject
{
public:
	enum{
		kDimMax = 6
	};

	TKDTree();
	TKDTree(Index npoints, Index ndim, UInt_t bsize, Value **data);
	~TKDTree();
	
	// getters
	inline	Index*  GetPointsIndexes(Int_t node) const {
		if(node < fNnodes) return 0x0;
		Int_t offset = (node >= fCrossNode) ? (node-fCrossNode)*fBucketSize : fOffset+(node-fNnodes)*fBucketSize;
		return &fIndPoints[offset];
	}
	inline UChar_t GetNodeAxis(Int_t id) const {return (id < 0 || id >= fNnodes) ? 0 : fAxis[id];}
	inline Value   GetNodeValue(Int_t id) const {return (id < 0 || id >= fNnodes) ? 0 : fValue[id];}
	inline Int_t   GetNNodes() const {return fNnodes;}
	inline Int_t   GetNTNodes() const {return fNpoints/fBucketSize + ((fNpoints%fBucketSize)?1:0);}
	inline Value*  GetBoundaries();
	inline Value*  GetBoundary(const Int_t node);
	static  Int_t   GetIndex(Int_t row, Int_t collumn){return collumn+(1<<row);}
	static inline void GetCoord(Int_t index, Int_t &row, Int_t &collumn){for (row=0; index>=(16<<row);row+=4); for (; index>=(2<<row);row++);collumn= index-(1<<row);};
	        Bool_t  FindNearestNeighbors(const Value *point, const Int_t kNN, Index *&i, Value *&d);
	        Index   FindNode(const Value * point);
	        void    FindPoint(Value * point, Index &index, Int_t &iter);
	        void    FindInRangeA(Value * point, Value * delta, Index *res , Index &npoints,Index & iter, Int_t bnode);
	        void    FindInRangeB(Value * point, Value * delta, Index *res , Index &npoints,Index & iter, Int_t bnode);
	inline void    FindBNodeA(Value * point, Value * delta, Int_t &inode);
	inline Bool_t  IsTerminal(Index inode){return (inode>=fNnodes);}
	Value           KOrdStat(Index ntotal, Value *a, Index k, Index *index);
	void            MakeBoundaries(Value *range = 0x0);
	void            SetData(Index npoints, Index ndim, UInt_t bsize, Value **data);
	void            Spread(Index ntotal, Value *a, Index *index, Value &min, Value &max);

protected:
	void            Build();  // build tree
									
private:
	TKDTree(const TKDTree &); // not implemented
	TKDTree<Index, Value>& operator=(const TKDTree<Index, Value>&); // not implemented
	void            CookBoundaries(const Int_t node, Bool_t left);


protected:
	Bool_t  kDataOwner;  //! Toggle ownership of the data
	Int_t   fNnodes;     // size of node array
	Index   fNDim;       // number of dimensions
	Index   fNDimm;      // dummy 2*fNDim
	Index   fNpoints;    // number of multidimensional points
	Index   fBucketSize; // limit statistic for nodes 
	UChar_t *fAxis;      //[fNnodes] nodes cutting axis
	Value   *fValue;     //[fNnodes] nodes cutting value
	Value		*fRange;     //[fNDimm] range of data for each dimension
	Value   **fData;     //! data points
	Value		*fBoundaries;//! nodes boundaries - check class doc

// kNN related data
protected:
	Int_t   fkNNdim;     //! current kNN arrays allocated dimension
	Index   *fkNN;       //! k nearest neighbors indexes
	Value   *fkNNdist;   //! k nearest neighbors distances
	Value   *fDistBuffer;//! working space for kNN
	Index   *fIndBuffer; //! working space for kNN
	
private:
	Index   *fIndPoints; //! array of points indexes
	Int_t   fRowT0;      //! smallest terminal row
	Int_t   fCrossNode;  //! cross node
	Int_t   fOffset;     //! offset in fIndPoints

	ClassDef(TKDTree, 1)  // KD tree
};


typedef TKDTree<Int_t, Double_t> TKDTreeID;
typedef TKDTree<Int_t, Float_t> TKDTreeIF;

//_________________________________________________________________
template <typename  Index, typename Value>
void TKDTree<Index, Value>::FindBNodeA(Value *point, Value *delta, Int_t &inode){
  //
  // find the smallest node covering the full range - start
  //
  inode =0; 
  for (;inode<fNnodes;){
    if (TMath::Abs(point[fAxis[inode]] - fValue[inode])<delta[fAxis[inode]]) break;
    inode = (point[fAxis[inode]] < fValue[inode]) ? (inode*2)+1: (inode*2)+2;
  }
}

//_________________________________________________________________
template <typename  Index, typename Value>
Value* TKDTree<Index, Value>::GetBoundaries()
{
	if(!fBoundaries) MakeBoundaries();
	return fBoundaries;
}

//_________________________________________________________________
template <typename  Index, typename Value>
Value* TKDTree<Index, Value>::GetBoundary(const Int_t node)
{
	if(!fBoundaries) MakeBoundaries();
	return &fBoundaries[node*2*fNDim];
}

#endif


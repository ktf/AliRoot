#ifndef ALIITSGETTER_H
#define ALIITSGETTER_H

#include <AliLoader.h>
#include <AliITSVertex.h>

class AliITSLoader: public AliLoader
 {
   public:
    AliITSLoader(){};
    AliITSLoader(const Char_t *name,const Char_t *topfoldername);
    AliITSLoader(const Char_t *name,TFolder *topfolder);
    
    virtual ~AliITSLoader();

    void           MakeTree(Option_t* opt);

    //Raw Clusters
    virtual void   CleanRawClusters() {fRawClustersDataLoader.GetBaseLoader(0)->Clean();}
    Int_t          LoadRawClusters(Option_t* opt=""){return fRawClustersDataLoader.GetBaseLoader(0)->Load(opt);}
    void           SetRawClustersFileName(const TString& fname){fRawClustersDataLoader.SetFileName(fname);}
    TTree*         TreeC(){ return fRawClustersDataLoader.Tree();} // returns a pointer to the tree of  RawClusters
    void           UnloadRawClusters(){fRawClustersDataLoader.GetBaseLoader(0)->Unload();}
    virtual Int_t  WriteRawClusters(Option_t* opt=""){return fRawClustersDataLoader.GetBaseLoader(0)->WriteData(opt);}

    //Vertices
    virtual void   CleanVertices() {fVertexDataLoader.GetBaseLoader(0)->Clean();}
    Int_t          LoadVertices(Option_t* opt=""){return fVertexDataLoader.GetBaseLoader(0)->Load(opt);}
    void           SetVerticesFileName(const TString& fname){fVertexDataLoader.SetFileName(fname);}
    void           UnloadVertices(){fVertexDataLoader.GetBaseLoader(0)->Unload();}
    virtual Int_t  WriteVertices(Option_t* opt=""){return fVertexDataLoader.GetBaseLoader(0)->WriteData(opt);}
    virtual Int_t PostVertex(AliITSVertex *ptr){return fVertexDataLoader.GetBaseLoader(0)->Post(ptr);}
    //    virtual void SetVerticesContName(const char *name){fVertexDataLoader.GetBaseLoader(0)->SetName(name);}
    AliITSVertex *GetVertex(){return static_cast <AliITSVertex*>(fVertexDataLoader.GetBaseLoader(0)->Get());}

    //V0s
    virtual void   CleanV0s() {fV0DataLoader.GetBaseLoader(0)->Clean();}
    Int_t          LoadV0s(Option_t* opt=""){return fV0DataLoader.GetBaseLoader(0)->Load(opt);}
    void           SetV0FileName(const TString& fname){fV0DataLoader.SetFileName(fname);}
    void           UnloadV0s(){fV0DataLoader.GetBaseLoader(0)->Unload();}
    virtual Int_t  WriteV0s(Option_t* opt=""){return fV0DataLoader.GetBaseLoader(0)->WriteData(opt);}
    TTree*         TreeV0(){ return fV0DataLoader.Tree();}

    //Cascades
    virtual void   CleanCascades() {fCascadeDataLoader.GetBaseLoader(0)->Clean();}
    Int_t          LoadCascades(Option_t* opt=""){return fCascadeDataLoader.GetBaseLoader(0)->Load(opt);}
    void           SetCascadeFileName(const TString& fname){fCascadeDataLoader.SetFileName(fname);}
    void           UnloadCascades(){fCascadeDataLoader.GetBaseLoader(0)->Unload();}
    virtual Int_t  WriteCascades(Option_t* opt=""){return fCascadeDataLoader.GetBaseLoader(0)->WriteData(opt);}
    TTree*         TreeX(){ return fCascadeDataLoader.Tree();}

    //Back Propagated Tracks

    virtual void   CleanBackTracks() {fBackTracksDataLoader.GetBaseLoader(0)->Clean();}
    Int_t          LoadBackTracks(Option_t* opt=""){return fBackTracksDataLoader.GetBaseLoader(0)->Load(opt);}
    void           SetBackTracksFileName(const TString& fname){fBackTracksDataLoader.SetFileName(fname);}
    TTree*         TreeB(){ return fBackTracksDataLoader.Tree();} // returns a pointer to the tree of  BackTracks
    void           UnloadBackTracks(){fBackTracksDataLoader.GetBaseLoader(0)->Unload();}
    virtual Int_t  WriteBackTracks(Option_t* opt=""){return fBackTracksDataLoader.GetBaseLoader(0)->WriteData(opt);}
    

   protected:

    // METHODS
    virtual void   MakeRawClustersContainer() {fRawClustersDataLoader.MakeTree();}
    Int_t          PostRawClusters(){return fRawClustersDataLoader.GetBaseLoader(0)->Post();}

    virtual void   MakeBackTracksContainer() {fBackTracksDataLoader.MakeTree();}
    Int_t          PostBackTracks(){return fBackTracksDataLoader.GetBaseLoader(0)->Post();}
    virtual void   MakeV0Container() {fV0DataLoader.MakeTree();}
    Int_t          PostV0s(){return fV0DataLoader.GetBaseLoader(0)->Post();}

    virtual void   MakeCascadeContainer() {fCascadeDataLoader.MakeTree();}
    Int_t          PostCascades(){return fCascadeDataLoader.GetBaseLoader(0)->Post();}

    // DATA
    AliDataLoader fRawClustersDataLoader;
    static const TString fgkDefaultRawClustersContainerName;

    AliDataLoader fBackTracksDataLoader;
    static const TString fgkDefaultBackTracksContainerName;

    AliDataLoader fVertexDataLoader;
    static const TString fgkDefaultVerticesContainerName;

    AliDataLoader fV0DataLoader;
    static const TString fgkDefaultV0ContainerName;

    AliDataLoader fCascadeDataLoader;
    static const TString fgkDefaultCascadeContainerName;

   public:
     ClassDef(AliITSLoader,2)
 };
 
#endif



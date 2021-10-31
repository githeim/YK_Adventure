#ifndef _CTMX_READER_H_
#define _CTMX_READER_H_ 
#include <string>
#include <map>
#include <vector>

typedef struct {
  int iID;
  int iWidth;
  int iHeight;
  std::string strName;
  std::vector<int> vecTileIdx;
  std::string strEncoding;
} MapLayer;


typedef struct {
  std::string strImgSourceFile;
  std::string strName;
  int iTileWidth;
  int iTileHeight;
  int iSpacing;
  int iMargin;
  int iTilecount;
  int iColumns;
  int iImgWidth;
  int iImgHeight;
  // int : tile index ,  map : <property name, value>
  std::map<int,std::map<std::string,std::string>> mapProperties;
} TileSet;

typedef struct {
  std::string strVersion;
  int iMapWidth;
  int iMapHeight;

  int iTileWidth;
  int iTileHeight;

  // int : tileset's firstgid  , TileSet* : TileSet pointer
  std::map<int,TileSet*> mapTileSets;

  std::map<std::string,MapLayer> mapLayers;
} TMX_Ctx;



class CTMX_Reader {
  public:
    CTMX_Reader() {
      printf("\033[1;33m[%s][%d] :x: chk \033[m\n",__FUNCTION__,__LINE__);
    };
    ~CTMX_Reader() {
      printf("\033[1;33m[%s][%d] :x: chk \033[m\n",__FUNCTION__,__LINE__);
    };

    bool Read_TMX_Ctx(std::string strTMX_File, TMX_Ctx& TMX_context, 
                      std::string strFilePath=""); 
    bool Read_TSX_Ctx(std::string strTSX_File, TileSet& TileSet_Ctx,
                      std::string strFilePath="");

    static int GetTileSetFromIdx(TMX_Ctx &TMX_context, int iIdx, 
                                 TileSet* &pTileSet,int &iFirstgid);
    static int GetPhysicType(TMX_Ctx &TMX_context,
                             int iIdx,std::string &strPhysicType);

    static int GetProperty(TMX_Ctx &TMX_context, int iIdx,
                           std::string strProperty,
                           std::string& strValue);
    static int GetSpecifiedTileSize(TMX_Ctx &TMX_context,int iIdx,
        std::tuple<int,int,int,int> &Tile_XYWH_size);

    static int GetTag(TMX_Ctx &TMX_context,int iIdx, std::string& strTag);

    std::map<std::string, TileSet> m_mapTileSets;

};
#endif /* ifndef _CTMX_READER_H_ */

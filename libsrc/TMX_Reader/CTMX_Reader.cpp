#include "CTMX_Reader.h"
#include "tinyxml2/tinyxml2.h"
#include <algorithm>
#include <sstream>



/**
 * @brief Return integer value from character pointer
 *        if pointer is null , the return is 0
 *
 * @param pVal[IN]
 *
 * @return translate to integer
 */
static int SetAttrValue( const char* pVal) {
  if (pVal) 
    return std::stoi(pVal);
  return 0;
}

bool CTMX_Reader::Read_TSX_Ctx(std::string strTSX_file,TileSet& TileSet_Ctx,
                               std::string strFilePath ) {
  tinyxml2::XMLDocument xmldoc;
  std::string strTSX_Path = strFilePath + strTSX_file;
  if ( xmldoc.LoadFile(strTSX_Path.c_str()))  {
    printf("\033[1;31m[%s][%d] :x: err LoadFile [%s]  \033[m\n",
        __FUNCTION__,__LINE__,strTSX_Path.c_str());
    return false;
  }
  tinyxml2::XMLElement * pRootElement = xmldoc.RootElement();

  if (!pRootElement->Attribute("name") ) {
    printf("\033[1;32m[%s][%d] :x: Err can not get tileset name from"
        " [%s] \033[m\n",
        __FUNCTION__,__LINE__,strTSX_Path.c_str());
    return false;
  } else {
    TileSet_Ctx.strName = pRootElement->Attribute("name");
  }

  TileSet_Ctx.iTileWidth = 
    std::stoi(pRootElement->Attribute("tilewidth"));
  TileSet_Ctx.iTileHeight = 
    std::stoi(pRootElement->Attribute("tileheight"));

  TileSet_Ctx.iSpacing = SetAttrValue(pRootElement->Attribute("spacing"));
  TileSet_Ctx.iMargin  = SetAttrValue(pRootElement->Attribute("margin"));

  TileSet_Ctx.iTilecount = 
    std::stoi(pRootElement->Attribute("tilecount"));
  TileSet_Ctx.iColumns = 
    std::stoi(pRootElement->Attribute("columns"));

  auto pImage = pRootElement->FirstChildElement("image");
  TileSet_Ctx.strImgSourceFile = pImage->Attribute("source"); 
  TileSet_Ctx.iImgWidth = 
    std::stoi(pImage->Attribute("width")); 
  TileSet_Ctx.iImgHeight = 
    std::stoi(pImage->Attribute("height")); 


  std::map<int,std::map<std::string,std::string>> &mapProperties = 
    TileSet_Ctx.mapProperties;
  
  for (auto pTile = pRootElement->FirstChildElement("tile") ; 
      pTile ; pTile=pTile->NextSiblingElement("tile"))
  {
    int id = std::stoi(pTile->Attribute("id"));

    for (auto property = pTile->FirstChildElement("properties")->FirstChildElement("property") ;
        property ; property= property->NextSiblingElement("property")) {
      mapProperties[id][property->Attribute("name")]=property->Attribute("value");
    }
  }

  return true;
}
bool CTMX_Reader::Read_TMX_Ctx(std::string strTMX_file,TMX_Ctx& TMX_context,
                               std::string strFilePath) {
  tinyxml2::XMLDocument xmldoc;
  std::string strTMX_path = strFilePath + strTMX_file;
  if ( xmldoc.LoadFile(strTMX_path.c_str()))  {
    printf("\033[1;31m[%s][%d] :x: err \033[m\n",__FUNCTION__,__LINE__);
    return false;
  }

  tinyxml2::XMLElement * pRootElement = xmldoc.RootElement();
  TMX_context.strVersion = pRootElement->Attribute("version");

  TMX_context.iMapWidth   = std::stoi(pRootElement->Attribute("width"));
  TMX_context.iMapHeight  = std::stoi(pRootElement->Attribute("height"));
  TMX_context.iTileWidth  = std::stoi(pRootElement->Attribute("tilewidth"));
  TMX_context.iTileHeight = std::stoi(pRootElement->Attribute("tileheight"));
  for (auto pTileSet = pRootElement->FirstChildElement("tileset") ; 
       pTileSet ; pTileSet=pTileSet->NextSiblingElement("tileset"))
  {
    int iFirstgid = std::stoi(pTileSet->Attribute("firstgid"));
    std::string strTsxSource = pTileSet->Attribute("source");
    if( m_mapTileSets.find(strTsxSource) == m_mapTileSets.end() ) {
      // can not find existed tileset, now create new tileset
      Read_TSX_Ctx(strTsxSource,  m_mapTileSets[strTsxSource],strFilePath);

    }
    TMX_context.mapTileSets[iFirstgid] = &m_mapTileSets[strTsxSource];
  }
  tinyxml2::XMLElement* pLayer = pRootElement->FirstChildElement("layer");
  if (pLayer == nullptr) {
    printf("\033[1;31m[%s][%d] :x: ERR,No layer \033[m\n",__FUNCTION__,__LINE__);
    return false;
  }
  while (pLayer) {
    MapLayer Layer;
    Layer.iID     = std::stoi(pLayer->Attribute("id"));
    Layer.strName = pLayer->Attribute("name");
    Layer.iWidth  = std::stoi(pLayer->Attribute("width"));
    Layer.iHeight = std::stoi(pLayer->Attribute("height"));
    auto pData = pLayer->FirstChildElement("data");

    std::stringstream ssData(pData->GetText());
    int iIdx;
    while (ssData >> iIdx) {
      if (ssData.peek() == ',') {
        ssData.ignore();
      }
      Layer.vecTileIdx.push_back(iIdx);
    }
    TMX_context.mapLayers[Layer.strName] = Layer;
    
    pLayer = pLayer->NextSiblingElement("layer");
  }



  return true;
}


/**
 * @brief Get tileset and its first gid from tile idx , 
 *        해당 타일(index)이 속한 타일 셋과 first gid를 반환한다
 *
 * @param TMX_context[IN]
 * @param iIdx[IN] tile index
 * @param pTileSet[OUT] the tileset that the tile(index) is included
 *
 * @return success, 0
 */
int CTMX_Reader::GetTileSetFromIdx(TMX_Ctx &TMX_context, 
                                   int iIdx, TileSet *&pTileSet,int &iFirstgid) {
  // 조건문,  해당 인덱스가 해당 타일셋 범위안에 있는지 확인한다
  auto Cond = [&iIdx](auto Item) {
     int iTmpFirstgid  = Item.first;
     int iTilecount = Item.second->iTilecount;
     return (bool)(iIdx >= iTmpFirstgid && iIdx < iTmpFirstgid+iTilecount );
  };
  auto Item = std::find_if(
                           begin(TMX_context.mapTileSets),
                           end(TMX_context.mapTileSets),
                           Cond);
  if (Item ==std::end(TMX_context.mapTileSets))
    return -1;
  iFirstgid = Item->first;
  pTileSet = Item->second;
  return 0;
}


/**
 * @brief Get Property of the tile
 *
 * @param TMX_context[IN]
 * @param iIdx[IN]
 * @param strProperty[IN] property name to know
 * @param strValue[OUT]   property's value
 *
 * @return success, 0
 */
int CTMX_Reader::GetProperty(TMX_Ctx &TMX_context, int iIdx,
                       std::string strProperty,
                       std::string &strValue) {
  TileSet* pTileSet;
  int iFirstgid;
  if (GetTileSetFromIdx(TMX_context, iIdx, pTileSet,iFirstgid))
    return -1;
  
  if (pTileSet->mapProperties.find(iIdx - iFirstgid) == 
      pTileSet->mapProperties.end()) 
  {
    // tile's properties are not existed
    return -1;
  }
  if (pTileSet->mapProperties[iIdx - iFirstgid].find(strProperty) ==
      pTileSet->mapProperties[iIdx - iFirstgid].end())
  {
    // the specified property is not exited
    return -1;
  }
  strValue = pTileSet->mapProperties[iIdx - iFirstgid][strProperty];

  return 0;
}


/**
 * @brief Get Physictype property from idx number
 *
 * @param TMX_context[IN]
 * @param iIdx[IN]
 * @param strPhysicType[OUT]
 *
 * @return 
 */
int CTMX_Reader::GetPhysicType  (TMX_Ctx &TMX_context,
                             int iIdx,std::string &strPhysicType) {
  return GetProperty(TMX_context,iIdx,"PhysicType",strPhysicType);
}

/**
 * @brief Get Specified tile size
 *        if the tile has properties 
 *           "PhysicSize_W","PhysicSize_H","PhysicSize_X","PhysicSize_Y"
 *        , returns its x,y,w,h pizel Size
 *
 * @param TMX_context[IN]
 * @param iIdx[IN]
 * @param Tile_XYWH_size[OUT] specified x,y,w,h pixel size
 *
 * @return if it does not have the properties, it will returns -1
 */
int CTMX_Reader::GetSpecifiedTileSize(TMX_Ctx &TMX_context, int iIdx, 
                              std::tuple<int, int, int, int> &Tile_XYWH_size) {
  // Get specified size of the object
  std::string strPhysicSize_W;
  std::string strPhysicSize_H;
  std::string strPhysicSize_X;
  std::string strPhysicSize_Y;

  if (
    !(!CTMX_Reader::GetProperty(TMX_context, iIdx, "PhysicSize_W", strPhysicSize_W) && 
      !CTMX_Reader::GetProperty(TMX_context, iIdx, "PhysicSize_H", strPhysicSize_H) && 
      !CTMX_Reader::GetProperty(TMX_context, iIdx, "PhysicSize_X", strPhysicSize_X) && 
      !CTMX_Reader::GetProperty(TMX_context, iIdx, "PhysicSize_Y", strPhysicSize_Y) )
     ){
    return -1;
  }
  Tile_XYWH_size = { std::stoi(strPhysicSize_X),
                     std::stoi(strPhysicSize_Y),
                     std::stoi(strPhysicSize_W),
                     std::stoi(strPhysicSize_H)};
  return 0;
}
int CTMX_Reader::GetTag(TMX_Ctx &TMX_context, int iIdx, std::string &strTag) {
  if (
      CTMX_Reader::GetProperty(TMX_context, iIdx, "Tag", strTag))
    return -1;
  return 0;
}

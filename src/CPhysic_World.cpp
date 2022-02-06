#include "CPhysic_World.h"
#include "algorithm"

static float GetDistance(float fX1,float fY1, float fX2, float fY2) {
  // Calculating distance
  return std::sqrt(pow(fX2 - fX1, 2) +
      pow(fY2 - fY1, 2) * 1.0);
};

class WorldContactListener : public b2ContactListener
{
public:
  WorldContactListener(CPhysic_World* pWorld):m_pPhysic_World(pWorld) {
  }
  ~WorldContactListener() {
  }
  
  void BeginContact(b2Contact* contact) {

  }

  void EndContact(b2Contact* contact) {

  }

  CPhysic_World* m_pPhysic_World;
};
int CPhysic_World::Create_World(TMX_Ctx &TMX_context,
                                std::map<std::string,ObjAttr_t*>& mapObjs) {
  b2Vec2 gravity(0.0f,-10.0f);
  m_pWorld = new b2World(gravity);
  m_pContactListener = new WorldContactListener(this);
  m_pWorld->SetContactListener(m_pContactListener);
  // Create body define
  Create_BodyDefs(m_mapBodyDef);
  // Create edge shape
  Create_EdgeShapes(m_mapEdgeshape);
  // Create polygon shape
  Create_PolygonShape(m_mapPolygonShape);

  float fLeftEdge_M;
  float fTopEdge_M;
  int iIdxCnt=0;
  // 기본 타일 사이즈 (18 pixel = 1M)
  // 기본 타일 사이즈 pixel
  float fBaseTileWidth = (float)TMX_context.iTileWidth;
  float fBaseTileHeight =(float)TMX_context.iTileHeight;
  // 기본 타일 사이즈 meter
  float fBaseTileWidth_M = (float)TMX_context.iTileWidth/(float)TMX_context.iTileWidth;
  float fBaseTileHeight_M =(float)TMX_context.iTileHeight/(float)TMX_context.iTileHeight;

  // 레이어 별로 아이템을 읽어서 body를 생성한다
  for (auto item : TMX_context.mapLayers) {
    printf("\033[1;33m[%s][%d] :x: Layer %s \033[m\n",
        __FUNCTION__,__LINE__,item.first.c_str());
    auto &Layer = item.second;
    // 물리 영역 좌상단
    fLeftEdge_M = - ( Layer.iWidth/2.0f )* fBaseTileWidth_M; ;
    fTopEdge_M  =   ( Layer.iHeight/2.0f)* fBaseTileHeight_M;
    iIdxCnt = 0;

    for ( auto iIdx : Layer.vecTileIdx) {
      TileSet *pTileSet;
      int iFirstgid;
      CTMX_Reader::GetTileSetFromIdx(TMX_context,iIdx,pTileSet,iFirstgid);

      // 해당 인덱스의 타일 사이즈, base tile을 1M로 상정하고 Meter를 계산
      float fTileSizeWidth_M =(float)(pTileSet->iTileWidth) /fBaseTileWidth;
      float fTileSizeHeight_M=(float)(pTileSet->iTileHeight)/fBaseTileHeight;
      
      // if the tile has specified its own size, it will be apply to 
      // physic engine
      std::tuple<int,int,int,int> Tile_XYWH_size;
      if (!CTMX_Reader::GetSpecifiedTileSize(TMX_context,iIdx,Tile_XYWH_size)) {
        fTileSizeWidth_M  = (float)std::get<2>(Tile_XYWH_size)/fBaseTileWidth;
        fTileSizeHeight_M = (float)std::get<3>(Tile_XYWH_size)/fBaseTileHeight;
      }
      // to remove compile warning
      fTileSizeWidth_M = fTileSizeWidth_M;

      // X,Y coordination in Index
      int iX_idx= iIdxCnt % Layer.iWidth;
      int iY_idx= iIdxCnt / Layer.iWidth;
      // X,Y coordination in meter
      // 좌표는 기본 타일 사이즈를 기준으로 계산한다
      float fX_M = fLeftEdge_M+ iX_idx* fBaseTileWidth_M  + fBaseTileWidth_M /2.f;
      // 해당 타일이 기본 타일보다 커도 바닥 위치를 일치시켜 배치한다
      float fY_M = fTopEdge_M - iY_idx* fBaseTileHeight_M - fBaseTileHeight_M/2.f
        + ((fTileSizeHeight_M /2)-(fTileSizeHeight_M/2));
      if (iIdx !=0) {
        Create_Element(TMX_context, iIdx,
            fX_M,fY_M,mapObjs);
      }

      iIdxCnt++;
    } 

  }

  for (auto item : m_mapTags) {
    for (auto pBody : item.second) {
    printf("\033[1;36m[%s][%d] :x: m_mapTags %s %p \033[m\n",
        __FUNCTION__,__LINE__,item.first.c_str(),pBody);
    }

  }
  return 0;
}

int CPhysic_World::Destroy_World() {
  for (auto BodyDef : m_mapBodyDef) {
    delete BodyDef.second;
  }
  
  for (auto PolygonShape : m_mapPolygonShape) {
    delete PolygonShape.second;
  }
  for (auto EdgeShape : m_mapEdgeshape) {
    delete EdgeShape.second;
  }

  if (m_pContactListener)
    delete m_pContactListener;

  delete m_pWorld;
  return 0;
}

void CPhysic_World::SpinWorld(double dbTimeStep, 
    int iVelocityIter, int iPositionIter ) {
  m_pWorld->Step(dbTimeStep, iVelocityIter, iPositionIter);
  return;
}

/**
 * @brief create Objects map 
 *
 * @param TMX_context[IN]
 * @param iTileIdx[IN]
 * @param fX_M[IN]
 * @param fY_M[IN]
 * @param mapObjs[OUT]
 *
 * @return created object's name 
 */
std::string CPhysic_World::Create_Element(TMX_Ctx &TMX_context, int iTileIdx,
                                  float fX_M,float fY_M,
                                  std::map<std::string,ObjAttr_t*>& mapObjs) {
  std::string strPhysicType;
  // 기본 타일 사이즈 pixel
  float fBaseTileWidth = (float)TMX_context.iTileWidth;
  float fBaseTileHeight =(float)TMX_context.iTileHeight;

  b2Body* pBody = nullptr;
  std::string strObjName="";
  TileSet *pTileSet;
  int iFirstgid;
  CTMX_Reader::GetTileSetFromIdx(TMX_context,iTileIdx,pTileSet,iFirstgid);
  // 해당 인덱스의 타일 사이즈, base tile을 1M로 상정하고 Meter를 계산
  float fTileSizeWidth_M =(float)(pTileSet->iTileWidth) /fBaseTileWidth;
  float fTileSizeHeight_M=(float)(pTileSet->iTileHeight)/fBaseTileHeight;

  // if the tile has specified its own size, it will be apply to 
  // physic engine
  std::tuple<int,int,int,int> Tile_XYWH_size;
  if (!CTMX_Reader::GetSpecifiedTileSize(TMX_context,iTileIdx,Tile_XYWH_size)) {
    fTileSizeWidth_M  = (float)std::get<2>(Tile_XYWH_size)/fBaseTileWidth;
    fTileSizeHeight_M = (float)std::get<3>(Tile_XYWH_size)/fBaseTileHeight;
  }

  if ( !CTMX_Reader::GetPhysicType(TMX_context,iTileIdx,strPhysicType)) {
    if (strPhysicType == "Background") {
      std::string strTag="";
      static int iCnt = 0;
      iCnt++;
      m_vecBackground.push_back(
          std::tuple<int, float,float,float,float,float>(
            iTileIdx,fX_M,fY_M,1.33,1.33,0));
      char pBuf[256]={}; 
      sprintf(pBuf,"%s_%03.2f_%03.2f",strPhysicType.c_str(),fX_M,fY_M);
      strObjName = pBuf;
      if (!CTMX_Reader::GetTag(TMX_context, iTileIdx, strTag)) {
        printf("\033[1;32m[%s][%d] :x: Tag %s \033[m\n",
            __FUNCTION__,__LINE__,strTag.c_str());
      }
      // background tile is 1.33M x 1.33M
      float fW_M=1.33, fH_M=1.33,fAngle =0;
      Register_Background(mapObjs,
                          strObjName,strTag,strPhysicType,
                          iTileIdx,fX_M,fY_M,fW_M,fH_M,fAngle);
    }

    if (strPhysicType == "Dynamic") {
      m_iObjCnt++;

      float fAngle =0;
      pBody = m_pWorld->CreateBody(m_mapBodyDef["dynamic_tile"]);
      std::string strTag;
      std::vector<std::string> vecTag;
      vecTag.clear();
      if (!CTMX_Reader::GetTag(TMX_context, iTileIdx, strTag)) {
        printf("\033[1;32m[%s][%d] :x: Tag %s \033[m\n",__FUNCTION__,__LINE__,strTag.c_str());
        m_mapTags[strTag].push_back(pBody); 
        vecTag.push_back(strTag);
      }
      char pBuf[256]={}; 
      sprintf(pBuf,"%s_%s_%05d",strPhysicType.c_str(),strTag.c_str(),m_iObjCnt);
      strObjName = pBuf;

      // Find the matched size of polygon
      auto Polygon = std::find_if(m_mapPolygonShape.begin(),m_mapPolygonShape.end(),
          [&fTileSizeWidth_M,&fTileSizeHeight_M](auto item) {
          b2PolygonShape* pShape = item.second;
          b2Vec2* vertices = pShape->m_vertices;
          float fWidth = GetDistance(vertices[0].x,vertices[0].y,
              vertices[1].x,vertices[1].y);
          float fHeight = GetDistance(vertices[1].x,vertices[1].y,
              vertices[2].x,vertices[2].y);
          if (fWidth == fTileSizeWidth_M && fHeight == fTileSizeHeight_M)
            return true;
          return false;
          });
      if (Polygon == m_mapPolygonShape.end())
      {
        // if there is no same polygon size, then make the new polygon
        std::string strPolygonName=std::to_string(fTileSizeWidth_M) + "_"+
          std::to_string(fTileSizeHeight_M);

        b2PolygonShape* pPolygon = new b2PolygonShape();
        pPolygon->SetAsBox(fTileSizeWidth_M/2.f, fTileSizeHeight_M/2.f);
        m_mapPolygonShape[strPolygonName] = pPolygon;
        pBody->CreateFixture(m_mapPolygonShape[strPolygonName],30.f);
      }
      else{
        pBody->CreateFixture(Polygon->second,30.f);
      }
      pBody->SetTransform(b2Vec2(fX_M,fY_M),0.f); 

      Register_Obj(mapObjs,strObjName,pBody,vecTag,strPhysicType,
                   iTileIdx,fTileSizeWidth_M,fTileSizeHeight_M,fAngle,
                   m_mapTagObj);

    }
    else if (strPhysicType == "Ground") {
      char pBuf[256]={}; 
      sprintf(pBuf,"%s_%03.2f_%03.2f",strPhysicType.c_str(),fX_M,fY_M);
      strObjName = pBuf;
      std::vector<std::string> vecTag;
      vecTag.clear();
      float fGroundTile_W_M=1.0;
      float fGroundTile_H_M=1.0;
      float fGroundTile_Angle=.0;

      //printf("\033[1;33m[%s][%d] :x: Ground \033[m\n",__FUNCTION__,__LINE__);
      b2Body* pBody =m_pWorld->CreateBody(m_mapBodyDef["ground_tile"]);
      // :x: using edge shape for tile structure
      pBody->CreateFixture(m_mapEdgeshape["1X1_Box_Up"],0.f);
      pBody->CreateFixture(m_mapEdgeshape["1X1_Box_Down"],0.f);
      pBody->CreateFixture(m_mapEdgeshape["1X1_Box_Left"],0.f);
      pBody->CreateFixture(m_mapEdgeshape["1X1_Box_Right"],0.f);

      pBody->SetTransform(b2Vec2(fX_M,fY_M),0.f); 

      Register_Obj(mapObjs, strObjName, pBody, vecTag, strPhysicType, iTileIdx, 
          fGroundTile_W_M, fGroundTile_H_M, fGroundTile_Angle,m_mapTagObj);
    }
  }


  return strObjName;
}

int CPhysic_World::Create_BodyDefs(std::map<std::string,b2BodyDef*> &mapBodyDef) {
  // Define body define
  b2BodyDef* pBodyDef;
  pBodyDef = new b2BodyDef();
  pBodyDef->type = b2_staticBody;
  mapBodyDef["ground_tile"] = pBodyDef;

  pBodyDef = new b2BodyDef();
  pBodyDef->type = b2_dynamicBody;
  mapBodyDef["dynamic_tile"] = pBodyDef;

  return 0;
}

int CPhysic_World::Create_EdgeShapes(
    std::map<std::string,b2EdgeShape*>&  mapEdgeshape) {
  // Define shapes
  // basic 1 m X 1 m box ; 18 pixel X 18 pixel
  b2EdgeShape* pBasic_1X1_Box_Up = new b2EdgeShape();
  b2EdgeShape* pBasic_1X1_Box_Down = new b2EdgeShape();
  b2EdgeShape* pBasic_1X1_Box_Left = new b2EdgeShape();
  b2EdgeShape* pBasic_1X1_Box_Right = new b2EdgeShape();
  b2Vec2 v0(-0.5f, 0.5f);
  b2Vec2 v1( 0.5f, 0.5f);
  b2Vec2 v2( 0.5f,-0.5f);
  b2Vec2 v3(-0.5f,-0.5f);

  pBasic_1X1_Box_Up->SetOneSided(v2,v1,v0,v3);
  pBasic_1X1_Box_Down->SetOneSided(v0,v3,v2,v1);
  pBasic_1X1_Box_Left->SetOneSided(v1,v0,v3,v2);
  pBasic_1X1_Box_Right->SetOneSided(v3,v2,v1,v0);
  mapEdgeshape["1X1_Box_Up"]    = pBasic_1X1_Box_Up;
  mapEdgeshape["1X1_Box_Down"]  = pBasic_1X1_Box_Down;
  mapEdgeshape["1X1_Box_Left"]  = pBasic_1X1_Box_Left;
  mapEdgeshape["1X1_Box_Right"] = pBasic_1X1_Box_Right;

  return 0;
}

int CPhysic_World::Create_PolygonShape(
                    std::map<std::string,b2PolygonShape*>& mapPolygonShape) {
  // Character 1.33 m X 1.33 m box ; 24 pixel X 24 pixel
  b2PolygonShape* pCharacter_Box = new b2PolygonShape();
  pCharacter_Box->SetAsBox(float(24.f/2.f/m_fScale_Pixel_per_Meter),
                           float(24.f/2.f/m_fScale_Pixel_per_Meter));
  mapPolygonShape["Character_Box"] = pCharacter_Box;
  return 0;
}
int CPhysic_World::Register_Background(std::map<std::string, ObjAttr_t*> &mapObjs,
    std::string &strObjName,
    std::string &strTag, std::string &strPhysicType, int &iTileIdx, 
    float &fX_M, float &fY_M, float &fW_M, float &fH_M, float &fAngle) {
  mapObjs[strObjName] = new ObjAttr_t;
  mapObjs[strObjName]->strObjName = strObjName;
  mapObjs[strObjName]->iTileIdx = iTileIdx;
  mapObjs[strObjName]->pBody = nullptr;
  if (strTag != "") {
    mapObjs[strObjName]->vecTag.clear(); 
    mapObjs[strObjName]->vecTag.push_back(strTag); 
  }
  mapObjs[strObjName]->strPhysicType = strPhysicType;
  mapObjs[strObjName]->pPlugin=nullptr;
  mapObjs[strObjName]->TileInfo={fX_M,fY_M,1.33,1.33,0};

  // Set getter functions
  mapObjs[strObjName]->fX_M   = std::bind(&ObjAttr_t::GetStaticTile_fX_M,mapObjs[strObjName]);
  mapObjs[strObjName]->fY_M   = std::bind(&ObjAttr_t::GetStaticTile_fY_M,mapObjs[strObjName]);
  mapObjs[strObjName]->fW_M   = std::bind(&ObjAttr_t::GetTile_fW_M,mapObjs[strObjName]);
  mapObjs[strObjName]->fH_M   = std::bind(&ObjAttr_t::GetTile_fH_M,mapObjs[strObjName]);
  mapObjs[strObjName]->fAngle = std::bind(&ObjAttr_t::GetStaticTile_fAngle,mapObjs[strObjName]);
#if 0 // :x: for test
  printf("\033[1;33m[%s][%d] :x: [Background Tiles] %s  %d %f,%f,%f,%f,%f \033[m\n",
      __FUNCTION__,__LINE__,
      strObjName.c_str(),iTileIdx,
      mapObjs[strObjName]->fX_M(),
      mapObjs[strObjName]->fY_M(),
      mapObjs[strObjName]->fW_M(),
      mapObjs[strObjName]->fH_M(),  
      mapObjs[strObjName]->fAngle()
      );
#endif // :x: for test
  return 0;
}
int CPhysic_World::Register_Obj(std::map<std::string, ObjAttr_t*> &mapObjs, 
    std::string &strObjName, b2Body *pBody, std::vector<std::string> &vecTag, 
    std::string &strPhysicType,  
    int &iTileIdx, 
    float &fW_M, float &fH_M, float &fAngle,
    std::map<std::string,std::vector<ObjAttr_t*>> &mapTagObj
 )
{
  mapObjs[strObjName] = new ObjAttr_t;
  mapObjs[strObjName]->strObjName = strObjName;
  mapObjs[strObjName]->iTileIdx = iTileIdx;
  mapObjs[strObjName]->pBody = pBody;
  mapObjs[strObjName]->vecTag.clear();
  if (vecTag.size() !=0) {
    for (auto Tag : vecTag) {
      mapObjs[strObjName]->vecTag.push_back(Tag);
    }
  }
  mapObjs[strObjName]->strPhysicType = strPhysicType;
  mapObjs[strObjName]->pPlugin=nullptr;
  mapObjs[strObjName]->TileInfo={0,0,fW_M,fH_M,fAngle};

  // Set getter functions
  mapObjs[strObjName]->fX_M   = std::bind(&ObjAttr_t::GetBody_fX_M,mapObjs[strObjName]);
  mapObjs[strObjName]->fY_M   = std::bind(&ObjAttr_t::GetBody_fY_M,mapObjs[strObjName]);
  mapObjs[strObjName]->fW_M   = std::bind(&ObjAttr_t::GetTile_fW_M,mapObjs[strObjName]);
  mapObjs[strObjName]->fH_M   = std::bind(&ObjAttr_t::GetTile_fH_M,mapObjs[strObjName]);
  mapObjs[strObjName]->fAngle = std::bind(&ObjAttr_t::GetBody_fAngle,mapObjs[strObjName]);
#if 0 // :x: for test
  printf("\033[1;33m[%s][%d] :x: [Obj Tiles] %s  %d %f,%f,%f,%f,%f \033[m\n",
      __FUNCTION__,__LINE__,
      strObjName.c_str(),iTileIdx,
      mapObjs[strObjName].fX_M(),
      mapObjs[strObjName].fY_M(),
      mapObjs[strObjName].fW_M(),
      mapObjs[strObjName].fH_M(),  
      mapObjs[strObjName].fAngle()
      );
#endif // :x: for test

  for (auto strTag : vecTag) {
    m_mapTagObj[strTag].push_back(mapObjs[strObjName]);
  }
    
  return 0;
}

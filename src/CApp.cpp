#include "CApp.h"
#include "CPlugin.h"

int CApp::Init(
      SDL_Window*   &pWindow, 
      SDL_Renderer* &pRenderer,
      TTF_Font*     &pFont 
    ) {
  if ( Init_SDL_ctx(pWindow,pRenderer) ) {
    printf("\033[1;31m[%s][%d] :x: Err \033[m\n",__FUNCTION__,__LINE__);
    return -1;
  }

  if ( Init_Fonts(pFont) ) {
    printf("\033[1;31m[%s][%d] :x: Err \033[m\n",__FUNCTION__,__LINE__);
    return -1;
  }
  m_pWorld = nullptr;

  // Create Texture for drawing vector box
  m_pVectorBoxTexture = SDL_CreateTexture(pRenderer, 0, 
                                          SDL_TEXTUREACCESS_TARGET, 10, 10);
  SDL_Rect DstRect = {0,0,10,10};
  SDL_SetRenderTarget(pRenderer, m_pVectorBoxTexture);
  SDL_SetRenderDrawColor(pRenderer, 255,105, 5, SDL_ALPHA_OPAQUE);
  SDL_RenderDrawRect(pRenderer, &DstRect);
  SDL_SetRenderTarget(pRenderer,nullptr);

  return 0;
}

int CApp::DeInit(
    SDL_Window*   &pWindow, 
    SDL_Renderer* &pRenderer,
    TTF_Font*     &pFont 
    ) {
  if (DeInit_Fonts(pFont) ) {
    return -1;
  }
  if (DeInit_SDL_ctx(pWindow, pRenderer)) {
    return -1;
  }

  // Clear textures
  for ( auto item : m_mapTextures) {
    SDL_DestroyTexture(item.second);
  }
  for (auto item : m_mapDrawingTextures) {
    SDL_DestroyTexture(item.second);
  }
  SDL_DestroyTexture(m_pVectorBoxTexture);

  for (auto Obj : m_mapObjs) {
    delete Obj.second;
  }
  
  // Clear Physic World
  if (m_pWorld) {
    delete m_pWorld;
  }

  return 0;
}

/**
 * @brief create textures for each tileset
 *
 * @param TMX_context[IN]
 * @param mapTexture[OUT]
 *
 * @return 
 */
int CApp::Register_Textures(TMX_Ctx                    & TMX_context,
                             std::string                & strTileResourcePath,
                             SDL_Renderer*              & pRenderer,
                             std::map<int,SDL_Texture*> & mapTexture
                             ) {
    Uint32 format;
    int access;
    int w,h;

  for (auto item : TMX_context.mapTileSets) {
    printf("\033[1;33m[%s][%d] :x: chk %d %s %s\033[m\n",
        __FUNCTION__,__LINE__,item.first,item.second->strImgSourceFile.c_str(),
       strTileResourcePath.c_str() );
    std::string strTileSetfilepath = 
                               strTileResourcePath+item.second->strImgSourceFile;
    mapTexture[item.first] = IMG_LoadTexture(pRenderer,strTileSetfilepath.c_str());

    SDL_QueryTexture(mapTexture[item.first], &format, &access, &w, &h);
  }

  // 화면출력을 위한 Drawing 전용 텍스처를 만든다
  for (int i= 0; i< MAX_SCREENS ; i++) {
    m_mapDrawingTextures[i] =SDL_CreateTexture(pRenderer, format,
        SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
  }
  return 0;
}


/**
 * @brief Get coordinates of the sprite 
 *
 * @param TMX_context[IN]
 * @param mapSprites[OUT]
 *
 * @return 
 */
int CApp::Generate_Sprites(
    TMX_Ctx &TMX_context, std::map<int, Sprite_t> &mapSprites) {
  
  for (auto tileset : TMX_context.mapTileSets) {
    int iFirstgid =tileset.first;
    int iTilecount = tileset.second->iTilecount;
    
    for (int iTileCnt =iFirstgid ; iTileCnt < iFirstgid + iTilecount ; 
        iTileCnt++) {
      int iIdxInTileset = iTileCnt - iFirstgid;
      int iTileWidth  = tileset.second->iTileWidth;
      int iTileHeight = tileset.second->iTileHeight;
      int iMargin     = tileset.second->iMargin;
      int iSpacing    = tileset.second->iSpacing;
      int iColumns    = tileset.second->iColumns;

      int iTileX   = iMargin + (iIdxInTileset%iColumns)*(iTileWidth+iSpacing);
      int iTileY   = iMargin + (iIdxInTileset/iColumns)*(iTileHeight+iSpacing);
      // if the tile has specified tile size, apply it
      std::tuple<int,int,int,int> Tile_XYWH_size;
      if (!CTMX_Reader::GetSpecifiedTileSize(TMX_context,iTileCnt,Tile_XYWH_size)) {
        iTileX +=     std::get<0>(Tile_XYWH_size);
        iTileY +=     std::get<1>(Tile_XYWH_size);
        iTileWidth  = std::get<2>(Tile_XYWH_size);
        iTileHeight = std::get<3>(Tile_XYWH_size);
      }

      mapSprites[iTileCnt] = {iFirstgid, {iTileX,iTileY,iTileWidth,iTileHeight}};
    }
  }
  return 0;
}
int CApp::OnExecute(SDL_Renderer* pRenderer) {
  bool bQuit = false;

  SDL_Event Evt;
  SDL_Event* pEvt=nullptr;

  double dbActualFPS = 0.0f;
  double dbTimeDiff = 0;

  //int iCnt=0;
  if (SDL_SetRenderDrawColor(pRenderer, 255, 0, 0, SDL_ALPHA_OPAQUE)) {
    printf("\033[1;31m[%s][%d] :x: Err \033[m\n",__FUNCTION__,__LINE__);
    bQuit = true;
  }
  if (SDL_RenderClear(pRenderer)) {
    printf("\033[1;31m[%s][%d] :x: Err \033[m\n",__FUNCTION__,__LINE__);
    bQuit = true;
  }

#if 0 // :x: for test
  int iSizeW = SCREEN_WIDTH/2;
  int iSizeH = SCREEN_HEIGHT/2;
  SDL_Rect rectScreen0 ={0,0,iSizeW,iSizeH};
  SDL_Rect rectScreen1 ={iSizeW,0,iSizeW,iSizeH};
  SDL_Rect rectScreen2 ={0,iSizeH,iSizeW,iSizeH};
  SDL_Rect rectScreen3 ={iSizeW,iSizeH,iSizeW,iSizeH};
#endif // :x: for test

  // Init Plugins
  //Init_Plugins(m_pWorld,m_mapObjs,m_vecPluginInstance);
  Init_ObjPlugins(m_pWorld,m_mapObjs,m_vecObjPluginInstance);
  while (!bQuit) {

    Draw_World(m_pWorld);
    
    SDL_SetRenderTarget(pRenderer,nullptr);
    SDL_RenderCopyEx(pRenderer,m_mapDrawingTextures[0],NULL,NULL,0,NULL,SDL_FLIP_NONE);
#if 0 // :x: for test
    SDL_RenderCopyEx(pRenderer,m_mapDrawingTextures[0],NULL,&rectScreen0,0,NULL,SDL_FLIP_NONE);
    SDL_RenderCopyEx(pRenderer,m_mapDrawingTextures[0],NULL,&rectScreen1,0,NULL,SDL_FLIP_NONE);
    SDL_RenderCopyEx(pRenderer,m_mapDrawingTextures[0],NULL,&rectScreen2,0,NULL,SDL_FLIP_NONE);
    SDL_RenderCopyEx(pRenderer,m_mapDrawingTextures[0],NULL,&rectScreen3,0,NULL,SDL_FLIP_NONE);
#endif // :x: for test
 //   SDL_RenderCopyEx(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_Rect *dstrect, const double angle, const SDL_Point *center, const SDL_RendererFlip flip)
    

    if (bQuit ==true) 
      break;
    // evt handling
    if ( SDL_PollEvent( &Evt ) == 0 ) {
      pEvt = nullptr;
    }
    else
    {
      //User requests quit
      if( Evt.type == SDL_QUIT )
      {
        bQuit = true;
      }
      else if( Evt.type == SDL_KEYDOWN )
      {
        switch( Evt.key.keysym.sym )
        {
          case SDLK_q:
            bQuit = true;
            break;

          case SDLK_d:
            break;
          case SDLK_LEFT:
            (std::get<0>(m_DisplayOffSet))--;
            break;
          case SDLK_RIGHT:
            (std::get<0>(m_DisplayOffSet))++;
            break;
          case SDLK_UP:
            (std::get<1>(m_DisplayOffSet))--;
            break;
          case SDLK_DOWN:
            (std::get<1>(m_DisplayOffSet))++;
            break;
          case SDLK_t:
            m_bTileDraw=!m_bTileDraw;
            m_bVectorDraw=!m_bVectorDraw;
            break;
          case SDLK_p:
            m_bStopFlag =!m_bStopFlag;
            break;
        }
        //printf("\033[1;33m[%s][%d] :x: Offset %d %d \033[m\n",
        //    __FUNCTION__,__LINE__,
        //    std::get<0>(m_DisplayOffSet),
        //    std::get<1>(m_DisplayOffSet)
        //    );
      }

      pEvt= &Evt;
    }
    //Execute_Plugins(m_pWorld,pEvt,dbTimeDiff,m_vecPluginInstance);
    Execute_Plugins(m_pWorld,m_mapObjs,pEvt,dbTimeDiff,m_vecObjPluginInstance);
    Add_Objs(m_pWorld,m_mapObjs,pEvt,dbTimeDiff, m_vecObjPluginToAdd, m_vecObjPluginInstance);
    Remove_Objs(m_pWorld,m_mapObjs,pEvt,dbTimeDiff, m_vecObjPluginToRemove,m_vecObjPluginInstance);
#if 0 // :x: for test
    Add_Plugins(m_pWorld,m_mapObjs,pEvt,dbTimeDiff, 
                m_vecPluginToAdd, m_vecPluginInstance,m_vecObjPluginInstance);
    Remove_Plugins(m_pWorld,m_mapObjs,pEvt,dbTimeDiff, 
                   m_vecPluginToAdd, m_vecPluginInstance,m_vecObjPluginInstance);
#endif // :x: for test

    dbActualFPS = Frame_Rate_Control(SCREEN_FPS,dbTimeDiff);
    Set_FPS(dbActualFPS);


    // render
    SDL_RenderPresent(pRenderer);

    if (!m_bStopFlag)
      Spin_World(dbTimeDiff, m_pWorld);

    //if ( (iCnt++)%100 ==0 )
    //  printf("\033[1;33m[%s][%d] :x: FPS = %f  \033[m\n",__FUNCTION__,__LINE__,dbActualFPS);


  }
  // DeInit Objs' Plugins
  DeInit_ObjPlugins(m_pWorld,m_mapObjs,m_vecObjPluginInstance);

  return 0;
}
void CApp::Draw_Sprite(int iPixel_X, int iPixel_Y, int iIdx, 
                       float fAngle,
                       std::map<int, Sprite_t>     &mapSprites,
                       std::map<int, SDL_Texture*> &mapTextures,
                       SDL_Renderer* &pRenderer
                       ) {
  SDL_SetRenderDrawColor(pRenderer, 255, 55, 55, SDL_ALPHA_OPAQUE);
  float fScale = 1; 
  int iOffset_X= std::get<0>(m_DisplayOffSet);
  int iOffset_Y=std::get<1>(m_DisplayOffSet);;
  int iTexIdx =std::get<0>(mapSprites[iIdx]);
  SDL_Rect Rect_Sprite = std::get<1>(mapSprites[iIdx]);

  SDL_Rect DstRect = {
    (int)((float)iPixel_X*fScale+(float)iOffset_X),
    (int)((float)iPixel_Y*fScale+(float)iOffset_Y),
    (int)((float)Rect_Sprite.w*fScale),
    (int)((float)Rect_Sprite.h*fScale)};

  // draw physic engine body boundary (vector graphic)
  if (m_bVectorDraw)
    SDL_RenderCopyEx(pRenderer, m_pVectorBoxTexture, NULL,&DstRect,fAngle,NULL,SDL_FLIP_NONE );
  // draw Tile
  if (m_bTileDraw)
    SDL_RenderCopyEx(pRenderer, m_mapTextures[iTexIdx], &Rect_Sprite,&DstRect,fAngle,NULL,SDL_FLIP_NONE );
}

void CApp::Draw_Sprite(int iPixel_X, int iPixel_Y, int iIdx,float fAngle) {
  return Draw_Sprite(iPixel_X, iPixel_Y, iIdx,
      fAngle,m_mapSprites,m_mapTextures, m_pRenderer);
}
void CApp::Draw_Sprite(int iPixel_X, int iPixel_Y, int iIdx) {
  return Draw_Sprite(iPixel_X,iPixel_Y, iIdx,
      0.0f);
};

int CApp::Draw_Line_Pixel(float fPixelA_X,float fPixelA_Y,
                          float fPixelB_X,float fPixelB_Y,
                          SDL_Renderer* & pRenderer )
{
  float fAX = fPixelA_X+std::get<0>(m_DisplayOffSet);
  float fAY = fPixelA_Y+std::get<1>(m_DisplayOffSet);
  float fBX = fPixelB_X+std::get<0>(m_DisplayOffSet);
  float fBY = fPixelB_Y+std::get<1>(m_DisplayOffSet);
  SDL_SetRenderDrawColor(pRenderer,0, 255, 0, SDL_ALPHA_OPAQUE);
  return SDL_RenderDrawLine(pRenderer, fAX, fAY, fBX, fBY);
}
int CApp::Draw_Line_Pixel(float fPixelA_X,float fPixelA_Y,
                          float fPixelB_X,float fPixelB_Y)
{
  return Draw_Line_Pixel(
                        fPixelA_X, fPixelA_Y, fPixelB_X, fPixelB_Y,m_pRenderer);
}

int CApp::Draw_Line_Scale(float fAX_M, float fAY_M, 
                          float fBX_M, float fBY_M, 
                          CPhysic_World *pWorld, SDL_Renderer *pRenderer) {
  float fScale_Pixel_per_Meter = pWorld->m_fScale_Pixel_per_Meter;
  float fPixelA_X =  (int)((fAX_M) *fScale_Pixel_per_Meter) +(SCREEN_WIDTH/2);
  float fPixelA_Y = -(int)((fAY_M) *fScale_Pixel_per_Meter) +(SCREEN_HEIGHT/2);

  float fPixelB_X =  (int)((fBX_M) *fScale_Pixel_per_Meter) +(SCREEN_WIDTH/2);
  float fPixelB_Y = -(int)((fBY_M) *fScale_Pixel_per_Meter) +(SCREEN_HEIGHT/2);

  Draw_Line_Pixel(fPixelA_X,fPixelA_Y,fPixelB_X,fPixelB_Y,pRenderer);

  return 0;
}

int CApp::Draw_Line_Scale(float fPixelA_X, float fPixelA_Y, 
                          float fPixelB_X, float fPixelB_Y) {
  return Draw_Line_Scale(fPixelA_X, fPixelA_Y, fPixelB_X, fPixelB_Y, 
                         m_pWorld, m_pRenderer);

}

int CApp::Draw_Point_Pixel(float fPixel_X,float fPixel_Y,
                           SDL_Renderer* & pRenderer) {
  float fX = fPixel_X+std::get<0>(m_DisplayOffSet);
  float fY = fPixel_Y+std::get<1>(m_DisplayOffSet);
  SDL_SetRenderDrawColor(m_pRenderer,255, 255, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderDrawPoint(pRenderer,fX,   fY);
  SDL_RenderDrawPoint(pRenderer,fX-1, fY);
  SDL_RenderDrawPoint(pRenderer,fX+1, fY);
  SDL_RenderDrawPoint(pRenderer,fX,   fY-1);
  SDL_RenderDrawPoint(pRenderer,fX,   fY+1);
  return 0;
}

int CApp::Draw_Point_Pixel(float fPixel_X,float fPixel_Y) {
  return Draw_Point_Pixel(fPixel_X,fPixel_Y,m_pRenderer);
}
int CApp::Draw_Point_Scale(float fX_M,float fY_M,CPhysic_World* pWorld,
                           SDL_Renderer* pRenderer) {
  float fScale_Pixel_per_Meter = pWorld->m_fScale_Pixel_per_Meter;
  float fPixel_X =  (int)((fX_M) *fScale_Pixel_per_Meter) +(SCREEN_WIDTH/2);
  float fPixel_Y = -(int)((fY_M) *fScale_Pixel_per_Meter) +(SCREEN_HEIGHT/2);

  return Draw_Point_Pixel(fPixel_X,fPixel_Y,pRenderer);
}

int CApp::Draw_Point_Scale(float fX_M,float fY_M) {
  return Draw_Point_Scale(fX_M,fY_M,m_pWorld,m_pRenderer);
}


int CApp::Create_World(TMX_Ctx &TMX_context,
                       std::map<std::string,ObjAttr*> &mapObjs) 
{
  m_mapObjs.clear();
  if (m_pWorld) {
    delete m_pWorld;
  }
  m_pWorld = new CPhysic_World(TMX_context,mapObjs);

  // Register Plugins
  Register_Plugins();

  return 0;
}

int CApp::Draw_World(CPhysic_World* pWorld,SDL_Renderer* pRenderer) {


  float fScale_Pixel_per_Meter = pWorld->m_fScale_Pixel_per_Meter;

  SDL_SetRenderTarget(m_pRenderer, m_mapDrawingTextures[0]);
  SDL_SetRenderDrawColor(m_pRenderer, 55, 55, 55, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(m_pRenderer);
  for (auto Item : m_mapObjs) {
    ObjAttr_t& Obj = *Item.second;
    int   iIdx    = Obj.iTileIdx;
    float fX_M    = Obj.fX_M();
    float fY_M    = Obj.fY_M();
    float fWidth  = Obj.fW_M();
    float fHeight = Obj.fH_M();
    float fAngle  = Obj.fAngle();

    int iPixel_X = (int)((fX_M-fWidth/2.0f) *fScale_Pixel_per_Meter) +(SCREEN_WIDTH/2);
    int iPixel_Y = -(int)((fY_M+fHeight/2.0f) *fScale_Pixel_per_Meter)+(SCREEN_HEIGHT/2);
    // do not draw background's vector
    if (Obj.strPhysicType=="Background" && m_bTileDraw == false)
      m_bVectorDraw = false;
    Draw_Sprite(iPixel_X, iPixel_Y, iIdx, fAngle);
    if (Obj.strPhysicType=="Background" && m_bTileDraw == false)
      m_bVectorDraw = true;
  }
  return 0;
}
int CApp::Draw_World(CPhysic_World* pWorld) {
  return Draw_World(pWorld,m_pRenderer);
}
int CApp::Spin_World(double &dbTimeDiff, CPhysic_World *pWorld) {
  pWorld->SpinWorld (dbTimeDiff);
  return 0;
}

int CApp::Register_Plugins(){
  //CreatePlugins_byTagMap(m_pWorld->m_mapTags,m_vecPluginInstance);

  CreatePlugins_byObjMap(m_mapObjs,m_vecObjPluginInstance);
  //CreateGeneralPlugins(m_vecPluginInstance);
  return 0;
}

int CApp::Execute_Plugins(CPhysic_World* pWorld,
                          std::map<std::string,ObjAttr_t*> &mapObjs,
                          SDL_Event* pEvt,double dbTimeDiff,
                          std::vector<ObjAttr_t*> &vecObjPluginInstance) {
  for (auto pObj : vecObjPluginInstance) {
    CPlugin* &pPlugin = pObj->pPlugin;
    if (pPlugin->OnExecute != nullptr) {
      pPlugin->OnExecute(pWorld,mapObjs,pEvt,dbTimeDiff,pPlugin);
    }
  }

  return 0;
}


int CApp::Add_Objs(CPhysic_World* pWorld,std::map<std::string,ObjAttr_t*> &mapObjs,
    SDL_Event* pEvt,double dbTimeDiff,
    std::vector<ObjAttr_t*> &vecObjToAdd,
    std::vector<ObjAttr_t*> &vecObjPluginInstance
    ){
  if (vecObjToAdd.size() > 0) {
    for (auto pObj : vecObjToAdd) {
      CPlugin* &pPlugin = pObj->pPlugin;
      if (pPlugin->OnInit != nullptr) {
        pPlugin->OnInit(pWorld,m_mapObjs,pEvt,dbTimeDiff,pPlugin);
      }
      vecObjPluginInstance.push_back(pObj);
    }
    vecObjToAdd.clear();
  }
  return 0;
}

int CApp::Remove_Objs(CPhysic_World* pWorld,
    std::map<std::string,ObjAttr_t*> &mapObjs,
    SDL_Event* pEvt,double dbTimeDiff,
    std::vector<ObjAttr_t*> &vecObjToRemove,
    std::vector<ObjAttr_t*> &vecObjPluginInstance
    ) {
  if (vecObjToRemove.size() > 0) {
    for (auto pObj : vecObjToRemove) {
      CPlugin* &pPlugin = pObj->pPlugin;
      if (pPlugin != nullptr) {
        if (pPlugin->OnDeInit != nullptr) {
          pPlugin->OnDeInit(pWorld,mapObjs,pEvt,dbTimeDiff,pPlugin);
        }
        vecObjPluginInstance.erase( 
            std::remove_if(vecObjPluginInstance.begin(), 
              vecObjPluginInstance.end(), 
              [pObj] (ObjAttr_t* item) { return (pObj == item);}), 
            vecObjPluginInstance.end());
        printf("\033[1;31m[%s][%d] :x: chk %p \033[m\n",__FUNCTION__,__LINE__,pPlugin);

        delete pPlugin;
        pPlugin = nullptr;
      }
      mapObjs.erase(pObj->strObjName);
      //delete pObj;
      //pObj = nullptr;
    }
    vecObjToRemove.clear();
  }

  return 0;
}

int CApp::Set_vecObjToAdd(ObjAttr_t* pObj) {
  m_mtxObjPluginToAdd.lock();
  m_vecObjPluginToAdd.push_back(pObj);
  m_mtxObjPluginToAdd.unlock();
  return 0;
}

int CApp::Set_vecObjToRemove(ObjAttr_t* pObj) {
  m_mtxObjPluginToRemove.lock();
  m_vecObjPluginToRemove.push_back(pObj);
  m_mtxObjPluginToRemove.unlock();
  return 0;
}


int CApp::Init_ObjPlugins(CPhysic_World* pWorld,
                       std::map<std::string,ObjAttr_t*> &mapObjs, 
                       std::vector<ObjAttr_t*> & vecObjPluginInstance) {
  SDL_Event* pEvt = nullptr;
  double dbTimeDiff = 0;
  for (auto Obj : vecObjPluginInstance) {
    CPlugin* &pPlugin = Obj->pPlugin;
    if (pPlugin->OnDeInit != nullptr) {
      pPlugin->OnInit(pWorld,mapObjs,pEvt,dbTimeDiff,pPlugin);
    }
  }
  return 0;
}

int CApp::DeInit_ObjPlugins(CPhysic_World* pWorld,
    std::map<std::string,ObjAttr_t*> &mapObjs,
    std::vector<ObjAttr_t*> & vecObjPluginInstance) {
  SDL_Event* pEvt = nullptr;
  double dbTimeDiff = 0;
  for (auto pObj : vecObjPluginInstance) {
printf("\033[1;33m[%s][%d] :x: chk \033[m\n",__FUNCTION__,__LINE__);

    CPlugin* &pPlugin = pObj->pPlugin;
    if (pPlugin->OnDeInit != nullptr) {
      pPlugin->OnDeInit(pWorld,mapObjs,pEvt,dbTimeDiff,pPlugin);
    }
  }
  return 0;
}




double g_dbActualFPS =0.0f;
double Get_FPS() {
  return g_dbActualFPS;
}
void   Set_FPS(double dbActualFPS)
{
  g_dbActualFPS = dbActualFPS;
}



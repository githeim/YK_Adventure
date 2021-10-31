#include "CApp.h"

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
  m_pVectorBoxTexture = SDL_CreateTexture(pRenderer, 0, SDL_TEXTUREACCESS_TARGET, 10, 10);
  SDL_Rect DstRect = {0,0,10,10};
  SDL_SetRenderTarget(pRenderer, m_pVectorBoxTexture);
  SDL_SetRenderDrawColor(pRenderer, 255,105, 5, SDL_ALPHA_OPAQUE);
  SDL_RenderDrawRect(pRenderer, &DstRect);
  SDL_SetRenderTarget(pRenderer,nullptr);

  // Register Plugins
  Register_Plugins();
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
  // Clear Physic World
  if (m_pWorld) {
    delete m_pWorld;
  }

  return 0;
}

/**
 * @brief 
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
    std::string strTileSetfilepath =strTileResourcePath+item.second->strImgSourceFile;
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

int CApp::Generate_Sprites(
    TMX_Ctx &TMX_context, std::map<int, Sprite_t> &mapSprites) {
  
  for (auto tileset : TMX_context.mapTileSets) {
    int iFirstgid =tileset.first;
    int iTilecount = tileset.second->iTilecount;
    printf("\033[1;32m[%s][%d] :x: iFirstgid = %d  count %d\033[m\n",
        __FUNCTION__,__LINE__,
        iFirstgid,tileset.second->iTilecount);
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
#if 1 // :x: for test
      // if the tile has specified tile size, apply it
      std::tuple<int,int,int,int> Tile_XYWH_size;
      if (!CTMX_Reader::GetSpecifiedTileSize(TMX_context,iTileCnt,Tile_XYWH_size)) {
        iTileX +=     std::get<0>(Tile_XYWH_size);
        iTileY +=     std::get<1>(Tile_XYWH_size);
        iTileWidth  = std::get<2>(Tile_XYWH_size);
        iTileHeight = std::get<3>(Tile_XYWH_size);
      }
#endif

      mapSprites[iTileCnt] = {iFirstgid, {iTileX,iTileY,iTileWidth,iTileHeight}};
      printf("\033[1;33m[%s][%d] :x: chk %d [%s] "
          "textidx %d x= %d  y= %d, w = %d, h= %d \033[m\n",
          __FUNCTION__,__LINE__,iTileCnt,tileset.second->strName.c_str(),
          iFirstgid,
          iTileX,iTileY,iTileWidth,iTileHeight
          );
    }
  }
  return 0;
}
int CApp::OnExecute(SDL_Renderer* pRenderer) {
  bool bQuit = false;

  SDL_Event Evt;
  SDL_Event* pEvt=nullptr;

  double dbActualFPS = 0.0f;
  double dbTimeDiff;
  int iCnt=0;
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

    // render
    SDL_RenderPresent(pRenderer);


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
printf("\033[1;32m[%s][%d] :x: chk \033[m\n",__FUNCTION__,__LINE__);

            break;
          case SDLK_LEFT:
            (std::get<0>(m_DisplayOffSet))--;
            printf("\033[1;33m[%s][%d] :x: Left \033[m\n",__FUNCTION__,__LINE__);

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
    Execute_Plugins(m_pWorld,pEvt,dbTimeDiff,m_vecPlugins);
    dbActualFPS = Frame_Rate_Control(SCREEN_FPS,dbTimeDiff);

    
    if (!m_bStopFlag)
      Spin_World(dbTimeDiff, m_pWorld);

    if ( (iCnt++)%100 ==0 )
      printf("\033[1;33m[%s][%d] :x: FPS = %f  \033[m\n",__FUNCTION__,__LINE__,dbActualFPS);


  }

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
  

  //SDL_Rect DstRect = {iPixel_X,iPixel_Y,Rect_Sprite.w,Rect_Sprite.h};
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



int CApp::Create_World(TMX_Ctx &TMX_context) {
  if (m_pWorld) {
    delete m_pWorld;
  }
  m_pWorld = new CPhysic_World(TMX_context);
  return 0;
}

int CApp::Draw_World(CPhysic_World* pWorld,SDL_Renderer* pRenderer) {
  std::map<b2Body*,std::tuple<int,float,float>> &mapBodies = pWorld->m_mapBodies;

  float fScale_Pixel_per_Meter = pWorld->m_fScale_Pixel_per_Meter;

  SDL_SetRenderTarget(m_pRenderer, m_mapDrawingTextures[0]);
  SDL_SetRenderDrawColor(m_pRenderer, 55, 55, 55, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(m_pRenderer);
  for (auto body : mapBodies) {
    int iIdx = std::get<0>(body.second);
    float fWidth = std::get<1>(body.second);
    float fHeight = std::get<2>(body.second);

    b2Body* pBody = body.first;
    b2Vec2 vecPos = pBody->GetPosition();
    float fAngle = pBody->GetAngle();

    int iPixel_X = (int)((vecPos.x-fWidth/2.0f) *fScale_Pixel_per_Meter) +(SCREEN_WIDTH/2);
    int iPixel_Y = -(int)((vecPos.y+fHeight/2.0f) *fScale_Pixel_per_Meter)+(SCREEN_HEIGHT/2);
    Draw_Sprite(iPixel_X, iPixel_Y, iIdx, fAngle);
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
  // for player 1's plugin
  m_vecPlugins.push_back(Plug_Player01);
  return 0;
}
int CApp::Execute_Plugins(
                        CPhysic_World *pWorld, SDL_Event* pEvt,double dbTimeDiff,
            std::vector<std::function<int(CPhysic_World*&,SDL_Event*&,double&)>> 
                                                                  &vecPlugins) {
  for (auto plugin : vecPlugins) {
    plugin(pWorld,pEvt,dbTimeDiff);
  }

  return 0;
}

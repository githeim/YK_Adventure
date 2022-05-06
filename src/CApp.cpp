#include "CApp.h"
#include "CPlugin.h"

// Sprite draw scale
float g_fDrawingScale=1.f ;
void Set_DrawingScale(float fVal) {
  g_fDrawingScale = fVal;
}
float Get_DrawingScale() {
  return g_fDrawingScale;
}


/**
 * @brief 화면 영역 밖의 Sprite는 출력하지 않도록 하기 위해서 
 *        Sprite가 화면 영역 안에 있는지 검사하는데 사용된다
 *        Not to show the sprites that are not in the screen,
 *        This function is used to check the sprite is in the screen or not
 * @param iPixel_X[IN]
 * @param iPixel_Y[IN]
 * @param iOffset_X[IN]
 * @param iOffset_Y[IN]
 * @param iScreenWidth[IN]
 * @param iScreenHeight[IN]
 * @param fScale[IN] drawing enlargement scale 1x 1.5x 2x ...
 * @param iBorderMargin_Pixel[IN]
 *
 * @return if the sprite is not in the screen it returns false
 */
inline bool Check_Border(int iPixel_X,int iPixel_Y,
                         int iOffset_X,int iOffset_Y,
                         float fScale =1.0f,
                         int iScreenWidth=SCREEN_WIDTH,
                         int iScreenHeight=SCREEN_HEIGHT,
                         int iBorderMargin_Pixel = 24) {

  if (
    (int)((float)iPixel_X*fScale+(float)iOffset_X) < 0 - iBorderMargin_Pixel ||
    (int)((float)iPixel_X*fScale+(float)iOffset_X) > iScreenWidth + iBorderMargin_Pixel 
    ) {
    return false;
  }
  return true;
}

inline void Dbg_Print_AVG(double dbFPS) {
  static int iCnt=0;
  static double dbSum=0;
  iCnt++;
  dbSum+=dbFPS;


  float fTimeInterval = 5.f;
  static std::chrono::time_point<std::chrono::system_clock> PrevTime;
  std::chrono::duration<double> Time_diff_SEC;
  Time_diff_SEC = std::chrono::system_clock::now() - PrevTime;
  if (Time_diff_SEC.count() > fTimeInterval) {
    PrevTime =std::chrono::system_clock::now();
    double dbAvg=dbSum/iCnt;
    printf("\033[1;32m[%s][%d] :x: In %f Sec, FPS Avg =%f \033[m\n",
        __FUNCTION__,__LINE__,fTimeInterval,dbAvg);
    dbSum=0;
    iCnt=0;

  }
}
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
  
  m_ObjDirectory.Clear();
  
  // Clear Physic World
  if (m_pWorld) {
    m_pWorld->Destroy_World();
    m_pWorld = nullptr;
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

  float fScale_Pixel_per_Meter = m_pWorld->m_fScale_Pixel_per_Meter;
  double dbActualFPS = 0.0f;
  double dbTimeDiff = 0;
  // for testing & debugging screen scroll
  int iOffset_Pixel = 40;
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
  Init_ObjPlugins(m_pWorld,m_ObjDirectory);
  while (!bQuit) {

    Draw_World(fScale_Pixel_per_Meter,m_pRenderer);
    
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
        int iDisplayOffset_X,iDisplayOffset_Y;
        Get_DisplayOffSet(iDisplayOffset_X,iDisplayOffset_Y);
        switch( Evt.key.keysym.sym )
        {
          case SDLK_q:
            bQuit = true;
            break;
          case SDLK_d:
            break;
          case SDLK_LEFT:
            Set_DisplayOffSet(iDisplayOffset_X+iOffset_Pixel,iDisplayOffset_Y);
            break;
          case SDLK_RIGHT:
            Set_DisplayOffSet(iDisplayOffset_X-iOffset_Pixel,iDisplayOffset_Y);
            break;
          case SDLK_UP:
            Set_DisplayOffSet(iDisplayOffset_X,iDisplayOffset_Y+iOffset_Pixel);
            break;
          case SDLK_DOWN:
            Set_DisplayOffSet(iDisplayOffset_X,iDisplayOffset_Y-iOffset_Pixel);
            break;
          case SDLK_t:
            m_bTileDraw=!m_bTileDraw;
            m_bVectorDraw=!m_bVectorDraw;
            break;
          case SDLK_p:
            m_bStopFlag =!m_bStopFlag;
            break;
        }
      }
      else if (Evt.type == SDL_MOUSEWHEEL) {
        if(Evt.wheel.y > 0) // scroll up
        {
          // Put code for handling "scroll up" here!
          Set_DrawingScale(Get_DrawingScale()+.1f);
        }
        else if(Evt.wheel.y < 0) // scroll down
        {
          // Put code for handling "scroll down" here!
          Set_DrawingScale(Get_DrawingScale()-.1f);

        }
        printf("\033[1;33m[%s][%d] :x: Drawing Scale =%f \033[m\n",
            __FUNCTION__,__LINE__,Get_DrawingScale());
      }

      pEvt= &Evt;
    }

    Execute_Plugins(m_pWorld,m_ObjDirectory,pEvt,dbTimeDiff);
    Add_Objs(m_pWorld,m_ObjDirectory,pEvt,dbTimeDiff, m_vecObjPluginToAdd);
    Remove_Objs(m_pWorld,m_ObjDirectory,pEvt,dbTimeDiff, m_setObjPluginToRemove);

    dbActualFPS = Frame_Rate_Control(SCREEN_FPS,dbTimeDiff);
#ifdef PERF_CHK
    Dbg_Print_AVG(dbActualFPS);
#endif
    Set_FPS(dbActualFPS);


    // render
    SDL_RenderPresent(pRenderer);

    if (!m_bStopFlag)
      Spin_World(dbTimeDiff, m_pWorld);

    //if ( (iCnt++)%100 ==0 )
    //  printf("\033[1;33m[%s][%d] :x: FPS = %f  \033[m\n",__FUNCTION__,__LINE__,dbActualFPS);


  }
  // DeInit Objs' Plugins
  DeInit_ObjPlugins(m_pWorld,m_ObjDirectory);

  return 0;
}

void CApp::Draw_Sprite(int iPixel_X, int iPixel_Y,int iOffset_X,int iOffset_Y, 
                   int iIdx, float fAngle,float fScale,
                   std::map<int, Sprite_t>     &mapSprites,
                   std::map<int, SDL_Texture*> &mapTextures,
                   bool bVectorDraw,bool bTileDraw,
                   SDL_Renderer* &pRenderer) {
  SDL_SetRenderDrawColor(pRenderer, 255, 55, 55, SDL_ALPHA_OPAQUE);
  Get_DisplayOffSet(iOffset_X,iOffset_Y);
  int iTexIdx =std::get<0>(mapSprites[iIdx]);
  SDL_Rect Rect_Sprite = std::get<1>(mapSprites[iIdx]);

  SDL_Rect DstRect = {
    (int)((float)iPixel_X*fScale+(float)iOffset_X),
    (int)((float)iPixel_Y*fScale+(float)iOffset_Y),
    (int)((float)Rect_Sprite.w*fScale),
    (int)((float)Rect_Sprite.h*fScale)};

  // draw physic engine body boundary (vector graphic)
  if (bVectorDraw)
    SDL_RenderCopyEx(pRenderer, m_pVectorBoxTexture, NULL,&DstRect,fAngle,
                     NULL,SDL_FLIP_NONE );
  // draw Tile
  if (bTileDraw)
    SDL_RenderCopyEx(pRenderer, m_mapTextures[iTexIdx], &Rect_Sprite,&DstRect,
                     fAngle,NULL,SDL_FLIP_NONE );
}


void CApp::Draw_Sprite(int iPixel_X, int iPixel_Y, int iIdx, 
                       float fAngle,
                       std::map<int, Sprite_t>     &mapSprites,
                       std::map<int, SDL_Texture*> &mapTextures,
                       SDL_Renderer* &pRenderer
                       ) {
  int iOffset_X,iOffset_Y;
  Get_DisplayOffSet(iOffset_X,iOffset_Y);

  SDL_Rect Rect_Sprite = std::get<1>(mapSprites[iIdx]);

  int iTexIdx =std::get<0>(mapSprites[iIdx]);
  SDL_Rect DstRect = {
    (int)((float)iPixel_X+(float)iOffset_X),
    (int)((float)iPixel_Y+(float)iOffset_Y),
    (int)((float)Rect_Sprite.w),
    (int)((float)Rect_Sprite.h)};

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


/**
 * @brief Draw sprite by scale - x meter, y meter 
 *
 * @param iPixel_X
 * @param iPixel_Y
 * @param iIdx
 */
void CApp::Draw_Sprite_Scale(float fX_M, float fY_M, int iIdx) {

  return Draw_Sprite_Scale(fX_M,fY_M, iIdx, 0.0f);
};

void CApp::Draw_Sprite_Scale(float fX_M, float fY_M, int iIdx,float fAngle) {
  float fPixel_X,fPixel_Y;
  MeterToPixel(fX_M,fY_M,fPixel_X,fPixel_Y);
  return Draw_Sprite(fPixel_X,fPixel_Y,iIdx,fAngle);
}

int CApp::Draw_Line_Pixel(float fPixelA_X,float fPixelA_Y,
                          float fPixelB_X,float fPixelB_Y,SDL_Color& Color,
                          SDL_Renderer* & pRenderer )
{
  int iDisplayOffset_X,iDisplayOffset_Y;
  float fScale = Get_DrawingScale(); 
  Get_DisplayOffSet(iDisplayOffset_X,iDisplayOffset_Y);
  float fAX = fPixelA_X*fScale+iDisplayOffset_X;
  float fAY = fPixelA_Y*fScale+iDisplayOffset_Y;
  float fBX = fPixelB_X*fScale+iDisplayOffset_X;
  float fBY = fPixelB_Y*fScale+iDisplayOffset_Y;
  SDL_SetRenderDrawColor(pRenderer,Color.r,Color.g,Color.b,Color.a);
  return SDL_RenderDrawLine(pRenderer, fAX, fAY, fBX, fBY);
}

int CApp:: Draw_Line_Pixel(float fPixelA_X,float fPixelA_Y,
                           float fPixelB_X,float fPixelB_Y,SDL_Color& Color) {
  return Draw_Line_Pixel(
                        fPixelA_X, fPixelA_Y, fPixelB_X, fPixelB_Y,
                        Color,m_pRenderer);
}

int CApp::Draw_Line_Pixel(float fPixelA_X,float fPixelA_Y,
                          float fPixelB_X,float fPixelB_Y)
{
  SDL_Color Color = COLOR_DEFAULT;
  return Draw_Line_Pixel(fPixelA_X, fPixelA_Y, fPixelB_X, fPixelB_Y,
                         Color,m_pRenderer);
}
int CApp::Draw_Line_Scale(float fAX_M, float fAY_M, 
                          float fBX_M, float fBY_M, SDL_Color &Color) {
  return Draw_Line_Scale(fAX_M, fAY_M, 
                          fBX_M, fBY_M, Color,
                          m_pWorld, m_pRenderer);
}

int CApp::Draw_Line_Scale(float fAX_M, float fAY_M, 
                          float fBX_M, float fBY_M, SDL_Color &Color,
                          CPhysic_World *pWorld, SDL_Renderer *pRenderer) {
  float fScale_Pixel_per_Meter = pWorld->m_fScale_Pixel_per_Meter;

  float fPixelA_X,fPixelA_Y,fPixelB_X,fPixelB_Y ;
  MeterToPixel(fAX_M,fAY_M,fPixelA_X,fPixelA_Y,fScale_Pixel_per_Meter);
  MeterToPixel(fBX_M,fBY_M,fPixelB_X,fPixelB_Y,fScale_Pixel_per_Meter);
  Draw_Line_Pixel(fPixelA_X,fPixelA_Y,fPixelB_X,fPixelB_Y,Color,pRenderer);

  return 0;
}


int CApp::Draw_Line_Scale(float fAX_M, float fAY_M, 
                          float fBX_M, float fBY_M, 
                          CPhysic_World *pWorld, SDL_Renderer *pRenderer) {
  SDL_Color Color = COLOR_DEFAULT;
  Draw_Line_Scale(fAX_M,fAY_M,fBX_M,fBY_M,Color,pWorld,pRenderer);
  return 0;
}

int CApp::Draw_Line_Scale(float fPixelA_X, float fPixelA_Y, 
                          float fPixelB_X, float fPixelB_Y) {
  return Draw_Line_Scale(fPixelA_X, fPixelA_Y, fPixelB_X, fPixelB_Y, 
                         m_pWorld, m_pRenderer);
}
int  CApp::Draw_Point_Pixel(float fPixel_X,float fPixel_Y,
                            SDL_Renderer* & pRenderer,
                            SDL_Color& Color)
{
  int iDisplayOffset_X,iDisplayOffset_Y;
  Get_DisplayOffSet(iDisplayOffset_X,iDisplayOffset_Y);

  float fScale = Get_DrawingScale(); 
  float fX = fPixel_X*fScale+iDisplayOffset_X;
  float fY = fPixel_Y*fScale+iDisplayOffset_Y;
  SDL_SetRenderDrawColor(pRenderer,Color.r,Color.g,Color.b,Color.a);
  // Cross shape point
  SDL_RenderDrawPoint(pRenderer,fX,   fY);
  SDL_RenderDrawPoint(pRenderer,fX-1, fY);
  SDL_RenderDrawPoint(pRenderer,fX+1, fY);
  SDL_RenderDrawPoint(pRenderer,fX,   fY-1);
  SDL_RenderDrawPoint(pRenderer,fX,   fY+1);
  return 0;
}

int CApp::Draw_Point_Pixel(float fPixel_X,float fPixel_Y,
                           SDL_Renderer* & pRenderer) {
  SDL_Color Color = COLOR_DEFAULT;
  return Draw_Point_Pixel(fPixel_X,fPixel_Y,pRenderer,Color);
}

int CApp::Draw_Point_Pixel(float fPixel_X,float fPixel_Y) {
  return Draw_Point_Pixel(fPixel_X,fPixel_Y,m_pRenderer);
}

int CApp::Draw_Point_Scale(float fX_M,float fY_M,CPhysic_World* pWorld,
                           SDL_Color& Color,SDL_Renderer* pRenderer) {
  float fScale_Pixel_per_Meter = pWorld->m_fScale_Pixel_per_Meter;
  float fPixel_X,fPixel_Y; 
  MeterToPixel(fX_M,fY_M,fPixel_X,fPixel_Y,fScale_Pixel_per_Meter);
  return Draw_Point_Pixel(fPixel_X,fPixel_Y,pRenderer,Color);
}

int CApp::Draw_Point_Scale(float fX_M,float fY_M,CPhysic_World* pWorld,
                           SDL_Renderer* pRenderer) {
  SDL_Color Color=COLOR_DEFAULT;
  return Draw_Point_Scale(fX_M,fY_M,pWorld,Color,
                           pRenderer);
}

int CApp::Draw_Point_Scale(float fX_M,float fY_M) {
  return Draw_Point_Scale(fX_M,fY_M,m_pWorld,m_pRenderer);
}

int CApp::Draw_Point_Scale(float fX_M,float fY_M,SDL_Color &Color) {
  return Draw_Point_Scale(fX_M,fY_M,m_pWorld,Color,m_pRenderer);
}

int CApp::Create_World(TMX_Ctx &TMX_context,
                       CObjDirectory &ObjDirectory ) 
{
  ObjDirectory.Clear();
  if (m_pWorld) {
    delete m_pWorld;
  }
  m_pWorld = new CPhysic_World(TMX_context,ObjDirectory);

  // Register Plugins
  Register_Plugins();

  return 0;
}

int CApp::Draw_World(float fScale_Pixel_per_Meter,SDL_Renderer* pRenderer) {
#ifdef PERF_CHK
  static int iCallCnt=0;
  static int iLoopCnt=0;
  static int iSkipCnt=0;
  static int iLayerCnt=0;
  iLoopCnt=0;
  iSkipCnt=0;
  iCallCnt++;
  iLayerCnt=0;
#endif

  int iDisplayOffset_X,iDisplayOffset_Y;
  Get_DisplayOffSet(iDisplayOffset_X,iDisplayOffset_Y);

  SDL_SetRenderTarget(pRenderer, m_mapDrawingTextures[0]);
  SDL_SetRenderDrawColor(pRenderer, 55, 55, 55, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(pRenderer);

  float fScale = Get_DrawingScale(); 
  // Draw Layers according to m_vecLayerSequence
  for (int iLayer : m_ObjDirectory.m_vecLayerSequence) {
#ifdef PERF_CHK
    iLayerCnt++;
#endif
    for (auto pObj : m_ObjDirectory.m_mapLayerObj[iLayer]) {
      ObjAttr_t& Obj = *pObj;
      int   iIdx    = Obj.iTileIdx;
      float fX_M    = Obj.fX_M();
      float fY_M    = Obj.fY_M();
      float fWidth  = Obj.fW_M();
      float fHeight = Obj.fH_M();
      float fAngle  = Obj.fAngle();
      int iPixel_X = 
           (int)((fX_M-fWidth/2.0f) *fScale_Pixel_per_Meter) +(SCREEN_WIDTH/2);
      int iPixel_Y = 
          -(int)((fY_M+fHeight/2.0f) *fScale_Pixel_per_Meter)+(SCREEN_HEIGHT/2);
#ifdef PERF_CHK
      iLoopCnt++;
#endif
      if (Check_Border(iPixel_X,iPixel_Y,iDisplayOffset_X,iDisplayOffset_Y,fScale) 
          == false) 
      {
#ifdef PERF_CHK
        iSkipCnt++;
#endif
        // Skip drawing tiles out of screen
        continue;
      }
      // do not draw background's vector
      if (Obj.strPhysicType=="Background" && m_bTileDraw == false)
        m_bVectorDraw = false;
      Draw_Sprite(iPixel_X, iPixel_Y, iIdx, fAngle);
      if (Obj.strPhysicType=="Background" && m_bTileDraw == false)
        m_bVectorDraw = true;

    }
  }

#ifdef PERF_CHK
  if (iCallCnt%1200 == 0) 
  {
   printf("\033[1;33m[%s][%d] :x: drawing %d skip %d Loop %d skip ratio %f Layer Cnt %d\033[m\n",
       __FUNCTION__,__LINE__,iLoopCnt-iSkipCnt,iSkipCnt,iLoopCnt,(float)(iSkipCnt*100.f/iLoopCnt),iLayerCnt);
  }
#endif
  return 0;
}


int CApp::Spin_World(double &dbTimeDiff, CPhysic_World *pWorld) {
  pWorld->SpinWorld (dbTimeDiff);
  return 0;
}

int CApp::Register_Plugins(){

  CreatePlugins_byObjDir(m_ObjDirectory);
  CreateGeneralPlugins(m_ObjDirectory);
  return 0;
}

int CApp::Execute_Plugins(CPhysic_World* pWorld,
                          CObjDirectory &ObjDirectory,
                          SDL_Event* pEvt,double dbTimeDiff)
{
  std::vector<ObjAttr_t*> &vecObjPluginInstance = 
                                            ObjDirectory.m_vecObjPluginInstance;
  for (auto pObj : vecObjPluginInstance) {
    CPlugin* &pPlugin = pObj->pPlugin;
    if (pPlugin != nullptr) {
      pPlugin->OnExecute(pWorld,ObjDirectory,pEvt,dbTimeDiff,pPlugin);
    }
  }
  return 0;
}

int CApp::Add_Objs(CPhysic_World* pWorld,CObjDirectory &ObjDirectory,
    SDL_Event* pEvt,double dbTimeDiff,
    std::vector<ObjAttr_t*> &vecObjToAdd
    ){
  std::vector<ObjAttr_t*> &vecObjPluginInstance = 
                                            ObjDirectory.m_vecObjPluginInstance;

  if (vecObjToAdd.size() > 0) {
    for (auto pObj : vecObjToAdd) {
      CPlugin* &pPlugin = pObj->pPlugin;
      if (pPlugin !=nullptr) {
        if (pPlugin->OnInit != nullptr) {
          pPlugin->OnInit(pWorld,ObjDirectory,pEvt,dbTimeDiff,pPlugin);
        }
      }
      vecObjPluginInstance.push_back(pObj);
    }
    vecObjToAdd.clear();
    ObjDirectory.UpdateDirectory();
  }
  return 0;
}

/**
 * @brief Remove objects in 'setObjToRemove'
 *
 * @param pWorld
 * @param ObjDirectory
 * @param pEvt
 * @param dbTimeDiff
 * @param setObjToRemove
 * @param vecObjPluginInstance
 *
 * @return 
 */
int CApp::Remove_Objs(CPhysic_World* pWorld,
    CObjDirectory &ObjDirectory,
    SDL_Event* pEvt,double dbTimeDiff,
    std::set<ObjAttr_t*> &setObjToRemove
    ) {
  std::vector<ObjAttr_t*> &vecObjPluginInstance = 
                                            ObjDirectory.m_vecObjPluginInstance;
  std::map<std::string,ObjAttr_t*> &mapObjs = ObjDirectory.m_mapObjs;

  if (setObjToRemove.size() > 0) {
    for (auto pObj : setObjToRemove) {
      CPlugin* &pPlugin = pObj->pPlugin;
      if (pPlugin != nullptr) {
        if (pPlugin->OnDeInit != nullptr) {
          pPlugin->OnDeInit(pWorld,ObjDirectory,pEvt,dbTimeDiff,pPlugin);
        }
        vecObjPluginInstance.erase( 
            std::remove_if(vecObjPluginInstance.begin(), 
              vecObjPluginInstance.end(), 
              [pObj] (ObjAttr_t* item) { return (pObj == item);}), 
            vecObjPluginInstance.end());
        delete pPlugin;
        pPlugin = nullptr;
      }
      printf("\033[1;31m[%s][%d] :x:  Destroy Body [%s]\033[m\n",
          __FUNCTION__,__LINE__,pObj->strObjName.c_str());
      if (pObj->pBody != nullptr) {
        pWorld->m_pWorld->DestroyBody(pObj->pBody);
        pObj->pBody = nullptr;
      }
      mapObjs.erase(pObj->strObjName);
      delete pObj;
      pObj = nullptr;
    }
    setObjToRemove.clear();
    ObjDirectory.UpdateDirectory();
  }

  return 0;
}

int CApp::Set_vecObjToAdd(ObjAttr_t* pObj) {
  m_mtxObjPluginToAdd.lock();
  m_vecObjPluginToAdd.push_back(pObj);
  m_mtxObjPluginToAdd.unlock();
  return 0;
}

int CApp::Set_setObjToRemove(ObjAttr_t* pObj) {
  m_mtxObjPluginToRemove.lock();
  m_setObjPluginToRemove.insert(pObj);
  m_mtxObjPluginToRemove.unlock();
  return 0;
}
int CApp::Init_ObjPlugins(CPhysic_World* pWorld,
                       CObjDirectory& ObjDirectory
                       ) {
  std::vector<ObjAttr_t*> & vecObjPluginInstance = 
                                            ObjDirectory.m_vecObjPluginInstance;
  SDL_Event* pEvt = nullptr;
  double dbTimeDiff = 0;
  for (auto Obj : vecObjPluginInstance) {
    CPlugin* &pPlugin = Obj->pPlugin;
    if (pPlugin->OnInit != nullptr) {
      pPlugin->OnInit(pWorld,ObjDirectory,pEvt,dbTimeDiff,pPlugin);
    }
  }
  return 0;
}

int CApp::DeInit_ObjPlugins(CPhysic_World* pWorld,
    CObjDirectory &ObjDirectory
    ){
  std::vector<ObjAttr_t*> &vecObjPluginInstance = 
                                            ObjDirectory.m_vecObjPluginInstance;
  SDL_Event* pEvt = nullptr;
  double dbTimeDiff = 0;
  
  for (auto pObj : vecObjPluginInstance) {
    CPlugin* &pPlugin = pObj->pPlugin;
    if (pPlugin) {
      if (pPlugin->OnDeInit != nullptr) {
        pPlugin->OnDeInit(pWorld,ObjDirectory,pEvt,dbTimeDiff,pPlugin);
      }
      delete pPlugin;
      pPlugin= nullptr;
    }
  }
  return 0;
}

void CApp::Set_DisplayOffSet(int iX_Pixel, int iY_Pixel) {
  m_DisplayOffSet.x=iX_Pixel;
  m_DisplayOffSet.y=iY_Pixel;

  return;
}
void CApp::Get_DisplayOffSet(int &iX_Pixel, int &iY_Pixel) {
  iX_Pixel=m_DisplayOffSet.x;
  iY_Pixel=m_DisplayOffSet.y;
  return;
}



double g_dbActualFPS =0.0f;
double Get_FPS() {
  return g_dbActualFPS;
}
void   Set_FPS(double dbActualFPS)
{
  g_dbActualFPS = dbActualFPS;
}



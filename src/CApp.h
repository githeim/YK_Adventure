#ifndef _CAPP_H_
#define _CAPP_H_ 

#include <tuple>
#include <mutex>
#include <stdio.h>
#include <functional>
#include <set>
#include "SDL2_Ctx.h"
#include "CPhysic_World.h"
#include "Plugins.h"
#include "CPlugin.h"
#include "ObjAttr.h"

// For debugging, performance checker flag
//#define PERF_CHK

void Set_DrawingScale(float fVal);
float Get_DrawingScale(); 

typedef struct ScreenOffset {
  int x;
  int y;
} Offset_T;


inline float MeterToPixel_X(float &fX_M,
                       float fScale_Pixel_per_Meter =WORLDSCALE_PIXEL_PER_METER,
                       int iScreenWidth =SCREEN_WIDTH) {
  return (float)((fX_M) *fScale_Pixel_per_Meter) +(SCREEN_WIDTH/2);
}
inline float MeterToPixel_Y(float &fY_M,
                       float fScale_Pixel_per_Meter =WORLDSCALE_PIXEL_PER_METER,
                       int iScreenWidth =SCREEN_WIDTH) {
  return -(float)((fY_M) *fScale_Pixel_per_Meter) +(SCREEN_HEIGHT/2);
}

inline void MeterToPixel(float &fX_M,float &fY_M,float &fPixel_X,float &fPixel_Y,
             float fScale_Pixel_per_Meter =WORLDSCALE_PIXEL_PER_METER,
             int iScreenWidth =SCREEN_WIDTH) 
{
  fPixel_X =MeterToPixel_X(fX_M,fScale_Pixel_per_Meter,iScreenWidth);
  fPixel_Y =MeterToPixel_Y(fY_M,fScale_Pixel_per_Meter,iScreenWidth);  
  return;
}

inline b2Vec2 PixelToMeter(int iPixel_X, int iPixel_Y,
                 float fScale_Pixel_per_Meter=WORLDSCALE_PIXEL_PER_METER,
                 int iScreenWidth =SCREEN_WIDTH ) 
{
  float fX_M =  (iPixel_X-(float)(SCREEN_WIDTH/2.f)) / fScale_Pixel_per_Meter;
  float fY_M = -(iPixel_Y -(SCREEN_HEIGHT/2))        / fScale_Pixel_per_Meter;
  return b2Vec2(fX_M,fY_M);
}


// Sprite type (texture index --> m_mapTextures, sprite area)
typedef std::tuple<int,SDL_Rect> Sprite_t;

#define MAX_SCREENS (4)
class CApp {
public:
  CApp() {
    printf("\033[1;33m[%s][%d] :x: chk \033[m\n",__FUNCTION__,__LINE__);
    if ( Init( m_pWindow, m_pRenderer, m_pFont) ) {
      printf("\033[1;31m[%s][%d] :x: Err on Init \033[m\n",
          __FUNCTION__,__LINE__);
    }
  };
  ~CApp() {
    printf("\033[1;33m[%s][%d] :x: chk \033[m\n",__FUNCTION__,__LINE__);
    if ( DeInit(m_pWindow, m_pRenderer, m_pFont) )
      printf("\033[1;31m[%s][%d] :x: Err on Init \033[m\n",
          __FUNCTION__,__LINE__);
  };
  
  int Init(  SDL_Window*   &pWindow, 
             SDL_Renderer* &pRenderer,
             TTF_Font*     &pFont ); 

  int DeInit( SDL_Window*   &pWindow, 
              SDL_Renderer* &pRenderer,
              TTF_Font*     &pFont ); 
  
  int OnExecute(SDL_Renderer* pRenderer);
  int OnExecute() {
    return OnExecute(m_pRenderer);
  }

  // Register textures that are used by sprites
  int Register_Textures(TMX_Ctx                     & TMX_context,
                        std::string                 & strTileResourcePath,
                        SDL_Renderer*               & pRenderer ,
                        std::map<int,SDL_Texture*>  & mapTexture
                         );
  int Register_Textures(TMX_Ctx &TMX_context,std::string &strTileResourcePath,
                        SDL_Renderer* &pRenderer ) { 
    return Register_Textures(TMX_context, strTileResourcePath,
                             pRenderer,m_mapTextures);
  };
  int Register_Textures(TMX_Ctx &TMX_context, std::string &strTileResourcePath) { 
    return Register_Textures(TMX_context,strTileResourcePath,m_pRenderer);
  };

  int Draw_World(float fScale_Pixel_per_Meter,SDL_Renderer* pRenderer);

  int Generate_Sprites(TMX_Ctx &TMX_context,
                       std::map<int, Sprite_t> &mapSprites);

  void Draw_Sprite(int iPixel_X, int iPixel_Y,int iOffset_X,int iOffset_Y, 
                   int iIdx, float fAngle,float fScale,
                   std::map<int, Sprite_t>     &mapSprites,
                   std::map<int, SDL_Texture*> &mapTextures,
                   bool bVectorDraw,bool bTileDraw,
                   SDL_Renderer* &pRenderer);

  void Draw_Sprite(int iPixel_X, int iPixel_Y, int iIdx, float fAngle,
                       std::map<int, Sprite_t>     &mapSprites,
                       std::map<int, SDL_Texture*> &mapTextures,
                       SDL_Renderer* &pRenderer);
  void Draw_Sprite(int iPixel_X, int iPixel_Y, int iIdx,float fAngle); 
  void Draw_Sprite_Scale(float fX_M, float fY_M, int iIdx, float fAngle); 
  void Draw_Sprite_Scale(float fX_M, float fY_M, int iIdx); 
  void Draw_Sprite(int iPixel_X, int iPixel_Y, int iIdx);
  int  Draw_Point_Pixel(float fPixel_X,float fPixel_Y,SDL_Renderer* & pRenderer,
                        SDL_Color& Color);
  int  Draw_Point_Pixel(float fPixel_X,float fPixel_Y,SDL_Renderer* & pRenderer);
  int  Draw_Point_Pixel(float fPixel_X,float fPixel_Y);
  int  Draw_Point_Scale(float fX_M,float fY_M,CPhysic_World* pWorld,
                        SDL_Color& Color,SDL_Renderer* pRenderer);
  int  Draw_Point_Scale(float fX_M,float fY_M,CPhysic_World* pWorld,
                        SDL_Renderer* pRenderer);
  int  Draw_Point_Scale(float fX_M,float fY_M,SDL_Color& Color);
  int  Draw_Point_Scale(float fX_M,float fY_M);

  int  Draw_Line_Pixel(float fPixelA_X,float fPixelA_Y,
                       float fPixelB_X,float fPixelB_Y);
  int  Draw_Line_Pixel(float fPixelA_X,float fPixelA_Y,
                       float fPixelB_X,float fPixelB_Y,SDL_Color& Color);
  int  Draw_Line_Pixel(float fPixelA_X,float fPixelA_Y,
                       float fPixelB_X,float fPixelB_Y,SDL_Color& Color,
                       SDL_Renderer* & pRenderer );


  int  Draw_Line_Scale(float fPixelA_X,float fPixelA_Y,
                       float fPixelB_X,float fPixelB_Y);
  int  Draw_Line_Scale(float fPixelA_X,float fPixelA_Y,
                       float fPixelB_X,float fPixelB_Y, 
                       CPhysic_World* pWorld, SDL_Renderer* pRenderer);
  int  Draw_Line_Scale(float fPixelA_X,float fPixelA_Y,
                       float fPixelB_X,float fPixelB_Y, SDL_Color& Color,
                       CPhysic_World* pWorld, SDL_Renderer* pRenderer);
  int Draw_Line_Scale(float fAX_M, float fAY_M, 
                          float fBX_M, float fBY_M, SDL_Color &Color);

  int Create_World(TMX_Ctx &TMX_context,
      CObjDirectory &ObjDirectory ) ;

  int Spin_World(double &dbTimeDiff,CPhysic_World* pWorld);


  int Execute_Plugins(CPhysic_World* pWorld,
                      CObjDirectory &ObjDirectory,
                      SDL_Event* pEvt,double dbTimeDiff
                      );

  int Remove_Objs(CPhysic_World* pWorld,
                     CObjDirectory &ObjDirectory,
                     SDL_Event* pEvt,double dbTimeDiff,
                     std::set<ObjAttr_t*> &setObjToRemove
      );

  int Add_Objs(CPhysic_World* pWorld,
                  CObjDirectory &ObjDirectory,
                  SDL_Event* pEvt,double dbTimeDiff,
                  std::vector<ObjAttr_t*> &vecObjToAdd
                  );

  int Init_ObjPlugins(CPhysic_World* pWorld,CObjDirectory &ObjDirectory);

  int DeInit_ObjPlugins(CPhysic_World* pWorld,
                        CObjDirectory &ObjDirectory
                        );


  int Set_vecObjToAdd(ObjAttr_t* pObj); 
  int Set_setObjToRemove(ObjAttr_t* pObj); 


  int Register_Plugins();
  // Plugin Instances
  std::vector<CPlugin*> m_vecPluginToAdd;
  std::vector<ObjAttr_t*> m_vecObjPluginToAdd;

  std::vector<CPlugin*> m_vecPluginToRemove;
  std::set<ObjAttr_t*> m_setObjPluginToRemove;

  
  std::mutex m_mtxObjPluginToAdd;
  std::mutex m_mtxObjPluginToRemove;


  SDL_Window*   m_pWindow   = NULL;
  SDL_Renderer* m_pRenderer = NULL;
  TTF_Font*     m_pFont     = NULL;

  std::map<int, Sprite_t>     m_mapSprites;
  
  // Textures for tile sets (tile idx, SDL_Texture*)
  std::map<int, SDL_Texture*> m_mapTextures;

  // Textures for drawing Screens (player idx, SDL_Texture*)
  std::map<int, SDL_Texture*> m_mapDrawingTextures;

  // Vector box texture to draw vector graphic
  SDL_Texture* m_pVectorBoxTexture;

  CPhysic_World* m_pWorld;


  // Display Offset
  Offset_T m_DisplayOffSet ={0,0};


  void Set_DisplayOffSet(int iX_Pixel,int iY_Pixel);
  void Get_DisplayOffSet(int &iX_Pixel,int &iY_Pixel);

  // tile draw flag
  bool m_bTileDraw = true;
  // vector draw flag - draw physic vector for debugging
  bool m_bVectorDraw = false;
  // stop flag
  bool m_bStopFlag = false;

  // Objects in the App
  CObjDirectory m_ObjDirectory;
  SDL_Joystick* m_joy;
};


double Get_FPS();
void   Set_FPS(double dbActualFPS);
#endif /* ifndef _CAPP_H_ */

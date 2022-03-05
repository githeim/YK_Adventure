#include <stdio.h>
#include "libmodule.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <string>

#include <chrono>

#include <unistd.h>

#include <memory>

// for box2d
#include "box2d/box2d.h"

#include "Physic_world.h"
#include "SDL2_Ctx.h"

#include "TMX_Reader/CTMX_Reader.h"
#include "CApp.h"
std::shared_ptr<CApp> g_pApp;
std::shared_ptr<CApp> Get_pApp() {
  return g_pApp;
}
int Dbg_DrawPoint_Scale(float fX_M,float fY_M,SDL_Color &Color) {
  return g_pApp->Draw_Point_Scale(fX_M,fY_M,Color);
}

int Dbg_DrawPoint_Scale(float fX_M,float fY_M) {
  return g_pApp->Draw_Point_Scale(fX_M,fY_M);
}

int Dbg_DrawLine_Scale(float fAX_M,float fAY_M,
                       float fBX_M,float fBY_M,SDL_Color& Color
    ) {
  return g_pApp->Draw_Line_Scale(fAX_M,fAY_M,fBX_M,fBY_M,Color);
}


int Dbg_DrawLine_Scale(float fAX_M,float fAY_M,
                       float fBX_M,float fBY_M
    ) {
  SDL_Color Color = COLOR_DEFAULT;
  return Dbg_DrawLine_Scale(fAX_M,fAY_M,fBX_M,fBY_M,Color);
}


std::shared_ptr<TMX_Ctx> g_pTMX_Ctx;
void Set_pTMX_Ctx(std::shared_ptr<TMX_Ctx> pTMX_Ctx) {
  g_pTMX_Ctx = pTMX_Ctx;
}
std::shared_ptr<TMX_Ctx> Get_pTMX_Ctx() {
  return g_pTMX_Ctx;
}

int main(int argc, char *argv[]) {
  printf("Project YK Adventure \n");

  std::string strTileResourcePath="resource/tiles/";
  CTMX_Reader TMX_Reader;
  std::shared_ptr<TMX_Ctx> pTMX_Ctx = std::make_shared<TMX_Ctx>();
//  TMX_Reader.Read_TMX_Ctx("stage00.tmx",*pTMX_Ctx,strTileResourcePath);
//  640x32 ; FPS 50
  TMX_Reader.Read_TMX_Ctx("template_stage_640_32.tmx",*pTMX_Ctx,strTileResourcePath);
//  320x32 ; FPS 100
//  TMX_Reader.Read_TMX_Ctx("template_stage_320_32.tmx",*pTMX_Ctx,strTileResourcePath);
//  160x32 ; FPS 200
//  TMX_Reader.Read_TMX_Ctx("template_stage_160_32.tmx",*pTMX_Ctx,strTileResourcePath);
//  80x32 ; FPS  350
//  TMX_Reader.Read_TMX_Ctx("template_stage_80_32.tmx",*pTMX_Ctx,strTileResourcePath);
//  70x32 ; FPS  380
//  TMX_Reader.Read_TMX_Ctx("template_stage_70_32.tmx",*pTMX_Ctx,strTileResourcePath);
  Set_pTMX_Ctx (pTMX_Ctx);

  std::shared_ptr<CApp> pApp = std::make_shared<CApp>();
  g_pApp = pApp;
  pApp->Register_Textures(*pTMX_Ctx,strTileResourcePath);
  pApp->Generate_Sprites(*pTMX_Ctx,pApp->m_mapSprites);
  
  //pApp->Create_World(*pTMX_Ctx,pApp->m_mapObjs);
  pApp->Create_World(*pTMX_Ctx,pApp->m_ObjDirectory);
  pApp->OnExecute();

  return 0;
}

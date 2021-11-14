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
int Dbg_DrawPoint_Scale(float fX_M,float fY_M) {
  return g_pApp->Draw_Point_Scale(fX_M,fY_M);
}
int Dbg_DrawLine_Scale(float fAX_M,float fAY_M,
                       float fBX_M,float fBY_M
    ) {
  return g_pApp->Draw_Line_Scale(fAX_M,fAY_M,fBX_M,fBY_M);
}


int main(int argc, char *argv[]) {
  printf("Project YK Adventure \n");

  std::string strTileResourcePath="resource/tiles/";
  CTMX_Reader TMX_Reader;
  std::shared_ptr<TMX_Ctx> pTMX_Ctx = std::make_shared<TMX_Ctx>();
  TMX_Reader.Read_TMX_Ctx("stage00.tmx",*pTMX_Ctx,strTileResourcePath);


  std::shared_ptr pApp = std::make_shared<CApp>();
  g_pApp = pApp;
  pApp->Register_Textures(*pTMX_Ctx,strTileResourcePath);
  pApp->Generate_Sprites(*pTMX_Ctx,pApp->m_mapSprites);
  
  pApp->Create_World(*pTMX_Ctx);

 
  pApp->OnExecute();

  return 0;
}

#ifndef _PLUGINS_H_
#define _PLUGINS_H_ 
#include "CPhysic_World.h"
#include <SDL2/SDL.h>
#include "CPlugin.h"
#include "CApp.h"
int Draw_Energy_Bar(float fEnergyRate, // 0.0~1.0 the rate of the energy
                    b2Vec2 &vec2CurPos,b2Vec2 &vec2Offset,float fBarSizeWidth_M,
                    float fBarSizeHeight_Pixel =5,
                    float fScale_Pixel_per_Meter = WORLDSCALE_PIXEL_PER_METER
                    );


int Dbg_DrawPoint_Scale(float fX_M,float fY_M); 
int Dbg_DrawPoint_Scale(float fX_M,float fY_M,SDL_Color &Color); 
int Dbg_DrawLine_Scale(float fAX_M,float fAY_M,
                       float fBX_M,float fBY_M
    );
int Dbg_DrawLine_Scale(float fAX_M,float fAY_M,
                       float fBX_M,float fBY_M,SDL_Color &Color
    );
bool Chk_Tag(CObjDirectory &ObjDirectory,
    std::vector<std::string> vecTargetTags,b2Body* pBody,
    ObjAttr_t* &pContactedObj
    );


int Plug_Player01_Init(CPhysic_World* &pWorld,
                  CObjDirectory &ObjDirectory,
                  SDL_Event* &pEvt,double& dbTimeDiff,
                  CPlugin* pInstance);
int Plug_Player02_Init(CPhysic_World* &pWorld,
                  CObjDirectory &ObjDirectory,
                  SDL_Event* &pEvt,double& dbTimeDiff,
                  CPlugin* pInstance);


int Plug_Player01(CPhysic_World* &pWorld,
                  CObjDirectory &ObjDirectory,
                  SDL_Event* &pEvt,double& dbTimeDiff,
                  CPlugin* pInstance);
int Plug_Enemy_Ground_Tracker_Init(
                    CPhysic_World* &pWorld,
                    CObjDirectory &ObjDirectory, SDL_Event* &pEvt,
                    double& dbTimeDiff, CPlugin* pInstance);
int Plug_Enemy_Ground_Tracker_DeInit(
                    CPhysic_World* &pWorld,
                    CObjDirectory &ObjDirectory, SDL_Event* &pEvt,
                    double& dbTimeDiff, CPlugin* pInstance);
int Plug_Enemy_Ground_Tracker(CPhysic_World* &pWorld,
                              CObjDirectory &ObjDirectory,
                              SDL_Event* &pEvt,double& dbTimeDiff,
                              CPlugin* pInstance);
int Plug_Enemy_Flyer_Init(CPhysic_World* &pWorld,
                          CObjDirectory &ObjDirectory, 
                          SDL_Event* &pEvt,
                          double& dbTimeDiff, CPlugin* pInstance);
int Plug_Enemy_Flyer_DeInit(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory, 
                              SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance);
int Plug_Enemy_Flyer(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory, 
                              SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance);

int Plug_FPS_Drawer_Init(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory, 
                              SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance);
int Plug_FPS_Drawer_DeInit(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory, 
                              SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance);
int Plug_FPS_Drawer(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory, 
                              SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance);
int Plug_Spawner(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory, 
                              SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance); 
int Plug_Scroll_Init(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory, 
                              SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance);
int Plug_Scroll(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory, 
                              SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance);
int Plug_WelcomeMsg_Init(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory, 
                              SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance);
int Plug_WelcomeMsg_DeInit(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory, 
                              SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance);
int Plug_WelcomeMsg(CPhysic_World* &pWorld,CObjDirectory &ObjDirectory, 
                              SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance);




#endif /* ifndef _PLUGINS_H_ */

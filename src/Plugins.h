#ifndef _PLUGINS_H_
#define _PLUGINS_H_ 
#include "CPhysic_World.h"
#include <SDL2/SDL.h>
#include "CPlugin.h"
#include "CApp.h"

int Dbg_DrawPoint_Scale(float fX_M,float fY_M); 
int Dbg_DrawLine_Scale(float fAX_M,float fAY_M,
                       float fBX_M,float fBY_M
    );

int Plug_Player01(CPhysic_World* &pWorld,
                  std::map<std::string,ObjAttr_t*> &mapObjs,
                  SDL_Event* &pEvt,double& dbTimeDiff,
                  CPlugin* pInstance);
int Plug_Enemy_Ground_Tracker_Init(
                    CPhysic_World* &pWorld,
                    std::map<std::string,ObjAttr_t*> &mapObjs, SDL_Event* &pEvt,
                    double& dbTimeDiff, CPlugin* pInstance);
int Plug_Enemy_Ground_Tracker_DeInit(
                    CPhysic_World* &pWorld,
                    std::map<std::string,ObjAttr_t*> &mapObjs, SDL_Event* &pEvt,
                    double& dbTimeDiff, CPlugin* pInstance);
int Plug_Enemy_Ground_Tracker(CPhysic_World* &pWorld,
                              std::map<std::string,ObjAttr_t*> &mapObjs,
                              SDL_Event* &pEvt,double& dbTimeDiff,
                              CPlugin* pInstance);
int Plug_Enemy_Flyer_Init(CPhysic_World* &pWorld,
                          std::map<std::string,ObjAttr_t*> &mapObjs, 
                          SDL_Event* &pEvt,
                          double& dbTimeDiff, CPlugin* pInstance);
int Plug_Enemy_Flyer_DeInit(CPhysic_World* &pWorld,std::map<std::string,ObjAttr_t*> &mapObjs, SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance);
int Plug_Enemy_Flyer(CPhysic_World* &pWorld,std::map<std::string,ObjAttr_t*> &mapObjs, SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance);

int Plug_FPS_Drawer_Init(CPhysic_World* &pWorld,std::map<std::string,ObjAttr_t*> &mapObjs, SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance);
int Plug_FPS_Drawer_DeInit(CPhysic_World* &pWorld,std::map<std::string,ObjAttr_t*> &mapObjs, SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance);
int Plug_FPS_Drawer(CPhysic_World* &pWorld,std::map<std::string,ObjAttr_t*> &mapObjs, SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance);
int Plug_Spawner(CPhysic_World* &pWorld,std::map<std::string,ObjAttr_t*> &mapObjs, SDL_Event* &pEvt,
                              double& dbTimeDiff, CPlugin* pInstance); 


#endif /* ifndef _PLUGINS_H_ */

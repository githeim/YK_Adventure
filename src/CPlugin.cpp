#include "CPlugin.h"
#include "Plugins.h"


/**
 * @brief Create Plugin instances & add created plugins to ObjDirectory variable
 *
 * @param mapObjs[IN]
 * @param vecObjPluginInstance[OUT]
 *
 * @return 
 */

int CreatePlugins_byObjDir(CObjDirectory &ObjDirectory)
{
  std::map<std::string,ObjAttr_t*> &mapObjs = ObjDirectory.m_mapObjs;
  std::vector<ObjAttr_t*> &vecObjPluginInstance = 
                                            ObjDirectory.m_vecObjPluginInstance;

  for (auto Item : mapObjs) {
    const std::string &strObjName = Item.first;
    ObjAttr_t* &pObj = Item.second;
    if (pObj->vecTag.size() ==0) 
      continue;
    CPlugin* pInstance = nullptr;

    for (auto strTag : pObj->vecTag) {
      printf("\033[1;33m[%s][%d] :x: Tag %s \033[m\n",__FUNCTION__,__LINE__,strTag.c_str());

      if (strTag == "Player01") {
        pInstance = new CPlugin(strObjName);
        pInstance->m_pBody = pObj->pBody;
        printf("\033[1;33m[%s][%d] :x: player1 instance [%s]\033[m\n",
            __FUNCTION__,__LINE__,strObjName.c_str());
        pInstance->OnExecute = Plug_Player01;
        pInstance->OnInit = Plug_Player01_Init;
      } else if (strTag == "Player02") {
        pInstance = new CPlugin(strObjName);
        pInstance->m_pBody = pObj->pBody;
        printf("\033[1;33m[%s][%d] :x: player2 instance [%s]\033[m\n",
            __FUNCTION__,__LINE__,strObjName.c_str());
        pInstance->OnExecute = Plug_Player01;
        pInstance->OnInit = Plug_Player02_Init;
      }else if (strTag == "Enemy_Ground_Tracker") {
        pInstance = new CPlugin(strObjName);
        pInstance->m_pBody = pObj->pBody;
        printf("\033[1;33m[%s][%d] :x: Enemy_Ground_Tracker instance [%s]\033[m\n",
            __FUNCTION__,__LINE__,strObjName.c_str());
        pInstance->OnExecute = Plug_Enemy_Ground_Tracker;
        pInstance->OnInit = Plug_Enemy_Ground_Tracker_Init;
        pInstance->OnDeInit = Plug_Enemy_Ground_Tracker_DeInit;

      } else if (strTag == "Enemy_Flyer") {
        pInstance = new CPlugin(strObjName);
        pInstance->m_pBody = pObj->pBody;
        printf("\033[1;33m[%s][%d] :x: Enemy_Ground_Tracker instance [%s]\033[m\n",
            __FUNCTION__,__LINE__,strObjName.c_str());
        pInstance->OnExecute = Plug_Enemy_Flyer;
        pInstance->OnInit = Plug_Enemy_Flyer_Init;
        pInstance->OnDeInit = Plug_Enemy_Flyer_DeInit;
      }
      else if (strTag == "Enemy_Spawner") {
        pInstance = new CPlugin(strObjName);
        pInstance->m_pBody = pObj->pBody;
        printf("\033[1;33m[%s][%d] :x: Enemy_Spawner instance [%s]\033[m\n",
            __FUNCTION__,__LINE__,strObjName.c_str());
        pInstance->OnExecute = Plug_Spawner;
      }
    }

    if (pInstance != nullptr) {
      mapObjs[strObjName]->pPlugin=pInstance;
      vecObjPluginInstance.push_back(mapObjs[strObjName]);
    }

  }
  return 0;
}

int CreateGeneralPlugin(ObjAttr_t* &pInstance,
  std::function<int(CPhysic_World* &,CObjDirectory &,SDL_Event* &,double &, CPlugin* )> OnExecute,
  // Used when it created
  std::function<int(CPhysic_World* &,CObjDirectory &,SDL_Event* &,double &, CPlugin* )> OnInit,
  // Used when it destroyed
  std::function<int(CPhysic_World* &,CObjDirectory &,SDL_Event* &,double &, CPlugin* )> OnDeInit
) {
  pInstance = new ObjAttr_t();
  auto pPlugInstance = new CPlugin();
  pPlugInstance->m_pBody   = nullptr;
  pPlugInstance->OnExecute = OnExecute;
  pPlugInstance->OnInit    = OnInit;  
  pPlugInstance->OnDeInit  = OnDeInit; 
  pInstance->pPlugin= pPlugInstance;
  return 0;
}
/**
 * @brief Create Plugin instances that has no physic bodies (= not actor)
 *
 * @param vecPluginInstance
 *
 * @return 
 */
int CreateGeneralPlugins(CObjDirectory &ObjDirectory) {
  ObjAttr_t* pInstance =nullptr;
  // Create FPS Drawer
  CreateGeneralPlugin(pInstance,
      Plug_FPS_Drawer,
      Plug_FPS_Drawer_Init,
      Plug_FPS_Drawer_DeInit
      );
  ObjDirectory.m_vecObjPluginInstance.push_back(pInstance);

  // Create scroll operator 
  CreateGeneralPlugin(pInstance,
      Plug_Scroll,
      Plug_Scroll_Init,
      nullptr
      );
  ObjDirectory.m_vecObjPluginInstance.push_back(pInstance);

  // Create welcome msg
  CreateGeneralPlugin(pInstance,
      Plug_WelcomeMsg,
      Plug_WelcomeMsg_Init,
      Plug_WelcomeMsg_DeInit
      );
  ObjDirectory.m_vecObjPluginInstance.push_back(pInstance);
  return 0;
}


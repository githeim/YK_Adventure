#include "CPlugin.h"
#include "Plugins.h"

/**
 * @brief Tag map에 따라 플러그인 인스턴스 vector를 생성한다
 *
 * @param mapTags[IN]
 * @param vecPluginInstance[OUT]
 *
 * @return 
 */
int CreatePlugins_byTagMap(std::map<std::string,std::vector<b2Body*>> &mapTags,
                  std::vector<CPlugin*> &vecPluginInstance)
{
  for (auto Item : mapTags) {
    const std::string &strTag = Item.first;
    const std::vector<b2Body*> &vecBodies = Item.second;
    for (auto pBody : vecBodies) {
      auto pInstance = new CPlugin();
      pInstance->m_pBody = pBody;

      if (strTag == "Player01") {
        printf("\033[1;33m[%s][%d] :x: player1 instance 0x%p\033[m\n",
            __FUNCTION__,__LINE__,pBody);
        pInstance->OnExecute = Plug_Player01;
      } else if (strTag == "Enemy_Ground_Tracker") {
        printf("\033[1;33m[%s][%d] :x: Enemy_Ground_Tracker instance 0x%p\033[m\n",
            __FUNCTION__,__LINE__,pBody);
        pInstance->OnExecute = Plug_Enemy_Ground_Tracker;
      } else if (strTag == "Enemy_Flyer") {
        printf("\033[1;33m[%s][%d] :x: Enemy_Ground_Tracker instance 0x%p\033[m\n",
            __FUNCTION__,__LINE__,pBody);
        pInstance->OnExecute = Plug_Enemy_Flyer;
      }
      vecPluginInstance.push_back(pInstance);

    }


  }
  return 0;
}


/**
 * @brief Create Plugin instances that has no physic bodies (= not actor)
 *
 * @param vecPluginInstance
 *
 * @return 
 */
int CreateGeneralPlugins(std::vector<CPlugin*> &vecPluginInstance) {

  // Create FPS Drawer

  auto pInstance = new CPlugin();
  pInstance->m_pBody= nullptr;
  pInstance->OnExecute= Plug_FPS_Drawer;
  vecPluginInstance.push_back(pInstance);

  return 0;
}


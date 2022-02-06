#include "ObjAttr.h"
bool CObjDirectory::Find_by_Name(std::string &strObjName, ObjAttr_t *pObj) {
  pObj = m_mapObjs[strObjName];
  return true;
}

bool CObjDirectory::Find_by_Tag(std::string &strTag, 
                                std::vector<ObjAttr_t *>& pvecObj) {
  return true;
}

bool CObjDirectory::Find_by_Body(b2Body* pBody, ObjAttr_t* pObj) {


  return true;
}



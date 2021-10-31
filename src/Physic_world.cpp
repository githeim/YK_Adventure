// for box2d
#include "box2d/box2d.h"
#include <stdio.h>
#include <tuple>

b2World* g_pWorld;


b2Body* g_pGroundBody;
b2Body* g_pBody;


void CreateWorld() {
  
  b2Vec2 gravity(0.0f,-10.0f);
  g_pWorld = new b2World(gravity);

  b2BodyDef groundBodyDef;
  groundBodyDef.position.Set(0.0f,-10.0f);

  g_pGroundBody = g_pWorld->CreateBody(&groundBodyDef);
  
  b2PolygonShape groundBox;
  groundBox.SetAsBox(20.0f, 10.0f);

  g_pGroundBody->CreateFixture(&groundBox,0.0f);

  b2BodyDef bodyDef;
  bodyDef.type = b2_dynamicBody;
  bodyDef.position.Set(0.0f,20.0f);

  bodyDef.linearVelocity.Set(3.0f, 0.0f);
  g_pBody = g_pWorld->CreateBody(&bodyDef);

  b2PolygonShape dynamicBox;
  dynamicBox.SetAsBox(1.0f, 1.0f);

  b2FixtureDef fixtureDef;
  fixtureDef.shape = &dynamicBox;
  fixtureDef.density = 1.0f;
  fixtureDef.friction = 0.3f;
  fixtureDef.restitution = 0.8f;

  g_pBody->CreateFixture(&fixtureDef);

}

void DestroyWorld() {
  g_pWorld->DestroyBody(g_pGroundBody);
  g_pWorld->DestroyBody(g_pBody);
  delete g_pWorld;
}

void SpinWorld(double dbTimeStep, float &fX,float &fY,float &fAngle, 
  int iVelocityIter=6, int iPositionIter = 2) {
  g_pWorld->Step(dbTimeStep, iVelocityIter, iPositionIter);

  b2Vec2 vec2Position = g_pBody->GetPosition();
  fAngle = g_pBody->GetAngle();
  fX = vec2Position.x;
  fY = vec2Position.y;
}

typedef struct {
  float fWidth;
  float fHeight;
} BoxShape;

typedef struct {
  int iIdx;
  int iType; // 0 : Rectangle
  void* pShape;
  float fCenter_X_M,fCenter_Y_M;
} BodyCtx;

void GetBodyDefinitions() {

}
void GetBodies() {
  // (int)idx, (int)type,area, center of massk
  std::tuple<int , int> tupleBodies;
  for (auto pBody = g_pWorld->GetBodyList(); pBody ; pBody=pBody->GetNext()){
    b2PolygonShape* pShape = (b2PolygonShape*)pBody->GetFixtureList()->GetShape();
    printf("\033[1;33m[%s][%d] :x: chk  x %f  y %f\033[m\n",
        __FUNCTION__,__LINE__,
        pShape->m_vertices[0].x,pShape->m_vertices[0].y);
    printf("\033[1;33m[%s][%d] :x: chk  count %d   \033[m\n",
        __FUNCTION__,__LINE__,
        pShape->m_count);
    for (int i =0 ; i < pShape->m_count ; i++) {
      printf("\033[1;33m[%s][%d] :x: chk  x %f  y %f\033[m\n",
          __FUNCTION__,__LINE__,
          pShape->m_vertices[i].x,pShape->m_vertices[i].y);
    }
  }
}

void ForceToLeft() {
  g_pBody->ApplyForceToCenter(b2Vec2(-100,0), true);
}
void ForceToRight() {
  g_pBody->ApplyForceToCenter(b2Vec2(100,0), true);
}
void ForceToUp() {
  g_pBody->ApplyForceToCenter(b2Vec2(0,300), true);
}
void ForceToDown() {
  g_pBody->ApplyForceToCenter(b2Vec2(0,-300), true);
}


#include "environment_templates.h"

namespace scripting
{
    
v8::Global<v8::ObjectTemplate> Templates::globalTemplate;
v8::Global<v8::ObjectTemplate> Templates::systemTemplate;
v8::Global<v8::ObjectTemplate> Templates::mathTemplate;
v8::Global<v8::ObjectTemplate> Templates::inputTemplate;
v8::Global<v8::ObjectTemplate> Templates::assetManagerTemplate;

v8::Global<v8::ObjectTemplate> Templates::meshCompTemplate;
v8::Global<v8::ObjectTemplate> Templates::lightCompTemplate;
v8::Global<v8::ObjectTemplate> Templates::dynamicBodyCompTemplate;
v8::Global<v8::ObjectTemplate> Templates::staticBodyCompTemplate;

v8::Global<v8::ObjectTemplate> Templates::collisionShapeTemplate;

v8::Global<v8::FunctionTemplate> Templates::entityTemplate;
v8::Global<v8::FunctionTemplate> Templates::componentTemplate;
v8::Global<v8::FunctionTemplate> Templates::sceneTemplate;

} // namespace scripting

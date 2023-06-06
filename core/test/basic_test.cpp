#include "scene.h"

#include <iostream>


int main()
{
    Scene* scene = Scene::NewScene();
    Entity* e0 = scene->NewEntity();
    Entity* e1 = scene->NewEntity();
    Entity* e2 = scene->NewEntity();

    e0->name = "e0";
    e1->name = "e1";
    e2->name = "e2";

    e1->ReparentTo(e0);
    e2->ReparentTo(e0);

    Entity* e3 = scene->NewEntity();
    e3->name = "e3";
    e0->ReparentTo(e3);

    Scene* scene2 = scene->Replicate(
        Scene::State::Editor);

    e3 = scene2->GetEntityByName("e3");
    std::cout << e3->name << std::endl;
    e2 = scene2->GetEntityByName("e2");
    std::cout << e2->name << std::endl;
    e1 = scene2->GetEntityByName("e1");
    std::cout << e1->name << std::endl;
    e0 = scene2->GetEntityByName("e0");
    std::cout << e0->name << std::endl;

    scene2->RemoveEntity(e3);
    scene2->SaveToFile("555.json");
    delete scene;
    scene = Scene::LoadFromFile("555.json",
        Scene::State::Editor);

    delete scene2;
}
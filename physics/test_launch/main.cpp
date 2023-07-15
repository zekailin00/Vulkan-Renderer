#include <iostream>
 
#include "physics_system.h"


int main(int, char**)
{
    physics::PhysicsSystem* system = new physics::PhysicsSystem();
    std::cout << "system size: " << sizeof(system) << std::endl;
    delete system;
    system = nullptr;
}
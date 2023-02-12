#include "application.h"
#include "renderer.h"

#include <iostream>


class TestApp: public Application
{
    void Build() override;
};

void TestApp::Build()
{
    // static int i = 0;
    // i++;
    // std::cout << "running: " << i << std::endl; 
}

int main(int argv, char** argc)
{
    TestApp app{};
    app.Run();
    std::cout << "Hello world." << std::endl;
}
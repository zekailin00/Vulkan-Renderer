#include "application.h"
#include "renderer.h"
#include "vulkan_renderer.h"

#include <iostream>


class TestApp: public Application
{
    void OnCreated() override;
    void OnUpdated() override;
    void OnDestroy() override;
};

void TestApp::OnCreated()
{

}

void TestApp::OnUpdated()
{
    // static int i = 0;
    // i++;
    // std::cout << "running: " << i << std::endl; 
}

void TestApp::OnDestroy()
{

}

int main(int argv, char** argc)
{
    TestApp app{};
    app.Run();
    std::cout << "Hello world." << std::endl;
}
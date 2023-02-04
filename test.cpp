#include "application.h"

#include <iostream>

class TestApp: public Application
{
    void Build() override;
};

void TestApp::Build()
{
    
}

int main(int argv, char** argc)
{
    TestApp app{};
    app.Run();
    std::cout << "Hello world." << std::endl;
}
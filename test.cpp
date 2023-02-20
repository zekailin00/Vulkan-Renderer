#include "application.h"

#include <iostream>

using namespace renderer;

class TestApp: public Application
{
    void OnCreated() override;
    void OnUpdated() override;
    void OnDestroy() override;



};

void TestApp::OnCreated()
{
    Node* root = renderer->CreateScene()->GetRootNode();
    std::unique_ptr<VulkanNode> node = std::make_unique<VulkanNode>();

    Node* nodePtr;
    nodePtr = root->AddChildNode(std::move(node));

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
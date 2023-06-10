#include "asset_manager.h"

#include "configuration.h"
#include "asset_manager.h"

#include <iostream>

int main(int, char**)
{
    std::cout << "hello" << std::endl;

    Configuration::Set(CONFIG_WORKSPACE_PATH,
        "C:\\Users\\zekai\\Desktop\\ws-test\\test1");

    AssetManager* manager = new AssetManager();
    manager->InitializeWorkspace();
}


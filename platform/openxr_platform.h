#pragma once

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <vector>


class OpenxrPlatform
{
public:

    static OpenxrPlatform* Initialize();

    const OpenxrPlatform& operator=(const OpenxrPlatform&) = delete;
    OpenxrPlatform(const OpenxrPlatform&) = delete;

    ~OpenxrPlatform() = default;
private:
    OpenxrPlatform() = default;

private:

    std::vector<XrApiLayerProperties> layerList{};
    std::vector<XrExtensionProperties> extensionList;

    XrInstance xrInstance;
    XrSystemId systemId;
};
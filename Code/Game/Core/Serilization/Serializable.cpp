#include "Serializable.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Core/LoggerSubsystem.hpp"

XmlElement* ISerializable::Create(const char* path)
{
    LOG(LogResource, Warning, Stringf("Loading XML File From: %s\n", path).c_str());
    XmlDocument doc;
    XmlResult   result = doc.LoadFile(path);
    if (result == XmlResult::XML_SUCCESS)
    {
        XmlElement* rootElement = doc.RootElement();
        if (rootElement)
        {
            return rootElement;
        }
        LOG(LogResource, Error, Stringf("File from \"%s\"was invalid (missing root element)\n", path).c_str());
        return rootElement;
    }
    LOG(LogResource, Error, Stringf("Fail Load Loading XML File From: \n", path).c_str());
    return nullptr;
}

bool ISerializable::Create(XmlDocument& outDoc, const char* path)
{
    LOG(LogResource, Warning, Stringf("Loading XML File From: %s\n", path).c_str());
    XmlResult result = outDoc.LoadFile(path);
    if (result != XmlResult::XML_SUCCESS)
    {
        LOG(LogResource, Error, Stringf("Failed to load XML File From: %s\n", path).c_str());
        return false;
    }

    if (!outDoc.RootElement())
    {
        LOG(LogResource, Error, Stringf("File \"%s\" was invalid (missing root element)\n", path).c_str());
        return false;
    }
    return true;
}

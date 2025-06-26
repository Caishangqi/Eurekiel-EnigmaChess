#pragma once
#include "Engine/Core/XmlUtils.hpp"

class ISerializable
{
public:
    virtual      ~ISerializable() = default;
    virtual void FromXML(const XmlElement& xmlElement) = 0;

    virtual XmlElement* ToXML() const = 0;
    static XmlElement*  Create(const char* path);
    static bool         Create(XmlDocument& outDoc, const char* path);
};

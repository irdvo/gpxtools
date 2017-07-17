// ==============================================================================
//
//                 XMLParser - the xml parser class
//
//               Copyright (C) 2017  Dick van Oudheusden
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free
// Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// ==============================================================================

#include <cstring>

#include "XMLParser.h"

XMLParser::XMLParser() :
  _parser(0)
{

}

XMLParser::~XMLParser()
{
  stopExpat();
}

bool XMLParser::parse(const char *data, size_t length, bool isFinal)
{
  if (_parser == 0) startExpat();

  if (_parser == 0) return false;

  if (XML_Parse(_parser, data, static_cast<int>(length), isFinal) != XML_STATUS_ERROR)
  {
    if (isFinal) stopExpat();

    return true;
  }
  else
  {
    parseError(std::string(XML_ErrorString(XML_GetErrorCode(_parser))),
               static_cast<int>(XML_GetCurrentLineNumber(_parser)),
               static_cast<int>(XML_GetCurrentColumnNumber(_parser)));

    if (isFinal) stopExpat();

    return false;
  }
}

bool XMLParser::parse(const char *text, bool isFinal)
{
  return parse(text, strlen(text), isFinal);
}

bool XMLParser::parse(const std::string &data, bool isFinal)
{
  return parse(data.c_str(), data.size(), isFinal);
}

bool XMLParser::parse(std::istream &stream)
{
  if (!stream.good()) return false;

  char buffer[4096];

  while (stream.good())
  {
    stream.read(buffer, sizeof(buffer));

    if (!parse(buffer, stream.gcount(), (stream.gcount() < sizeof(buffer)))) return false;
  }

  return true;
}

// Privates

void XMLParser::startExpat()
{
  _parser = XML_ParserCreate(NULL);

  XML_SetUserData(_parser, this);

  XML_SetXmlDeclHandler(_parser, xmlDeclHandler);
  XML_SetElementHandler(_parser, startElementHandler, endElementHandler);
  XML_SetCharacterDataHandler(_parser, characterDataHandler);
  XML_SetCommentHandler(_parser, commentHandler);
  XML_SetProcessingInstructionHandler(_parser, processingInstructionHandler);
  XML_SetCdataSectionHandler(_parser, startCdataSectionHandler, endCdataSectionHandler);
  XML_SetDefaultHandlerExpand(_parser, defaultHandler);
  XML_SetNamespaceDeclHandler(_parser, startNamespaceDeclHandler, endNamespaceDeclHandler);
}

void XMLParser::stopExpat()
{
  if (_parser != 0)
  {
    XML_ParserFree(_parser);

    _parser = 0;
  }
}

void XMLParser::xmlDeclHandler(void *userData, const XML_Char *version, const XML_Char *encoding, int standalone)
{

  static_cast<XMLParser*>(userData)->xmlDecl(version != 0 ? std::string(version) : std::string(),
                                             encoding != 0 ? std::string(encoding) : std::string(),
                                             standalone);
}

void XMLParser::startElementHandler(void *userData, const XML_Char *name, const XML_Char **atts)
{
  std::map<std::string, std::string> attributes;

  for (int i = 0; atts[i] != NULL; i+=2)
  {
    attributes[std::string(atts[i])] = std::string(atts[i+1]);
  }

  static_cast<XMLParser*>(userData)->startElement(std::string(name), attributes);
}

void XMLParser::endElementHandler(void *userData, const XML_Char *name)
{
  XMLParser *self = static_cast<XMLParser*>(userData);

  self->endElement(std::string(name));
}

void XMLParser::characterDataHandler(void *userData, const XML_Char *s, int len)
{
  static_cast<XMLParser*>(userData)->characterData(std::string(s, static_cast<size_t>(len)));
}

void XMLParser::commentHandler(void *userData, const XML_Char *data)
{
  static_cast<XMLParser*>(userData)->comment(std::string(data));
}

void XMLParser::processingInstructionHandler(void *userData, const XML_Char *target, const XML_Char *data)
{
  static_cast<XMLParser*>(userData)->processingInstruction(std::string(target), std::string(data));
}

void XMLParser::startCdataSectionHandler(void *userData)
{
  static_cast<XMLParser*>(userData)->startCDATASection();
}

void XMLParser::endCdataSectionHandler(void *userData)
{
  static_cast<XMLParser*>(userData)->endCDATASection();
}

void XMLParser::defaultHandler(void *userData, const XML_Char *s, int len)
{
  static_cast<XMLParser*>(userData)->unspecified(std::string(s, static_cast<size_t>(len)));
}

void XMLParser::startNamespaceDeclHandler   (void *userData, const XML_Char *prefix, const XML_Char *uri)
{
  static_cast<XMLParser*>(userData)->startNamespaceDecl(std::string(prefix), std::string(uri));
}

void XMLParser::endNamespaceDeclHandler     (void *userData, const XML_Char *prefix)
{
  static_cast<XMLParser*>(userData)->endNamespaceDecl(std::string(prefix));
}

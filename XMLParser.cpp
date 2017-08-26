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
  _handler(nullptr),
  _state(TEXT),
  _lineNumber(1),
  _columnNumber(1)
{
}

XMLParser::XMLParser(XMLParserHandler *handler) :
  _handler(handler),
  _state(TEXT),
  _lineNumber(1),
  _columnNumber(1)
{
}

XMLParser::~XMLParser()
{
}

bool XMLParser::parse(const std::string &in, bool isFinal)
{
  Result result = OK;

  std::string::size_type i = 0;
  while (i < in.size() && result == OK)
  {
    switch(_state)
    {
      case TEXT:   result = parseText  (in, i); break;
      case MARKUP: result = parseMarkup(in, i); break;
    }
  }

  if (isFinal)
  {
    if (_state == TEXT && result != FAIL)
    {
      if (_handler != nullptr && !_out.empty()) _handler->text(_out);
    }

    if (_state == MARKUP)
    {
      if (_handler != nullptr && !_out.empty()) _handler->unhandled(_out, _lineNumber, _columnNumber);
    }
  }

  return (result == OK || result == MORE);
}

bool XMLParser::parse(const char *data, size_t length, bool isFinal)
{
  return parse(std::string(data, length), isFinal);
}

bool XMLParser::parse(const char *text, bool isFinal)
{
  return parse(std::string(text), isFinal);
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

// Set the state
void XMLParser::setState(State state)
{
  _state = state;

  _out.clear();
}

// Text processing
std::string XMLParser::trim(const std::string &text)
{
  std::size_t start = text.find_first_not_of(" \t\r\n");

  if (start == std::string::npos) return std::string();

  std::size_t end   = text.find_last_not_of(" \t\r\n");

  return text.substr(start, end - start + 1);
}

void XMLParser::translateEntityRef(std::string &text, const std::string &pattern, const std::string &replace)
{
  std::size_t i = 0;

  while ((i = text.find(pattern, i)) != std::string::npos)
  {
    text.replace(i, pattern.size(), replace);
    i++;
  }
}

std::string XMLParser::translateEntityRefs(const std::string &text)
{
  std::string result = text;

  translateEntityRef(result, "&lt;",   "<");
  translateEntityRef(result, "&gt;",   ">");
  translateEntityRef(result, "&apos;", "'");
  translateEntityRef(result, "&quot;", "\"");
  translateEntityRef(result, "&amp;",  "&");

  return result;
}

// XML Parsing
char XMLParser::hasChar(const std::string &in,  std::string::size_type &i)
{
  if (i == in.size()) return '\0';

  char ch = in[i++];

  if (ch == '\n')
  {
    _columnNumber = 1;
    _lineNumber++;
  }
  else
  {
    _columnNumber++;
  }

  return ch;
}

char XMLParser::hasChar(const std::string &in,        std::string::size_type &i,
                              std::string &out,       std::string::size_type  j)
{
  if (j < out.size()) return out[j];

  char ch = hasChar(in, i);

  if (ch != '\0') out.push_back(ch);

  return ch;
}


XMLParser::Result XMLParser::parseText(const std::string &in, std::string::size_type &i)
{
  char ch;

  while ((ch = hasChar(in, i)) != '\0')
  {
    if (ch == '<')
    {
      if (!_out.empty())
      {
        if (_handler != nullptr) _handler->text(_out);
      }
      _state = MARKUP;

      _out = ch;

      break;
    }
    else
    {
      _out.push_back(ch);
    }
  }

  return (ch == '\0' ? MORE : OK);
}

XMLParser::Result XMLParser::parseMarkup(const std::string &in, std::string::size_type &i)
{
  Result result = FAIL;

  if (result == FAIL) result = parseDeclaration(in, i);

  if (result == FAIL) result = parseSection(in, i);

  if (result == FAIL) result = parseElement(in, i);

  if (result == FAIL) result = doUnhandled();

  return result;
}

XMLParser::Result XMLParser::doUnhandled()
{
  if (_handler != nullptr) _handler->unhandled(_out, _lineNumber, _columnNumber);

  setState(TEXT);

  return OK;
}

XMLParser::Result XMLParser::parseDeclaration(const std::string &in, std::string::size_type &i)
{
  std::string::size_type j = 1;

  Result result;

  std::string target;

  if ((result = matchChar("?", in, i, _out, j)) != OK) return result;

  if ((result = skipTillChar(" \t\n\r?>", in, i, _out, j, target)) != OK) return result;

  if (target == "xml") // XMLDecl
  {
    if ((result = matchChars(" \t\n\r", in, i, _out, j)) == MORE) return result;

    XMLParserHandler::Attributes attributes;

    while (true)
    {
      std::string                  key;
      std::string                  value;

      if ((result = parseAttribute(" \t\n\r=?>", in, i, _out, j, key, value)) != OK) return result;

      if (key.empty()) break;

      attributes.insert(std::make_pair(key, value));
    }

    if ((result = matchString("?>", in, i, _out, j)) != OK) return result;

    if (_handler != nullptr) _handler->xmlDecl(_out, attributes);
  }
  else // Processing instruction
  {
    std::string value;

    if ((result = skipTillChar(" \t\r\n?>", in, i, _out, j, target)) != OK) return result;

    if (target.empty()) return FAIL;

    if ((result = matchChars(" \t\r\n", in, i, _out, j)) == MORE) return result;

    if ((result = skipTillString("?>", in, i, _out, j, value)) != OK) return result;

    if (_handler != nullptr) _handler->processingInstruction(_out, target, value);
  }

  setState(TEXT);

  return OK;
}

XMLParser::Result XMLParser::parseSection(const std::string &in, std::string::size_type &i)
{
  std::string::size_type j = 1;

  Result result;

  if ((result = matchChar("!", in, i, _out, j)) != OK) return result;

  if ((result = matchString("--", in, i, _out, j)) == MORE) return result;

  if (result == OK) // Comment
  {
    std::string text;

    text.clear();

    if ((result = skipTillString("-->", in, i, _out, j, text)) != OK) return result;

    if (_handler != nullptr) _handler->comment(_out, text);

    setState(TEXT);

    return OK;
  }

  if ((result = matchString("[CDATA[", in, i, _out, j)) == MORE) return result;

  if (result == OK)
  {
    std::string text;

    text.clear();

    if ((result = skipTillString("]]>", in, i, _out, j, text)) != OK) return result;

    if (_handler != nullptr) _handler->cdataDecl(_out, text);
  }
  else
  {
    if ((result = matchString("DOCTYPE", in, i, _out, j)) != OK) return result;

    if ((result = matchChar(" \t\n\r", in, i, _out, j)) != OK) return result;

    std::string dummy;

    if ((result = skipTillChar("[>", in, i, _out, j, dummy)) != OK) return result;

    if ((result = matchChar("[", in, i, _out, j)) == MORE) return result;

    if (result == OK)
    {
      if ((result = skipTillChar("]", in, i, _out, j, dummy)) != OK) return result;

      if ((result = matchChar("]", in, i, _out, j)) != OK) return result;
    }

    if ((result = skipTillChar(">", in, i, _out, j, dummy)) != OK) return result;

    if ((result = matchChar(">", in, i, _out, j)) != OK) return result;

    if (_handler != nullptr) _handler->docTypeDecl(_out);
  }
  setState(TEXT);

  return OK;
}

XMLParser::Result XMLParser::parseAttribute(const std::string &pattern,
                                            const std::string &in,  std::string::size_type &i,
                                                  std::string &out, std::string::size_type &j,
                                                  std::string &key, std::string &value)
{
  Result result;

  key.clear();

  if ((result = skipTillChar(pattern, in, i, out, j, key)) != OK) return result;

  if (key.empty()) return OK;

  if ((result = matchChars(" \t\r\n", in, i, out, j)) == MORE) return result;

  if ((result = matchChar("=", in, i, out, j)) != OK) return result;

  if ((result = matchChars(" \t\r\n", in, i, out, j)) == MORE) return result;

  if ((result = matchChar("\"", in, i, out, j)) == MORE) return result;

  value.clear();

  if (result == OK)
  {
    if ((result = skipTillChar("\">", in, i, out, j, value)) != OK) return result;

    if ((result = matchChar(">", in, i, out, j)) == OK) return FAIL;

    if ((result = matchChar("\"", in, i, out, j)) != OK) return result;
  }
  else
  {
    if ((result = matchChar("'", in, i, out, j)) != OK) return result;

    if ((result = skipTillChar("'>", in, i, out, j, value)) != OK) return result;

    if ((result = matchChar(">", in, i, out, j)) == OK) return FAIL;

    if ((result = matchChar("'", in, i, out, j)) != OK) return result;
  }

  if ((result = matchChars(" \t\r\n", in, i, out, j)) == MORE) return result;

  return OK;
}

XMLParser::Result XMLParser::parseElement(const std::string &in, std::string::size_type &i)
{
  std::string::size_type j = 1;

  bool startTag = false;
  bool endTag   = false;

  Result result;

  XMLParserHandler::Attributes attributes;

  if ((result = matchChar("/", in, i, _out, j)) == MORE) return result;

  if (result == OK) endTag = true; else startTag = true;

  std::string name;
  std::string key;
  std::string value;

  name.clear();

  if ((result = skipTillChar(" \t\n\r/>", in, i, _out, j, name)) != OK) return result;

  if (name.empty()) return FAIL;

  if ((result = matchChars(" \t\r\n", in, i, _out, j)) == MORE) return result;

  while (startTag)
  {
    if ((result = parseAttribute(" \t\n\r=/>", in, i, _out, j, key, value)) != OK) return result;

    if (key.empty()) break;

    attributes.insert(std::make_pair(key, value));
  }

  if ((result = matchChar("/", in, i, _out, j)) == MORE) return result;

  if (result == OK)
  {
    if (endTag) return FAIL;

    endTag = true;
  }

  if ((result = matchChar(">", in, i, _out, j)) != OK) return result;

  if (_handler != nullptr)
  {
    if (startTag && endTag)
    {
      _handler->startEndElement(_out, name, attributes);
    }
    else if (startTag)
    {
      _handler->startElement(_out, name, attributes);
    }
    else if (endTag)
    {
      _handler->endElement(_out, name);
    }

    setState(TEXT);
  }

  return OK;
}

// Helpers
bool XMLParser::isInChars(char ch, const std::string &chars)
{
  for (std::string::size_type k = 0; k < chars.size(); k++)
  {
    if (ch == chars[k]) return true;
  }

  return false;
}

XMLParser::Result XMLParser::matchChar(const std::string &chars,
                                       const std::string &in,  std::string::size_type &i,
                                             std::string &out, std::string::size_type &j)
{
  char ch;

  if ((ch = hasChar(in, i, out, j)) == '\0') return MORE;

  if (!isInChars(ch, chars)) return FAIL;

  j++;

  return OK;
}

XMLParser::Result XMLParser::matchNotChar(const std::string &chars,
                                          const std::string &in,  std::string::size_type &i,
                                                std::string &out, std::string::size_type &j)
{
  char ch;

  if ((ch = hasChar(in, i, out, j)) == '\0') return MORE;

  if (isInChars(ch, chars)) return FAIL;

  j++;

  return OK;
}

XMLParser::Result XMLParser::matchString(const std::string &pattern,
                                         const std::string &in,  std::string::size_type &i,
                                               std::string &out, std::string::size_type &j)
{
  for (std::string::size_type k = 0; k < pattern.size(); k++)
  {
    char ch;

    if ((ch = hasChar(in, i, out, j + k)) == '\0') return MORE;

    if (ch != pattern[k]) return FAIL;
  }

  j += pattern.size();

  return OK;
}

XMLParser::Result XMLParser::matchChars(const std::string &chars,
                                        const std::string &in,  std::string::size_type &i,
                                              std::string &out, std::string::size_type &j)
{
  std::string::size_type k = j;

  Result result;

  while ((result = matchChar(chars, in, i, out, j)) == OK)
  {
  }

  if (result == FAIL && j > k) result = OK;

  return result;
}

XMLParser::Result XMLParser::matchNotChars(const std::string &chars,
                                           const std::string &in,  std::string::size_type &i,
                                                 std::string &out, std::string::size_type &j)
{
  std::string::size_type k = j;

  Result result;

  while ((result = matchNotChar(chars, in, i, out, j)) == OK)
  {
  }

  if (result == FAIL && j > k) result = OK;

  return result;
}


// Pattern is processed !
XMLParser::Result XMLParser::skipTillString(const std::string &pattern,
                                            const std::string &in,  std::string::size_type &i,
                                                  std::string &out, std::string::size_type &j,
                                                  std::string &text)
{
  Result result = FAIL;

  while ((result = matchString(pattern, in, i, out, j)) == FAIL)
  {
    text.push_back(out[j++]);
  }

  return result;
}

// Till char is not processed !
XMLParser::Result XMLParser::skipTillChar(const std::string &chars,
                                          const std::string &in,  std::string::size_type &i,
                                                std::string &out, std::string::size_type &j,
                                                std::string &text)
{
  Result result;

  while ((result = matchChar(chars, in, i, out, j)) == FAIL)
  {
    text.push_back(out[j++]);
  }

  if (result == OK) j--;

  return result;
}

// Till char is not processed !
XMLParser::Result XMLParser::skipTillNotChar(const std::string &chars,
                                             const std::string &in,  std::string::size_type &i,
                                                   std::string &out, std::string::size_type &j,
                                                   std::string &text)
{
  Result result = FAIL;

  while ((result = matchNotChar(chars, in, i, out, j)) == FAIL)
  {
    text.push_back(out[j++]);
  }

  if (result == OK) j--;

  return result;
}

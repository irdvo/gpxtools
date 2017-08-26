#ifndef XMLPARSER_H
#define XMLPARSER_H

//==============================================================================
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
//==============================================================================

#include <string>
#include <iostream>
#include <map>

///
/// @class XMLParserHandler
///
/// @brief The xml parser handler class.
///
class XMLParserHandler
{
public:
  XMLParserHandler() {}
  virtual ~XMLParserHandler() {}

  typedef std::map<std::string, std::string> Attributes;

  virtual void xmlDecl(const std::string &text, const Attributes &attributes) = 0;
  virtual void processingInstruction(const std::string &text, const std::string &target, const std::string &value) = 0;
  virtual void docTypeDecl(const std::string &text) = 0;
  virtual void comment(const std::string &text, const std::string &comment) = 0;
  virtual void startElement(const std::string &text, const std::string &name, const Attributes &attributes) = 0;
  virtual void endElement(const std::string &text, const std::string &name) = 0;
  virtual void startEndElement(const std::string &text, const std::string &name, const Attributes &attributes) = 0;
  virtual void text(const std::string &text) = 0;
  virtual void cdataDecl(const std::string &text, const std::string &data) = 0;
  virtual void unhandled(const std::string &text, int lineNumber, int columnNumber) = 0;
};

///
/// @class XMLParser
///
/// @brief The xml parser class.
///
class XMLParser
{
public:

  ///
  /// Constructor
  ///
  XMLParser();

  ///
  /// Constructor
  ///
  /// @param handler     the XMLParser handler
  ///
  XMLParser(XMLParserHandler *handler);

  ///
  /// Deconstructor
  ///
  virtual ~XMLParser();

  // Properties

  ///
  /// Set the XMLParser handler
  ///
  /// @param handler     the XMLParser handler
  ///
  void setHandler(XMLParserHandler *handler) { _handler = handler; }

  ///
  /// Get the current line number
  ///
  /// @return the current line number
  ///
  int lineNumber() const { return _lineNumber; }

  ///
  /// Get the current column number
  ///
  /// @return the current column number
  ///
  int columnNumber() const { return _columnNumber; }

  // Parsing methods

  ///
  /// Parse data
  ///
  /// @param  data    the data to be parsed
  /// @param  length  the length of the data
  /// @param  isFinal is this the last data ?
  ///
  /// @return success
  ///
  bool parse(const char *data, size_t length, bool isFinal);

  ///
  /// Parse text
  ///
  /// @param  text    the zero terminated text to be parsed
  /// @param  isFinal is this the last data ?
  ///
  /// @return success
  ///
  bool parse(const char *text, bool isFinal);

  ///
  /// Parse string
  ///
  /// @param  data    the data to be parsed
  /// @param  isFinal is this the last data ?
  ///
  /// @return success
  ///
  bool parse(const std::string &data, bool isFinal);

  ///
  /// Parse an input stream
  ///
  /// @param  stream        the stream to be parsed
  ///
  /// @return success
  ///
  bool parse(std::istream &stream);


  ///
  /// Trim the text from whitespace
  ///
  /// @param  text         the text
  ///
  /// @return the text without whitespace
  ///
  static std::string trim(const std::string &text);

  ///
  /// Translate an entity reference in the text
  ///
  /// @param text          the text
  /// @param pattern       the entity reference (example: &amp;)
  /// @param replace       the replacement (example: &)
  ///
  static void translateEntityRef(std::string &text, const std::string &pattern, const std::string &replace);

  ///
  /// Translate the entity references: &lt; &gt; &apos; &quot; &amp;
  ///
  /// @param text          the text
  ///
  /// @return the text with the translated entity refs
  ///
  static std::string translateEntityRefs(const std::string &text);

private:
  // Types
  enum State
  {
    TEXT,
    MARKUP,
    COMMENT,
    PI,
    ELEMENT
  };
  void setState(State state);

  enum Result
  {
    ERROR,
    MORE,
    OK,
    FAIL
  };

  // String parsing

  char hasChar(const std::string &in, std::string::size_type &i);

  char hasChar(const std::string &in, std::string::size_type &i, std::string &out, std::string::size_type j);

  static bool isInChars(char ch, const std::string &chars);

  // XML Parsing
  Result parseText(const std::string &in, std::string::size_type &i);
  Result parseMarkup(const std::string &in, std::string::size_type &i);

  Result parseDeclaration(const std::string &in, std::string::size_type &i);
  Result parseSection(const std::string &in, std::string::size_type &i);
  Result parseElement(const std::string &in, std::string::size_type &i);
  Result parseAttribute(const std::string &pattern,
                        const std::string &in,  std::string::size_type &i,
                              std::string &out, std::string::size_type &j,
                              std::string &key, std::string &value);
  Result doUnhandled();

  Result matchChar(const std::string &chars,
                   const std::string &in, std::string::size_type &i,
                         std::string &out, std::string::size_type &j);
  Result matchNotChar(const std::string &chars,
                      const std::string &in, std::string::size_type &i,
                            std::string &out, std::string::size_type &j);
  Result matchString(const std::string &pattern,
                     const std::string &in, std::string::size_type &i,
                           std::string &out, std::string::size_type &j);
  Result matchChars(const std::string &chars,
                    const std::string &in, std::string::size_type &i,
                    std::string &out, std::string::size_type &j);
  Result matchNotChars(const std::string &chars,
                       const std::string &in, std::string::size_type &i,
                       std::string &out, std::string::size_type &j);
  Result skipTillString(const std::string &pattern,
                         const std::string &in, std::string::size_type &i,
                               std::string &out, std::string::size_type &j,
                               std::string &text);
  Result skipTillNotChar(const std::string &chars,
                          const std::string &in, std::string::size_type &i,
                                std::string &out, std::string::size_type &j,
                                std::string &text);
  Result skipTillChar(const std::string &chars,
                       const std::string &in, std::string::size_type &i,
                             std::string &out, std::string::size_type &j,
                             std::string &text);

  // Members
  XMLParserHandler *_handler;

  State             _state;
  std::string       _out;

  int               _lineNumber;
  int               _columnNumber;

  // Disable copy constructors
  XMLParser(const XMLParser &);
  XMLParser& operator=(const XMLParser &);

};

#endif


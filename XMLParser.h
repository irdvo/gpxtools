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

#include "expat.h"


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
  /// Deconstructor
  ///
  virtual ~XMLParser();

  // Properties

  ///
  /// Get the last error text
  ///
  /// @return the last error text
  ///
  const std::string &errorText() const { return _errorText; }

  ///
  /// Get the last error line number
  ///
  /// @return the last error line number
  ///
  int errorLineNumber() const { return _errorLineNumber; }

  ///
  /// Get the last error column number
  ///
  /// @return the last error column number
  ///
  int errorColumnNumber() const { return _errorColumnNumber; }

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

  // Interface
  typedef std::map<std::string, std::string> Attributes;

  virtual void parseError(const std::string &text, int lineNumber, int columnNumber) { }
  virtual void xmlDecl(const std::string &version, const std::string &encoding, int standalone) { }
  virtual void startElement(const std::string &name, const Attributes &atts) { }
  virtual void endElement(const std::string &name) { }
  virtual void characterData(const std::string &data) { }
  virtual void comment(const std::string &data) { }
  virtual void processingInstruction(const std::string &target, const std::string &data) { }
  virtual void startCDATASection() { }
  virtual void endCDATASection() { }
  virtual void unspecified(const std::string &data) { }
  virtual void startNamespaceDecl(const std::string &prefix, const std::string &uri) { }
  virtual void endNamespaceDecl(const std::string &prefix) { }

private:

  // Start using expat
  void startExpat();

  // Stop using expat
  void stopExpat();

  // Statics for expat
  static void xmlDeclHandler              (void *userData, const XML_Char *version, const XML_Char *encoding, int standalone);
  static void startElementHandler         (void *userData, const XML_Char *name,    const XML_Char **atts);
  static void endElementHandler           (void *userData, const XML_Char *name);
  static void characterDataHandler        (void *userData, const XML_Char *s, int len);
  static void commentHandler              (void *userData, const XML_Char *data);
  static void processingInstructionHandler(void *userData, const XML_Char *target, const XML_Char *data);
  static void startCdataSectionHandler    (void *userData);
  static void endCdataSectionHandler      (void *userData);
  static void defaultHandler              (void *userData, const XML_Char *s, int len);
  static void startNamespaceDeclHandler   (void *userData, const XML_Char *prefix, const XML_Char *uri);
  static void endNamespaceDeclHandler     (void *userData, const XML_Char *prefix);

  // Members
  XML_Parser      _parser;
  std::string     _errorText;
  int             _errorLineNumber;
  int             _errorColumnNumber;

  // Disable copy constructors
  XMLParser(const XMLParser &);
  XMLParser& operator=(const XMLParser &);
};

#endif


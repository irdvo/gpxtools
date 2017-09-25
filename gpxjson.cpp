#include <iostream>
#include <cstring>
#include <fstream>
#include <vector>
#include <cmath>
#include <limits>
#include <iomanip>

#include "XMLParser.h"

const std::string tool    = "gpxjson";
const std::string version = "0.1.0";

// ----------------------------------------------------------------------------

class GpxJson : public XMLParserHandler
{
public:
  // -- Constructor -----------------------------------------------------------
  GpxJson() :
    _mode(NORMAL),
    _number(4)
  {
  }

  // -- Deconstructor ---------------------------------------------------------
  virtual ~GpxJson()
  {
  }

  // -- Properties ------------------------------------------------------------

  enum Mode { COMPACT, NORMAL };

  void setMode(Mode mode) { _mode = mode; }

  void setNumber(int number) { _number = number; }

  // -- Parse a file ----------------------------------------------------------
  bool parseFile(std::istream &input, std::ostream &output)
  {
    _path.clear();
    _lines.clear();
    _points.clear();

    XMLParser parser(this);

    parser.parse(input);

    outputJson(output);

    return true;
  }

  static double getDouble(const std::string &value)
  {
    try
    {
      return std::stod(value);
    }
    catch (...)
    {
      return std::numeric_limits<double>::min();
    }
  }

  static double getInt(const std::string &value)
  {
    try
    {
      return std::stoi(value);
    }
    catch (...)
    {
      return 0;
    }
  }

private:
  // Structs
  struct Point
  {
    Point(double lat, double lon)
    {
      _lat        = lat;
      _lon        = lon;
    }

    double        _lat;
    double        _lon;
  };


  void outputJson(std::ostream &output)
  {
    int points = _points.size();
    int lines  = _lines.size();

    if (points == 1) outputOnePoint(output);
    if (points >  1) outputMultiplePoints(output);

    if (lines == 1) outputOneLine(output);
    if (lines >  1) outputMultipleLines(output);
  }

  void outputOnePoint(std::ostream &output)
  {
    output << std::fixed << std::setprecision(6);
    output << '{'; doEndl(output);
    doIndent();
    output << _indent << "\"type\":\"Point\","; doEndl(output);
    output << _indent << "\"coordinates\":[" << _points.front()._lon << ',' << _points.front()._lat << "]"; doEndl(output);
    doOutdent();
    output << '}'; doEndl(output);
  }

  void outputMultiplePoints(std::ostream &output)
  {
    output << std::fixed << std::setprecision(6);
    output << '{'; doEndl(output);
    doIndent();
    output << _indent << "\"type\":\"MultiPoint\","; doEndl(output);
    output << _indent << "\"coordinates\":["; doEndl(output);
    doIndent();

    auto iter = _points.begin();
    while (iter != _points.end())
    {
      output << _indent;

      int count = 0;
      while (iter != _points.end() && count < _number)
      {
        auto next = iter + 1;

        output << '[' << iter->_lon << ',' << iter->_lat << ']';

        if (next != _points.end()) output << ',';

        count++;
      }
      doEndl(output);

      ++iter;
    }

    doOutdent();
    output << _indent << "]"; doEndl(output);
    doOutdent();
    output << '}'; doEndl(output);
  }

  void outputOneLine(std::ostream &output)
  {
    output << std::fixed << std::setprecision(6);
    output << '{'; doEndl(output);
    doIndent();
    output << _indent << "\"type\":\"LineString\","; doEndl(output);
    output << _indent << "\"coordinates\":["; doEndl(output);
    doIndent();

    auto iter = _lines.front().begin();
    auto end  = _lines.front().end();

    while (iter != end)
    {
      output << _indent;

      int count = 0;
      while (iter != end && count < _number)
      {
        auto next = iter + 1;

        output << '[' << iter->_lon << ',' << iter->_lat << ']';

        if (next != end) output << ',';

        count++;
      }
      doEndl(output);

      ++iter;
    }

    doOutdent();
    output << _indent << "]"; doEndl(output);
    doOutdent();
    output << '}'; doEndl(output);
  }

  void outputMultipleLines(std::ostream &output)
  {
    output << std::fixed << std::setprecision(6);
    output << '{'; doEndl(output);
    doIndent();
    output << _indent << "\"type\":\"MultiLineString\","; doEndl(output);
    output << _indent << "\"coordinates\":["; doEndl(output);
    doIndent();

    output << _indent;
    for (auto iter2 = _lines.begin(); iter2 != _lines.end(); ++iter2)
    {
      auto next2 = iter2 + 1;

      output << "["; doEndl(output);
      doIndent();

      auto iter = iter2->begin();
      while (iter != iter2->end())
      {
        output << _indent;

        int count = 0;
        while (iter != iter2->end() && count < _number)
        {
          auto next = iter + 1;

          output << '[' << iter->_lon << ',' << iter->_lat << ']';

          if (next != iter2->end()) output << ',';

          ++iter; count++;
        }
        doEndl(output);
      }
      doOutdent();

      output << _indent << "]";
      if (next2 != _lines.end()) output << ",";
    }
    doEndl(output);
    doOutdent();

    output << _indent << "]"; doEndl(output);
    doOutdent();

    output << "}"; doEndl(output);
  }

  void doEndl(std::ostream &output)
  {
    if (_mode == NORMAL) output << std::endl;
  }

  void doIndent()
  {
    if (_mode == NORMAL) _indent += "  ";
  }

  void doOutdent()
  {
    if (_mode == NORMAL) _indent.erase(_indent.size()-2);
  }

  static double getDoubleAttribute(const Attributes &atts, const std::string &key)
  {
    auto iter = atts.find(key);

    return iter != atts.end() ? getDouble(iter->second) : std::numeric_limits<double>::min();
  }


  void doStartElement(const std::string &name, const Attributes &attributes)
  {
    _path.append("/");
    _path.append(name);

    if (_path == "/gpx/trk/trkseg" || _path == "/gpx/rte")
    {
      _line.clear();
    }
    else if (_path == "/gpx/trk/trkseg/trkpt" || _path == "/gpx/rte/rtept")
    {
      double lat = getDoubleAttribute(attributes, "lat");
      double lon = getDoubleAttribute(attributes, "lon");

      _line.push_back(Point(lat, lon));
    }
    else if (_path == "/gpx/wpt")
    {
      double lat = getDoubleAttribute(attributes, "lat");
      double lon = getDoubleAttribute(attributes, "lon");

      _points.push_back(Point(lat, lon));
    }
  }

  void doEndElement()
  {
    if (_path == "/gpx/trk/trkseg" || _path == "/gpx/rte")
    {
      _lines.push_back(_line);
    }

    size_t i =  _path.find_last_of('/');

    if (i != std::string::npos) _path.erase(i);
  }

public:
  // -- Callbacks -------------------------------------------------------------
  virtual void xmlDecl(const std::string &, const Attributes &)
  {
  }

  virtual void processingInstruction(const std::string &, const std::string &, const std::string &)
  {
  }

  virtual void docTypeDecl(const std::string &text)
  {
  }

  virtual void unhandled(const std::string &text, int lineNumber, int columnNumber)
  {
    std::cerr << "  ERROR: Unexpected gpx info: " << text <<  " on line: " << lineNumber << " columnNumber: " << columnNumber << std::endl;
    exit(1);
  }

  virtual void cdataDecl(const std::string &, const std::string &)
  {
  }

  virtual void comment(const std::string &, const std::string &)
  {
  }

  virtual void startEndElement(const std::string &, const std::string &name, const Attributes &attributes)
  {
    doStartElement(name, attributes);

    doEndElement();
  }

  virtual void startElement(const std::string &, const std::string &name, const Attributes &attributes)
  {
    doStartElement(name, attributes);
  }

  virtual void text(const std::string &text)
  {
  }

  virtual void endElement(const std::string &, const std::string &)
  {
    doEndElement();
  }

private:
  // Types
  typedef std::vector<Point>  Line;

  // Members
  std::string         _path;
  Mode                _mode;
  int                 _number;

  Line                _line;

  std::vector<Line>   _lines;
  std::vector<Point>  _points;

  std::string         _indent;
};

// -- Main program ------------------------------------------------------------

int main(int argc, char *argv[])
{
  GpxJson gpxJson;

  std::string inputFilename;
  std::string outputFilename;

  int i = 1;
  while (i < argc)
  {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-?") == 0)
    {
      std::cout << "Usage: " << tool << " [-h] [-v] [-m compact|semi|full] [-o <out.json] [<file.gpx>]" << std::endl;
      std::cout << "  -h                   help" << std::endl;
      std::cout << "  -v                   show version" << std::endl;
      std::cout << "  -m compact|normal    set the output mode" << std::endl;
      std::cout << "  -n <number>          set the number of points per line (in normal mode) (def. 4)" << std::endl;
      std::cout << "  -o <out.json>        the output json file (overwrites existing file)" << std::endl;
      std::cout << " file.gpx              the input gpx file" << std::endl << std::endl;
      std::cout << "   Convert a gpx file to GeoJson." << std::endl;
      return 0;
    }
    else if (strcmp(argv[i], "-v") == 0)
    {
      std::cout << tool << " v" << version << std::endl;
      return 0;
    }
    else if (strcmp(argv[i], "-m") == 0 && i+1 < argc)
    {
      i++;
      if (strcmp(argv[i], "compact") == 0)
      {
        gpxJson.setMode(GpxJson::COMPACT);
      }
      else if (strcmp(argv[i], "normal") == 0)
      {
        gpxJson.setMode(GpxJson::NORMAL);
      }
      else
      {
        std::cerr << "Error: unknown mode: " << argv[i] << std::endl;
        return 1;
      }
    }
    else if (strcmp(argv[i], "-n") == 0 && i+1 < argc)
    {
      int number = GpxJson::getInt(argv[++i]);

      if (number > 0)
      {
        gpxJson.setNumber(number);
      }
      else
      {
        std::cerr << "Error: invalid number: " << argv[i] << std::endl;
      }
    }
    else if (strcmp(argv[i], "-o") == 0 && i+1 < argc)
    {
      if (outputFilename.empty())
      {
        outputFilename = argv[++i];
      }
      else
      {
        std::cerr << "Error: multiple output files specified: " << argv[i] << std::endl;
        return 1;
      }
    }
    else if (argv[i][0] != '-')
    {
      if (inputFilename.empty())
      {
        inputFilename = argv[i];
      }
      else
      {
        std::cerr << "Error: multiple input files defined:" << argv[i] << std::endl;
        return 1;
      }
    }
    else
    {
      std::cerr << "Error: unknown option:" << argv[i] << std::endl;
      return 1;
    }

    i++;
  }


  std::ifstream input;

  if (!inputFilename.empty())
  {
    input.open(inputFilename.c_str());

    if (!input.is_open())
    {
      std::cerr << "Error: unable to open the inputfile: " << inputFilename << std::endl;
      return 1;
    }
  }

  std::ofstream output;

  if (!outputFilename.empty())
  {
    output.open(outputFilename.c_str());

    if (!output.is_open())
    {
      std::cerr << "Error: unable to open the outputfile: " << outputFilename << std::endl;
    }
  }

  gpxJson.parseFile((inputFilename.empty() ? std::cin : input), (outputFilename.empty() ? std::cout : output));

  if (!inputFilename .empty()) input.close();
  if (!outputFilename.empty()) output.close();
  
  return 0;
}

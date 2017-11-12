#include <iostream>
#include <cstring>
#include <fstream>
#include <list>
#include <cmath>
#include <limits>
#include <iomanip>

#include "XMLParser.h"

const std::string tool   = "gpxcat";
const std::string version= "0.1.0";

// ----------------------------------------------------------------------------

class GpxCat : public XMLParserHandler
{
public:
  // -- Constructor -----------------------------------------------------------
  GpxCat() :
    _distance(-1.0),
    _doConcat(false)
  {
  }

  // -- Deconstructor ---------------------------------------------------------
  virtual ~GpxCat()
  {
  }

  // -- Properties ------------------------------------------------------------

  void setDistance(double distance) { _distance = distance; }

  // -- Process a file ----------------------------------------------------------
  void processFile(const std::string &inputFilename)
  {
    std::ifstream stream(inputFilename.c_str());

    if (stream.is_open())
    {
      parseFile(stream);

      stream.close();
    }
  }

  static bool getDoubleAttribute(const Attributes &atts, const std::string &key, double &value)
  {
    auto iter = atts.find(key);

    if (iter == atts.end()) return false;

    return getDouble(iter->second, value);
  }

  static bool getDouble(const std::string &str, double &value)
  {
    try
    {
      value = std::stod(str);

      return true;
    }
    catch (...)
    {
      return false;
    }
  }

private:
  // -- Parse a file ----------------------------------------------------------
  bool parseFile(std::istream &input)
  {
    _path.clear();

    XMLParser parser(this);

    parser.parse(input);

    return true;
  }

  static double deg2rad(double deg)
  {
    return (deg * M_PI) / 180.0;
  }
  
  static double rad2deg(double rad)
  {
    return (rad * 180.0) / M_PI;
  }
  
  // http://www.movable-type.co.uk/scripts/latlong.html
  
  // distance in metres
  static double calcDistance(double lat1deg, double lon1deg, double lat2deg, double lon2deg)
  {
    const double R = 6371E3; // m
    
    double lat1rad = deg2rad(lat1deg);
    double lon1rad = deg2rad(lon1deg);
    double lat2rad = deg2rad(lat2deg);
    double lon2rad = deg2rad(lon2deg);
    
    double dlat    = lat2rad - lat1rad;
    double dlon    = lon2rad - lon1rad;
    
    double a       = sin(dlat / 2.0) * sin(dlat / 2.0) + cos(lat1rad) * cos(lat2rad) * sin(dlon / 2.0) * sin(dlon / 2.0);
    double c       = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    
    return c * R;
  }

  void store(const std::string &text)
  {
    if (_doConcat)
    {
      _current.append(text);
    }
    else
    {
      std::cout << text;
    }
  }

  void doStartElement(const std::string &name, const Attributes &attributes)
  {
    _path.append("/");
    _path.append(name);

    if (_path == "/gpx/trk/trkseg/trkpt")
    {
      double lat, lon = 0.0;

      if (getDoubleAttribute(attributes, "lat", lat) && getDoubleAttribute(attributes, "lon", lon))
      {
        if (_doConcat)
        {
          if (_distance >= 0.0 && calcDistance(_lastLat, _lastLon, lat, lon) > _distance)
          {
            std::cout << _current;
          }

          _doConcat = false;
        }

        _lastLat = lat;
        _lastLon = lon;
      }
    }
  }

  void doEndElement()
  {
    if (_path == "/gpx/trk")
    {
      if (_doConcat)
      {
        std::cout << _current;

        _doConcat = false;
      }
    }
    else if (_path == "/gpx/trk/trkseg")
    {
      _doConcat = true;
      _current.clear();
    }

    size_t i =  _path.find_last_of('/');

    if (i != std::string::npos) _path.erase(i);
  }

public:
  // -- Callbacks -------------------------------------------------------------
  virtual void xmlDecl(const std::string &text, const Attributes &)
  {
    store(text);
  }

  virtual void processingInstruction(const std::string &text, const std::string &, const std::string &)
  {
    store(text);
  }

  virtual void docTypeDecl(const std::string &text)
  {
    store(text);
  }

  virtual void unhandled(const std::string &text, int lineNumber, int columnNumber)
  {
    std::cerr << "  ERROR: Unexpected gpx info: " << text <<  " on line: " << lineNumber << " columnNumber: " << columnNumber << std::endl;
    exit(1);
  }

  virtual void cdataDecl(const std::string &text, const std::string &)
  {
    store(text);
  }

  virtual void comment(const std::string &text, const std::string &)
  {
    store(text);
  }

  virtual void startEndElement(const std::string &text, const std::string &name, const Attributes &attributes)
  {
    doStartElement(name, attributes);

    doEndElement();

    store(text);
  }

  virtual void startElement(const std::string &text, const std::string &name, const Attributes &attributes)
  {
    doStartElement(name, attributes);

    store(text);
  }

  virtual void text(const std::string &text)
  {
    store(text);
  }

  virtual void endElement(const std::string &text, const std::string &)
  {
    doEndElement();

    store(text);
  }

private:
  // Members
  double            _distance;

  std::string       _path;
  std::string       _current;
  double            _lastLat;
  double            _lastLon;
  bool              _doConcat;
};

// -- Main program ------------------------------------------------------------

int main(int argc, char *argv[])
{
  GpxCat gpxCat;

  int i = 1;
  while (i < argc)
  {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-?") == 0)
    {
      std::cout << "Usage: " << tool << " [-h] [-v] [-d <distance>] <file.gpx> .." << std::endl;
      std::cout << "  -h              help" << std::endl;
      std::cout << "  -v              show version" << std::endl;
      std::cout << "  -d <distance>   concatenate only if the distance between the end and" << std::endl;
      std::cout << "                  the start of the segments are less than distance (metres)" << std::endl;
      std::cout << " file.gpx         the input gpx file" << std::endl << std::endl;
      std::cout << "   Concatenate the segments in the track in the GPX input file." << std::endl;
      return 0;
    }
    else if (strcmp(argv[i], "-v") == 0)
    {
      std::cout << tool << " v" << version << std::endl;
      return 0;
    }
    else if (strcmp(argv[i], "-d") == 0 && i+1 < argc)
    {
      double distance;

      if (GpxCat::getDouble(argv[++i], distance))
      {
        gpxCat.setDistance(distance);
      }
      else
      {
        std::cerr << "Error: invalid distance for option -s." << std::endl;
        return 1;
      }
    }
    else if (argv[i][0] != '-')
    {
      gpxCat.processFile(argv[i]);
    }
    else
    {
      std::cerr << "Error: unknown option:" << argv[i] << std::endl;
      return 1;
    }

    i++;
  }

  return 0;
}

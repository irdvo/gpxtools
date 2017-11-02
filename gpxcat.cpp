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
    _doWaypoints(false),
    _doRoutes(false),
    _doTracks(false),
    _doSegments(-1.0)
  {
  }

  // -- Deconstructor ---------------------------------------------------------
  virtual ~GpxCat()
  {
  }

  // -- Properties ------------------------------------------------------------

  void setWaypoints(bool waypoints) { _doWaypoints = waypoints; }

  void setRoutes(bool routes) { _doRoutes = routes; }

  void setTracks(bool tracks) { _doTracks = tracks; }

  void setSegments(double distance) { _doSegments = distance; }

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

  void outputFile(std::ostream &output)
  {
    ChunkType last = ChunkType::POINT;

    while (!_chunks.empty())
    {
      if (last != ChunkType::TEXT || _chunks.front()._type != ChunkType::TEXT)
      {
        output << _chunks.front()._text;
      }

      last = _chunks.front()._type;

      _chunks.pop_front();
    }
  }

private:
  // Structs
  enum ChunkType { TEXT, POINT };

  struct Chunk
  {
    void clear()
    {
      _type       = TEXT;
      _text.clear();
      _lat        = 0.0;
      _lon        = 0.0;
      _crossTrack = std::numeric_limits<double>::max();
    }

    void point(double lat, double lon)
    {
      _type       = POINT;
      _lat        = lat;
      _lon        = lon;
      _crossTrack = std::numeric_limits<double>::max();
    }

    ChunkType     _type;
    std::string   _text;
    double        _lat;
    double        _lon;
    double        _crossTrack;
  };

  void store(const std::string &text)
  {
    _current._text.append(text);
  }


  static bool getDoubleAttribute(const Attributes &atts, const std::string &key, double &value)
  {
    auto iter = atts.find(key);

    if (iter == atts.end()) return false;

    return getDouble(iter->second, value);
  }


  void doStartElement(const std::string &name, const Attributes &attributes)
  {
    _path.append("/");
    _path.append(name);

    if (_path == "/gpx/trk/trkseg" || _path == "/gpx/rte")
    {
      _current.clear();
    }
    else if (_path == "/gpx/trk/trkseg/trkpt" || _path == "/gpx/rte/rtept")
    {
      if (!_current._text.empty()) _chunks.push_back(_current);

      _current.clear();

      double lat, lon;

      getDoubleAttribute(attributes, "lat", lat);
      getDoubleAttribute(attributes, "lon", lon);

      _current.point(lat, lon);
    }
  }

  void doEndElement()
  {
    if (_path == "/gpx/trk/trkseg" || _path == "/gpx/rte")
    {
      if (!_current._text.empty()) _chunks.push_back(_current);
    }
    else if (_path == "/gpx/trk/trkseg/trkpt" || _path == "/gpx/rte/rtept")
    {
      _chunks.push_back(_current);

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

    store(text);

    doEndElement();
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
    store(text);

    doEndElement();
  }

private:

  // Members
  bool              _doWaypoints;
  bool              _doRoutes;
  bool              _doTracks;
  double            _doSegments;

  std::string       _path;

  Chunk             _current;
  std::list<Chunk>  _chunks;
};

// -- Main program ------------------------------------------------------------

int main(int argc, char *argv[])
{
  GpxCat gpxCat;

  std::string outputFilename;

  int i = 1;
  while (i < argc)
  {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-?") == 0)
    {
      std::cout << "Usage: " << tool << " [-h] [-v] [-w] [-r] [-t] [-s <distance>] [-o <out.gpx>] <file.gpx> .." << std::endl;
      std::cout << "  -h              help" << std::endl;
      std::cout << "  -v              show version" << std::endl;
      std::cout << "  -w              concatenate the waypoints" << std::endl;
      std::cout << "  -r              concatenate the routes" << std::endl;
      std::cout << "  -t              concatenate the tracks" << std::endl;
      std::cout << "  -s <distance>   concatenate the track segments if the distance between the two segments is less than <distance> " << std::endl;
      std::cout << " file.gpx         the input gpx file" << std::endl << std::endl;
      std::cout << "   Concatenate the waypoints, routes, tracks and/or segments in the input files." << std::endl;
      return 0;
    }
    else if (strcmp(argv[i], "-v") == 0)
    {
      std::cout << tool << " v" << version << std::endl;
      return 0;
    }
    else if (strcmp(argv[i], "-s") == 0 && i+1 < argc)
    {
      double distance;

      if (GpxCat::getDouble(argv[++i], distance))
      {
        gpxCat.setSegments(distance);
      }
      else
      {
        std::cerr << "Error: invalid distance for option -s." << std::endl;
        return 1;
      }
    }
    else if (strcmp(argv[i], "-w") == 0)
    {
      gpxCat.setWaypoints(true);
    }
    else if (strcmp(argv[i], "-r") == 0)
    {
      gpxCat.setRoutes(true);
    }
    else if (strcmp(argv[i], "-t") == 0)
    {
      gpxCat.setTracks(true);
    }
    else if (strcmp(argv[i], "-o") == 0 && i+1 < argc)
    {
      outputFilename = argv[++i];
    }
    else if (argv[i][0] != '-')
    {
      std::ifstream stream(argv[i]);

      if (!stream.is_open())
      {
        std::cerr << "Error: unable to open: " << argv[i] << std::endl;
        return 1;
      }

      gpxCat.parseFile(stream);

      stream.close();
    }
    else
    {
      std::cerr << "Error: unknown option:" << argv[i] << std::endl;
      return 1;
    }

    i++;
  }

  if (outputFilename.empty())
  {
    gpxCat.outputFile(std::cout);
  }
  else
  {
    std::ofstream output(outputFilename.c_str());

    if (output.is_open())
    {
      gpxCat.outputFile(output);

      output.close();
    }
    else
    {
      std::cerr << "Error: unable to open the outputfile: " << outputFilename << std::endl;
      return 1;
    }
  }

  return 0;
}

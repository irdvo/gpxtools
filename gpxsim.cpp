#include <iostream>
#include <cstring>
#include <fstream>
#include <list>
#include <cmath>
#include <limits>
#include <iomanip>

#include "XMLParser.h"

const std::string version= "0.1.0";
// ----------------------------------------------------------------------------

class GpxSim : public XMLParserHandler
{
public:
  // -- Constructor -----------------------------------------------------------
  GpxSim() :
    _outputFile(&std::cout),
    _verbose(false),
    _simplifyDistance(0.0),
    _simplifyCrossTrack(0.0),
    _simplifyToNumber(0),
    _inPoints(false)
  {
  }

  // -- Deconstructor ---------------------------------------------------------
  virtual ~GpxSim()
  {
  }

  // -- Properties ------------------------------------------------------------

  void setVerbose(bool verbose) { _verbose = verbose; }

  void setSimplifyDistance(double distance) { _simplifyDistance = distance; }

  void setSimplifyCrossTrack(double crossTrack) { _simplifyCrossTrack = crossTrack; }

  void setSimplifyToNumber(int number) { _simplifyToNumber = number; }

  // -- Parse a file ----------------------------------------------------------
  bool parseFile(std::istream &input, std::ostream &output)
  {
    _path.clear();

    _outputFile = &output;

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
  // std::cout << "Distance:  " << GpxSim::calcDistance(50.06639, -5.71472, 58.64389, -3.07000) << std::endl; // 968853.52

  // bearing in rads
  static double calcBearing(double lat1deg, double lon1deg, double lat2deg, double lon2deg)
  {
    double lat1rad = deg2rad(lat1deg);
    double lon1rad = deg2rad(lon1deg);
    double lat2rad = deg2rad(lat2deg);
    double lon2rad = deg2rad(lon2deg);
    
    double dlon    = lon2rad - lon1rad;
    
    double y       = sin(dlon) * cos(lat2rad);
    double x       = cos(lat1rad) * sin(lat2rad) - sin(lat1rad) * cos(lat2rad) * cos(dlon);
    
    return atan2(y, x);
  }
  // std::cout << "Bearing:   " << GpxSim::calcBearing(50.06639, -5.71472, 58.64389, -3.07000) << std::endl; // 0.16


  // crosstrack distance in metres from point3 to the point1-point2 line
  static double calcCrosstrack(double lat1deg, double lon1deg, double lat2deg, double lon2deg, double lat3deg, double lon3deg)
  {
    const double R = 6371E3; // m
    
    double distance13 = calcDistance(lat1deg, lon1deg, lat3deg, lon3deg) / R;
    double bearing13  = calcBearing(lat1deg, lon1deg, lat3deg, lon3deg);
    double bearing12  = calcBearing(lat1deg, lon1deg, lat2deg, lon2deg);
    
    return asin(sin(distance13) * sin(bearing13 - bearing12)) * R;
  }
  // std::cout << "Crosstrack:" << GpxSim::calcCrosstrack(53.3206, -1.7297, 53.1887, 0.1334, 53.2611, -0.7972) << std::endl; // -307.55


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
    if (_inPoints)
    {
      _current._text.append(text);
    }
    else
    {
      *_outputFile << text;
    }
  }

  void outputChunks()
  {
    ChunkType last = ChunkType::POINT;

    while (!_chunks.empty())
    {
      if (last != ChunkType::TEXT || _chunks.front()._type != ChunkType::TEXT)
      {
        *_outputFile << _chunks.front()._text;
      }

      last = _chunks.front()._type;

      _chunks.pop_front();
    }
  }

  void verboseChunks(const std::string &title)
  {
    int points      = 0;
    double distance = 0.0;

    auto prev = _chunks.end();

    for (auto iter = _chunks.begin(); iter != _chunks.end(); ++iter)
    {
      if (iter->_type == ChunkType::POINT)
      {
        points++;

        if (prev != _chunks.end())
        {
          distance += calcDistance(prev->_lat, prev->_lon, iter->_lat, iter->_lon);
        }

        prev = iter;
      }
    }

    std::cout << title << " Points: " << std::setw(4) << points << " Distance: " << std::setw(10) << std::setprecision(2) << std::fixed << distance << " m" << std::endl;
  }

  void simplifyDistance()
  {
    auto iter = _chunks.begin();
    auto prev = _chunks.end();

    while (iter != _chunks.end())
    {
      if (iter->_type == ChunkType::POINT)
      {
        if (prev != _chunks.end() && calcDistance(prev->_lat, prev->_lon, iter->_lat, iter->_lon) < _simplifyDistance)
        {
          iter = _chunks.erase(iter);
        }
        else
        {
          prev = iter++;
        }
      }
      else
      {
        ++iter;
      }
    }
  }

  void simplifyCrossTrack()
  {
    auto p1 = _chunks.end();
    auto p2 = _chunks.end();
    auto p3 = _chunks.begin();

    std::cout << std::setprecision(8);
    while (p3 != _chunks.end())
    {
      if (p3->_type == ChunkType::POINT)
      {
        if (p1 != _chunks.end() && p2 != _chunks.end() &&
            fabs(calcCrosstrack(p1->_lat, p1->_lon, p3->_lat, p3->_lon, p2->_lat, p2->_lon)) < _simplifyCrossTrack)
        {
          _chunks.erase(p2);
          p2 = p3;
        }
        else
        {
          p1 = p2;
          p2 = p3;
        }
      }
      ++p3;
    }
  }

  int setCrossTracks()
  {
    int points = 0;

    auto p1 = _chunks.end();
    auto p2 = _chunks.end();
    auto p3 = _chunks.begin();

    while (p3 != _chunks.end())
    {
      if (p3->_type == ChunkType::POINT)
      {
        points++;

        p3->_crossTrack = std::numeric_limits<double>::max();

        if (p1 != _chunks.end() && p2 != _chunks.end())
        {
          p2->_crossTrack = fabs(calcCrosstrack(p1->_lat, p1->_lon, p3->_lat, p3->_lon, p2->_lat, p2->_lon));
        }

        p1 = p2;
        p2 = p3;
      }
      ++p3;
    }

    return points;
  }

  std::list<Chunk>::iterator forwards(std::list<Chunk>::iterator p)
  {
    do
    {
      ++p;
    }
    while (p != _chunks.end() && p->_type != ChunkType::POINT);

    return p;
  }

  std::list<Chunk>::iterator backwards(std::list<Chunk>::iterator p)
  {
    while (p != _chunks.begin())
    {
      if ((--p)->_type == ChunkType::POINT)
      {
        return p;
      }
    }

    return _chunks.end();
  }

  void removeLowestCrossTrack()
  {
    auto lowest = _chunks.end();

    for (auto p = _chunks.begin(); p != _chunks.end(); ++p)
    {
      if (p->_type == ChunkType::POINT && (lowest == _chunks.end() || lowest->_crossTrack > p->_crossTrack))
      {
        lowest = p;
      }
    }

    if (lowest != _chunks.end())
    {
      auto p2 = backwards(lowest);
      auto p1 = (p2 != _chunks.end() ? backwards(p2) : _chunks.end());
      // p3 = lowest
      auto p4 = forwards(lowest);
      auto p5 = (p4 != _chunks.end() ? forwards(p4) : _chunks.end());

      _chunks.erase(lowest);

      // Update the crosstracks
      if (p2 != _chunks.end())
      {
        if (p1 != _chunks.end() && p4 != _chunks.end())
        {
          p2->_crossTrack = fabs(calcCrosstrack(p1->_lat, p1->_lon, p4->_lat, p4->_lon, p2->_lat, p2->_lon));
        }
        else
        {
          p2->_crossTrack = std::numeric_limits<double>::max();
        }
      }

      if (p4 != _chunks.end())
      {
        if (p2 != _chunks.end() && p5 != _chunks.end())
        {
          p4->_crossTrack = fabs(calcCrosstrack(p2->_lat, p2->_lon, p5->_lat, p5->_lon, p4->_lat, p4->_lon));
        }
        else
        {
          p4->_crossTrack = std::numeric_limits<double>::max();
        }
      }
    }
  }

  void simplifyToNumber()
  {
    int points = setCrossTracks();

    while (_simplifyToNumber < points)
    {
      removeLowestCrossTrack();
      points--;
    }
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

    if (_path == "/gpx/trk/trkseg")
    {
      _current.clear();

      _inPoints = true;
    }
    else if (_path == "/gpx/trk/trkseg/trkpt")
    {
      if (!_current._text.empty()) _chunks.push_back(_current);

      _current.clear();

      double lat = getDoubleAttribute(attributes, "lat");
      double lon = getDoubleAttribute(attributes, "lon");

      _current.point(lat, lon);
    }
  }

  void doEndElement()
  {
    if (_path == "/gpx/trk/trkseg")
    {
      if (!_current._text.empty()) _chunks.push_back(_current);

      if (_verbose) verboseChunks("Original  track segment:");

      if (_simplifyDistance > 0.0)   simplifyDistance();
      if (_simplifyCrossTrack > 0.0) simplifyCrossTrack();
      if (_simplifyToNumber > 0)     simplifyToNumber();

      if (_verbose) verboseChunks("Optimized track segment:");

      outputChunks();

      _inPoints = false;
    }
    else if (_path == "/gpx/trk/trkseg/trkpt")
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
  std::ostream     *_outputFile;
  bool              _verbose;
  double            _simplifyDistance;
  double            _simplifyCrossTrack;
  int               _simplifyToNumber;

  std::string       _path;

  bool              _inPoints;
  Chunk             _current;
  std::list<Chunk>  _chunks;
};

// -- Main program ------------------------------------------------------------

int main(int argc, char *argv[])
{
  GpxSim gpxSim;

  std::string outputFilename;

  int i = 1;
  while (i < argc)
  {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-?") == 0)
    {
      std::cout << "Usage: gpxsim [-h] [-v] [-i] [-d <distance>] [-n <number> | -t <distance>] [-o <out.gpx>] <file.gpx>" << std::endl;
      std::cout << "  -h              help" << std::endl;
      std::cout << "  -v              show version" << std::endl;
      std::cout << "  -i              report the results of the simplification (only with -o)" << std::endl;
      std::cout << "  -d <distance>   remove route or track points within distance of the previous point (in m)" << std::endl;
      std::cout << "  -n <number>     remove route or track points until the route or track contains <number> points (2..)" << std::endl;
      std::cout << "  -x <distance>   remove route or track points with a cross track distance less than <distance> (in m)" << std::endl;
      std::cout << "  -o <out.gpx>    the output gpx file (overwrites existing file)" << std::endl;
      std::cout << " file.gpx         the input gpx file" << std::endl << std::endl;
      std::cout << "   Simplify a route or track using the distance threshold and/or the Douglas-Peucker algorithm." << std::endl;
      return 0;
    }
    else if (strcmp(argv[i], "-v") == 0)
    {
      std::cout << "gpxsim v" << version << std::endl;
      return 0;
    }
    else if (strcmp(argv[i], "-i") == 0)
    {
      gpxSim.setVerbose(true);
    }
    else if (strcmp(argv[i], "-d") == 0 && i+1 < argc)
    {
      double distance = GpxSim::getDouble(argv[++i]);

      if (distance > 0.0)
      {
        gpxSim.setSimplifyDistance(distance);
      }
      else
      {
        std::cerr << "Error: invalid distance for option -d." << std::endl;
        return 1;
      }
    }
    else if (strcmp(argv[i], "-n") == 0 && i+1 < argc)
    {
      int number = GpxSim::getInt(argv[++i]);

      if (number >= 2)
      {
        gpxSim.setSimplifyToNumber(number);
      }
      else
      {
        std::cerr << "Error: invalid number for option -n." << std::endl;
        return 1;
      }
    }
    else if (strcmp(argv[i], "-x") == 0 && i+1 < argc)
    {
      double crossTrack = GpxSim::getDouble(argv[++i]);

      if (crossTrack > 0.0)
      {
        gpxSim.setSimplifyCrossTrack(crossTrack);
      }
      else
      {
        std::cerr << "Error: invalid cross track distance for option -x." << std::endl;
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
        std::cerr << "Error: output file specified twice." << std::endl;
        return 1;
      }
    }
    else if (argv[i][0] != '-')
    {
      std::ifstream stream(argv[i]);

      if (!stream.is_open())
      {
        std::cerr << "Error: unable to open: " << argv[i] << std::endl;
        return 1;
      }

      if (outputFilename.empty())
      {
        gpxSim.setVerbose(false);

        gpxSim.parseFile(stream, std::cout);
      }
      else
      {
        std::ofstream output(outputFilename.c_str());

        if (output.is_open())
        {
          gpxSim.parseFile(stream, output);

          output.close();
        }
        else
        {
          std::cerr << "Error: unable to open the outputfile: " << outputFilename << std::endl;
          return 1;
        }
      }
      break;
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

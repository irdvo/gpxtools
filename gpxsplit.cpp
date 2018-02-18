#include <iostream>
#include <cstring>
#include <fstream>
#include <list>
#include <cmath>
#include <ctime>
#include <limits>
#include <iomanip>
#include <stdexcept>

#include "XMLParser.h"

const std::string tool    = "gpxsplit";
const std::string version = "0.1.0";

// ----------------------------------------------------------------------------

class GpxSplit : public XMLParserHandler
{
public:
  // -- Constructor -----------------------------------------------------------
  GpxSplit() :
    _outputFile(&std::cout),
    _TrkNr(0),
    _TrkSegNr(0),
    _inTrkSeg(false),
    _inTime(false),
    _analyse(false),
    _time(-1),
    _distance(-1)
  {
  }

  // -- Deconstructor ---------------------------------------------------------
  virtual ~GpxSplit()
  {
  }

  // -- Properties ------------------------------------------------------------

  void setAnalyse(bool analyse) { _analyse = analyse; }

  bool getAnalyse() { return _analyse; }

  void setDistance(double distance) { _distance = distance; }

  void setTime(time_t time) { _time = time; }

  // -- Parse a file ----------------------------------------------------------
  bool parseFile(std::istream &input, std::ostream &output)
  {
    _path.clear();

    _outputFile = &output;

    _TrkNr = 0;
    _TrkSegNr = 0;

    XMLParser parser(this);

    parser.parse(input);

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
  // -- Types -----------------------------------------------------------------
  enum ChunkType { TEXT, POINT };

  struct Chunk
  {
    Chunk()
    {
      clear(TEXT);
    }

    void clear(ChunkType type)
    {
      _type       = type;
      _text.clear();
      _lat        = 0.0;
      _lon        = 0.0;
      _distance   = -1.0;
      _timeStr.clear();
      _time       = -1.0;
    }

    ChunkType     _type;
    std::string   _text;
    double        _lat;
    double        _lon;
    double        _distance;
    std::string   _timeStr;
    time_t        _time;
  };

  // -- Methods ---------------------------------------------------------------
  static bool getDoubleAttribute(const Attributes &atts, const std::string &key, double &value)
  {
    auto iter = atts.find(key);

    if (iter == atts.end()) return false;

    if (iter->second.empty()) return false;
    try
    {
      value = std::stod(iter->second);

      return true;
    }
    catch(...)
    {
      return false;
    }
  }

  void store(const std::string &text)
  {
    if (_inTrkSeg)
    {
      _current._text.append(text);
    }
    else if (!_analyse)
    {
      *_outputFile << text;
    }
  }

  void processTimeStr(std::string &timeStr, time_t &time)
  {
    timeStr = XMLParser::trim(timeStr);

    if (timeStr.size() == 0) return;

    struct tm tm;

    if (strptime(timeStr.c_str(), "%Y-%m-%dT%TZ", &tm) == nullptr) return;

    time = mktime(&tm);
  }

  // http://www.movable-type.co.uk/scripts/latlong.html

  static double deg2rad(double deg)
  {
    return (deg * M_PI) / 180.0;
  }

  // Distance in metres
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

  void analyseChunks()
  {
    int trkPtNr = 0;

    Chunk previous;

    for (auto iter = _chunks.begin(); iter != _chunks.end(); ++iter)
    {
      if (iter->_type == POINT)
      {
        trkPtNr++;

        if (_distance > 0 && iter->_distance > _distance)
        {
          if (_analyse)
          {
            std::cout << "Track " << _TrkNr << " Segment " << _TrkSegNr << " Point " << trkPtNr << " is split by distance " << iter->_distance << "m (" << previous._lat << ',' << previous._lon << ")-(" << iter->_lat << ',' << iter->_lon << ")" << std::endl;
          }
          else
          {
            *_outputFile << _endTrkSeg;
            *_outputFile << _startTrkSeg;
          }
        }

        previous = *iter;
      }

      if (!_analyse) *_outputFile << iter->_text;
    }
  }

  void doStartElement(const std::string &text, const std::string &name, const Attributes &attributes)
  {
    _path.append("/");
    _path.append(name);

    if (_path == "/gpx/trk")
    {
      _TrkNr++;
      _TrkSegNr = 0;
    }
    else if (_path == "/gpx/trk/trkseg")
    {
      _startTrkSeg = text;

      _chunks.clear();
      _current.clear(TEXT);
      _previous.clear(TEXT);

      _inTrkSeg = true;
      _TrkSegNr++;
    }
    else if (_path == "/gpx/trk/trkseg/trkpt")
    {
      if (!_current._text.empty()) _chunks.push_back(_current);

      _current.clear(TEXT);

      if (getDoubleAttribute(attributes, "lat", _current._lat) &&
          getDoubleAttribute(attributes, "lon", _current._lon))
      {
        _current._type = POINT;

        if (_previous._type == POINT)
        {
          _current._distance = calcDistance(_previous._lat, _previous._lon, _current._lat, _current._lon);
        }
      }
    }
    else if (_path == "/gpx/trk/trkseg/trkpt/time")
    {
      // <time>2012-12-03T13:13:38Z</time>
      _inTime = true;
      _current._timeStr.clear();
    }
  }

  void doEndElement(const std::string &text)
  {
    if (_path == "/gpx/trk/trkseg")
    {
      _endTrkSeg = text;

      if (!_current._text.empty()) _chunks.push_back(_current);

      analyseChunks();

      _inTrkSeg = false;
    }
    else if (_path == "/gpx/trk/trkseg/trkpt")
    {
      _chunks.push_back(_current);
      if (_current._type == POINT) _previous = _current;
      _current.clear(TEXT);
    }
    else if (_path == "/gpx/trk/trkseg/trkpt/time")
    {
      _inTime = false;
      processTimeStr(_current._timeStr, _current._time);
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
    doStartElement(text, name, attributes);

    store(text);

    doEndElement(text);
  }

  virtual void startElement(const std::string &text, const std::string &name, const Attributes &attributes)
  {
    doStartElement(text, name, attributes);

    store(text);
  }

  virtual void text(const std::string &text)
  {
    if (_inTime)
    {
      _current._timeStr.append(text);
    }
    store(text);
  }

  virtual void endElement(const std::string &text, const std::string &)
  {
    store(text);

    doEndElement(text);
  }

private:
  // -- Members ---------------------------------------------------------------
  std::string         _path;

  std::ostream       *_outputFile;

  int                 _TrkNr;
  int                 _TrkSegNr;

  bool                _inTrkSeg;
  bool                _inTime;
  std::string         _startTrkSeg;
  std::string         _endTrkSeg;
  Chunk               _current;
  Chunk               _previous;
  std::list<Chunk>    _chunks;

  bool                _analyse;
  time_t              _time;
  double              _distance;
};

// -- Main program ------------------------------------------------------------
int main(int argc, char *argv[])
{
  GpxSplit gpxSplit;

  std::string inputFilename;
  std::string outputFilename;

  int i = 1;
  while (i < argc)
  {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-?") == 0)
    {
      std::cout << "Usage: " << tool << " [-h] [-v] [-a] [-d <distance>] [-t <time>] [-o <out.gpx>] [<file.gpx>]" << std::endl;
      std::cout << "  -h                   help" << std::endl;
      std::cout << "  -v                   show version" << std::endl;
      std::cout << "  -a                   analyse the file for splitting" << std::endl;
      std::cout << "  -d <distance>        split based on distance in metres" << std::endl;
      std::cout << "  -t <time>            split based on time, format: yyyy-mm-dd hh:mm:ss" << std::endl;
      std::cout << "  -o <out.gpx>         the output gpx file (overwrites existing file)" << std::endl;
      std::cout << " file.gpx              the input gpx file" << std::endl << std::endl;
      std::cout << "   Split the track segments in a gpx in multiple track segments based on distance or time." << std::endl;
      return 0;
    }
    else if (strcmp(argv[i], "-v") == 0)
    {
      std::cout << tool << " v" << version << std::endl;
      return 0;
    }
    else if (strcmp(argv[i], "-a") == 0)
    {
      if (!outputFilename.empty())
      {
        std::cerr << "Error: option -a only valid without output file."  << std::endl;
      }
      else
      {
        gpxSplit.setAnalyse(true);
      }
    }
    else if (strcmp(argv[i], "-d") == 0 && i+1 < argc)
    {
      double distance = GpxSplit::getDouble(argv[++i]);

      if (distance > 0.0)
      {
        gpxSplit.setDistance(distance);
      }
      else
      {
        std::cerr << "Error: invalid distance for option -d." << std::endl;
        return 1;
      }
    }
    else if (strcmp(argv[i], "-t") == 0 && i+1 < argc)
    {
      struct tm fields;

      if (strptime(argv[++i], "%Y-%m-%d %H:%M:%S", &fields) != nullptr)
      {
        gpxSplit.setTime(mktime(&fields));
      }
      else
      {
        std::cerr << "Error: invalid date/time: " << argv[i] << std::endl;
      }
    }
    else if (strcmp(argv[i], "-o") == 0 && i+1 < argc)
    {
      if (gpxSplit.getAnalyse())
      {
        std::cerr << "Error: option -o is not valid for analyse mode." << std::endl;
        return 1;
      }
      else if (outputFilename.empty())
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

  gpxSplit.parseFile((inputFilename.empty() ? std::cin : input), (outputFilename.empty() ? std::cout : output));

  if (!inputFilename .empty()) input.close();
  if (!outputFilename.empty()) output.close();
  
  return 0;
}

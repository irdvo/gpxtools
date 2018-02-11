#include <iostream>
#include <cstring>
#include <fstream>
#include <list>
#include <cmath>
#include <ctime>
#include <limits>
#include <iomanip>

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

  void setDistance(double distance) { _distance = distance; }

  void setTime(time_t time) { _time = time; }

  // -- Parse a file ----------------------------------------------------------
  bool parseFile(std::istream &input, std::ostream &output)
  {
    _path.clear();

    _outputFile = &output;

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
    void clear()
    {
      _type       = TEXT;
      _text.clear();
      _lat        = 0.0;
      _lon        = 0.0;
      _time       = "";
    }

    void point(double lat, double lon)
    {
      _type       = POINT;
      _lat        = lat;
      _lon        = lon;
      _time       = "";
    }

    ChunkType     _type;
    std::string   _text;
    double        _lat;
    double        _lon;
    std::string   _time;
  };

  // -- Methods ---------------------------------------------------------------
  static double getDoubleAttribute(const Attributes &atts, const std::string &key)
  {
    auto iter = atts.find(key);

    return iter != atts.end() ? getDouble(iter->second) : std::numeric_limits<double>::min();
  }

  void store(const std::string &text)
  {
    if (_inTrkSeg)
    {
      _current._text.append(text);
    }
    else
    {
      *_outputFile << text;
    }
  }

  void doStartElement(const std::string &text, const std::string &name, const Attributes &attributes)
  {
    _path.append("/");
    _path.append(name);

    if (_path == "/gpx/trk/trkseg")
    {
      _current.clear();

      _inTrkSeg = true;
    }
    else if (_path == "/gpx/trk/trkseg/trkpt")
    {
      if (!_current._text.empty()) _chunks.push_back(_current);

      _current.clear();

      double lat = getDoubleAttribute(attributes, "lat");
      double lon = getDoubleAttribute(attributes, "lon");

      _current.point(lat, lon);
    }
    else if (_path == "/gpx/trk/trkseg/trkpt/time")
    {
      // <time>2012-12-03T13:13:38Z</time>
      _inTime = true;
    }
  }

  void doEndElement(const std::string &text)
  {
    if (_path == "/gpx/trk/trkseg")
    {
      if (!_current._text.empty()) _chunks.push_back(_current);

      if (_analyse)
      {
        //TODO: analyseChunks();
      }
      else
      {
        if (_distance > 0.0)   ; //TODO: splitByDistance();
        if (_time     > 0)     ; //TODO: splitByTime();
      }

      //TODO: outputChunks();

      _inTrkSeg = false;
    }
    else if (_path == "/gpx/trk/trkseg/trkpt")
    {
      _chunks.push_back(_current);

      _current.clear();
    }
    else if (_path == "/gpx/trk/trkseg/trkpt/time")
    {
      _inTime = false;
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
      _current._time.append(text);
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

  bool                _inTrkSeg;
  bool                _inTime;
  Chunk               _current;
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
      gpxSplit.setAnalyse(true);
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

      if (strptime(argv[++i], "%Y-%m-%d %H:%M:%S", &fields) != NULL)
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

  gpxSplit.parseFile((inputFilename.empty() ? std::cin : input), (outputFilename.empty() ? std::cout : output));

  if (!inputFilename .empty()) input.close();
  if (!outputFilename.empty()) output.close();
  
  return 0;
}

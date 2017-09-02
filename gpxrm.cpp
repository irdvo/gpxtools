#include <iostream>
#include <cstring>
#include <fstream>

#include "XMLParser.h"

const std::string version= "0.1.0";
// ----------------------------------------------------------------------------

class GpxRm : public XMLParserHandler
{
public:
  // -- Constructor -----------------------------------------------------------
  GpxRm() :
    _outputFile(&std::cout),
    _segmentNr(0),
    _inWaypoint(false),
    _inRoute(false),
    _inTrack(false),
    _inSegment(false)
  {
  }

  // -- Deconstructor ---------------------------------------------------------
  virtual ~GpxRm()
  {
  }

  // -- Properties ------------------------------------------------------------
  void setWaypointName(const std::string &name) { _waypointName = name; }
  const std::string &waypointName() const { return _waypointName; }

  void setTrackName(const std::string &name) { _trackName = name; }
  const std::string &trackName() const { return _trackName; }

  void setSegmentNr(int nr) { _segmentNr = nr; }
  int segmentNr() const { return _segmentNr; }

  void setRouteName(const std::string &name) { _routeName = name; }
  const std::string &routeName() const { return _routeName; }

  // -- Parse a file ----------------------------------------------------------
  bool parseFile(std::istream &input, std::ostream &output)
  {
    _path.clear();

    _outputFile = &output;

    XMLParser parser(this);

    parser.parse(input);

    return true;
  }

private:
  void log(const std::string &text)
  {
    if (_inWaypoint || _inRoute || _inTrack || _inSegment)
    {
      _currentText.append(text);
    }
    else
    {
      *_outputFile << text;
    }
  }

  void doStartElement(const std::string &name)
  {
    _path.append("/");
    _path.append(name);

    if (_path == "/gpx/wpt")
    {
      if (!_waypointName.empty()) _inWaypoint = true;

      _currentText.clear();
      _currentName.clear();

      _currentSegmentNr = 0;
    }
    else if (_path == "/gpx/rte")
    {
      if (!_routeName.empty()) _inRoute = true;

      _currentText.clear();
      _currentName.clear();

      _currentSegmentNr = 0;
    }
    else if (_path == "/gpx/trk")
    {
      if (!_trackName.empty() && _segmentNr == 0) _inTrack = true;

      _currentText.clear();
      _currentName.clear();

      _currentSegmentNr = 0;
    }
    else if (_path == "/gpx/trk/trkseg")
    {
      _currentSegmentNr++;

      if (_segmentNr != 0 && _segmentNr == _currentSegmentNr)
      {
        _currentText.clear();

        _inSegment = true;
      }
    }
  }

  void doEndElement()
  {
    if (_path == "/gpx/wpt")
    {
      if (_inWaypoint && _currentName != _waypointName) *_outputFile << _currentText;

      _inWaypoint = false;
    }
    else if (_path == "/gpx/rte")
    {
      if (_inRoute && _currentName != _routeName) *_outputFile << _currentText;

      _inRoute = false;
    }
    else if (_path == "/gpx/trk")
    {
      if (_inTrack && _currentName != _trackName) *_outputFile << _currentText;

      _inTrack = false;
    }
    else if (_path == "/gpx/trk/trkseg")
    {
      if (_inSegment && _currentName != _trackName) *_outputFile << _currentText;

      _inSegment = false;
    }

    size_t i =  _path.find_last_of('/');

    if (i != std::string::npos) _path.erase(i);
  }

public:
  // -- Callbacks -------------------------------------------------------------
  virtual void xmlDecl(const std::string &text, const Attributes &)
  {
    log(text);
  }

  virtual void processingInstruction(const std::string &text, const std::string &, const std::string &)
  {
    log(text);
  }

  virtual void docTypeDecl(const std::string &text)
  {
    log(text);
  }

  virtual void unhandled(const std::string &text, int lineNumber, int columnNumber)
  {
    std::cerr << "  ERROR: Unexpected gpx info: " << text <<  " on line: " << lineNumber << " columnNumber: " << columnNumber << std::endl;
    exit(1);
  }

  virtual void cdataDecl(const std::string &text, const std::string &)
  {
    log(text);
  }

  virtual void comment(const std::string &text, const std::string &)
  {
    log(text);
  }

  virtual void startEndElement(const std::string &text, const std::string &name, const Attributes &)
  {
    doStartElement(name);

    log(text);

    doEndElement();
  }

  virtual void startElement(const std::string &text, const std::string &name, const Attributes &)
  {
    doStartElement(name);

    log(text);
  }

  virtual void text(const std::string &text)
  {
    if (_path == "/gpx/wpt/name" || _path == "/gpx/rte/name" || _path == "/gpx/trk/name")
    {
      _currentName = XMLParser::translateEntityRefs(XMLParser::trim(text));
    }

    log(text);
  }

  virtual void endElement(const std::string &text, const std::string &)
  {
    log(text);

    doEndElement();
  }


private:
  // Members
  std::ostream *_outputFile;
  std::string   _waypointName;
  std::string   _trackName;
  int           _segmentNr; // 1..
  std::string   _routeName;

  std::string   _path;

  bool          _inWaypoint;
  bool          _inRoute;
  bool          _inTrack;
  std::string   _currentText;
  std::string   _currentName;

  bool          _inSegment;
  int           _currentSegmentNr;
};

// -- Main program ------------------------------------------------------------

int main(int argc, char *argv[])
{
  GpxRm gpxrm;

  std::string outputFilename;

  int i = 1;
  while (i < argc)
  {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-?") == 0)
    {
      std::cout << "Usage: gpxrm [-h] [-v] [-w \"<name>\"] [-t \"<name>\" [-s <segment>]] [-r \"<name>\"] [-o <out.gpx>] <file.gpx>" << std::endl;
      std::cout << "  -h              help" << std::endl;
      std::cout << "  -v              show version" << std::endl;
      std::cout << "  -w \"name\"       remove the waypoint with <name>" << std::endl;
      std::cout << "  -t \"<name>\"     remove the track with <name>" << std::endl;
      std::cout << "  -s <segment>    remove only segment in track (1..)" << std::endl;
      std::cout << "  -r \"<name>\"     remove the route with <name>" << std::endl;
      std::cout << "  -o <out.gpx>    the output gpx file (overwrites existing file)" << std::endl;
      std::cout << " file.gpx         the input gpx file" << std::endl << std::endl;
      std::cout << "   Remove a waypoint, track or route from a gpx file" << std::endl;
      return 0;
    }
    else if (strcmp(argv[i], "-v") == 0)
    {
      std::cout << "gpxrm v" << version << std::endl;
      return 0;
    }
    else if (strcmp(argv[i], "-w") == 0 && i+1 < argc)
    {
      gpxrm.setWaypointName(argv[++i]);
    }
    else if (strcmp(argv[i], "-t") == 0 && i+1 < argc)
    {
      gpxrm.setTrackName(argv[++i]);
    }
    else if (strcmp(argv[i], "-s") == 0 && i+1 < argc)
    {
      if (gpxrm.trackName().empty())
      {
        std::cerr << "Error: segment number without track filter" << std::endl;
        return 1;
      }
      else
      {
        gpxrm.setSegmentNr(atoi(argv[++i]));
      }
    }
    else if (strcmp(argv[i], "-r") == 0 && i < argc)
    {
      gpxrm.setRouteName(argv[++i]);
    }
    else if (strcmp(argv[i], "-o") == 0 && i < argc)
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
        gpxrm.parseFile(stream, std::cout);
      }
      else
      {
        std::ofstream output(outputFilename.c_str());

        if (output.is_open())
        {
          gpxrm.parseFile(stream, output);

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

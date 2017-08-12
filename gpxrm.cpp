#include <iostream>
#include <cstring>
#include <fstream>
#include <list>
#include <limits>
#include <iomanip>

#include "XMLParser.h"

// ----------------------------------------------------------------------------

class GpxRm : public XMLParser
{
public:
  // Constructor
  GpxRm()
  {
  }

  // Deconstructor
  virtual ~GpxRm()
  {
  }

  // Properties
  void setOutputFilename(const std::string &name) { _outputFilename = name; }
  const std::string &outputFilename() const { return _outputFilename; }

  void setWaypointName(const std::string &name) { _waypointName = name; }
  const std::string &waypointName() const { return _waypointName; }

  void setTrackName(const std::string &name) { _trackName = name; }
  const std::string &trackName() const { return _trackName; }

  void setSegmentNr(int nr) { _segmentNr = nr; }
  int segmentNr() const { return _segmentNr; }

  void setRouteName(const std::string &name) { _routeName = name; }
  const std::string &routeName() const { return _routeName; }

  // Parse a file
  bool parseFile(const std::string &name)
  {
    std::ifstream stream(name.c_str());

    if (!stream.is_open()) return false;

    _path.clear();

    std::cout << name << ":" << std::endl;

    parse(stream);

    stream.close();

    return true;
  }


  // Callbacks
  virtual void parseError(const std::string &text, int lineNumber, int columnNumber)
  {
    std::cerr << "  XML parser reports '" << text << "' on line " << lineNumber << ", column " << columnNumber << std::endl;
  }

  virtual void startElement(const std::string &name, const Attributes &atts)
  {
    _path.append("/");
    _path.append(name);

    if (_path == "/gpx/wpt")
    {
    }
    else if (_path == "/gpx/rte")
    {
    }
    else if (_path == "/gpx/rte/rtept")
    {
    }
    else if (_path == "/gpx/trk")
    {
    }
    else if (_path == "/gpx/trk/trkseg")
    {
    }
    else if (_path == "/gpx/trk/trkseg/trkpt")
    {
    }
  }

  virtual void characterData(const std::string &data)
  {
    if (_path == "/gpx/wpt/name")
    {
    }
    else if (_path == "/gpx/wpt/ele")
    {
    }
    else if (_path == "/gpx/wpt/time")
    {
    }
    else if (_path == "/gpx/rte/name")
    {
    }
    else if (_path == "/gpx/trk/name")
    {
    }
    else if (_path == "/gpx/trk/trkseg/trkpt/ele")
    {
    }
    else if (_path == "/gpx/trk/trkseg/trkpt/time")
    {
    }
  }

  virtual void endElement(const std::string &)
  {
    if (_path == "/gpx/wpt")
    {
    }
    else if (_path == "/gpx/rte/rtept")
    {
    }
    else if (_path == "/gpx/rte")
    {
    }
    else if (_path == "/gpx/trk")
    {
    }
    else if (_path == "/gpx/trk/trkseg")
    {
    }
    else if (_path == "/gpx/trk/trkseg/trkpt")
    {
    }

    size_t i;
    if ((i = _path.find_last_of('/')) != std::string::npos) _path.erase(i);
  }

private:
  // Members
  std::string   _outputFilename;
  std::string   _waypointName;
  std::string   _trackName;
  int           _segmentNr; // 1..
  std::string   _routeName;

  std::string   _path;
};

// -- Main program ------------------------------------------------------------

int main(int argc, char *argv[])
{
  GpxRm gpxrm;

  int i = 1;
  while (i < argc)
  {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-?") == 0)
    {
      std::cout << "Usage: gpxrm [-h] [-v] [-w \"name\"] [-t \"<name>\" [-s <segment>]] [-r \"<name>\"] [-o <out.gpx>] <file.gpx>..." << std::endl;
      std::cout << "  -h            help" << std::endl;
      std::cout << "  -v            show version" << std::endl;
      std::cout << "  -w \"name\"     remove the waypoint with <name>" << std::endl;
      std::cout << "  -t \"<name>\"   remove the track with <name>" << std::endl;
      std::cout << "  -s <segment>  remove only segment in track (1..)" << std::endl;
      std::cout << "  -r \"<name>\"   remove the route with <name>" << std::endl;
      std::cout << "  -o <out.gpx>  the output gpx file" << std::endl;
      std::cout << " file.gpx...    the input gpx file" << std::endl << std::endl;
      std::cout << "   Remove a waypoint, track or route from a gpx file" << std::endl;
      return 0;
    }
    else if (strcmp(argv[i], "-v") == 0)
    {
      std::cout << "gpxrm v0.1.0" << std::endl;
      return 0;
    }
    else if (strcmp(argv[i], "-w") == 0 && i < argc)
    {
      gpxrm.setWaypointName(argv[++i]);
    }
    else if (strcmp(argv[i], "-t") == 0 && i < argc)
    {
      gpxrm.setTrackName(argv[++i]);
    }
    else if (strcmp(argv[i], "-s") == 0 && i < argc)
    {
      if (gpxrm.waypointName().empty())
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
      gpxrm.setOutputFilename(argv[++i]);
    }
    else if (argv[i][0] != '-')
    {
      if (!gpxrm.parseFile(argv[i]))
      {
        std::cerr << "Unable to open: " << argv[i] << std::endl;
      }
    }
    else
    {
      std::cerr << "Unknown option:" << argv[i] << std::endl;
      return 1;
    }

    i++;
  }

  return 0;
}

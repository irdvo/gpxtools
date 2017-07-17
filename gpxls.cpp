#include <iostream>
#include <cstring>
#include <fstream>
#include <list>
#include <limits>
#include <iomanip>

#include "XMLParser.h"

// ----------------------------------------------------------------------------

class GpxLs : public XMLParser
{
public:
  // Constructor
  GpxLs() :
    _waypoints(),
    _routes(),
    _tracks()
  {

  }

  // Deconstructor
  virtual ~GpxLs()
  {

  }

  // Properties

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

  // Output the result
  enum Mode { SUMMARY, FULL };

  void report(Mode mode)
  {
    std::cout << "  Waypoints: " << _waypoints.size() << std::endl;

    if (mode == FULL && !_waypoints.empty())
    {
      std::cout << "    Waypoint  Latitude   Longitude Time                     Elevation" << std::setprecision(5) << std::fixed << std::endl;

      for (auto iter = _waypoints.begin(); iter != _waypoints.end(); ++iter)
      {
        std::cout << "    "
                  << std::left << std::setw(10) << iter->_name << std::right
                  << std::setw(10) << iter->_lat
                  << std::setw(10) << iter->_lon
                  << ' ' << std::left << std::setw(24) << iter->_time << std::right
                  << std::setw(10) << iter->_ele << std::endl;
      }
    }

    std::cout << "  Routes:    " << _routes.size() << std::endl;
    for(auto iter = _routes.begin(); iter != _routes.end(); ++iter)
    {
      std::cout << "    Route: '" << iter->_name << "' Points: " << iter->_points.size() << std::endl;

      if (mode == FULL && !iter->_points.empty())
      {
        std::cout << "      Latitude   Longitude" << std::setprecision(5) << std::fixed << std::endl;

        for (auto iter2 = iter->_points.begin(); iter2 != iter->_points.end(); ++iter2)
        {
          std::cout << "      "
                    << std::setw(10) << iter2->_lat
                    << std::setw(10) << iter2->_lon << std::endl;
        }
      }
    }

    std::cout << "  Tracks:   " << _tracks.size() << std::endl;
    for (auto iter = _tracks.begin(); iter != _tracks.end(); ++iter)
    {
      std::cout << "    Track: '" << iter->_name << "' Segments: " << iter->_segments.size() << std::endl;

      int i = 1;
      for (auto iter2 = iter->_segments.begin(); iter2 != iter->_segments.end(); ++iter2, i++)
      {
        std::cout << "      Segment: " << i << " Points: " << iter2->_points.size() << std::endl;

        if (mode == FULL && !iter2->_points.empty())
        {
          std::cout << "        Latitude   Longitude Time                     Elevation" << std::setprecision(5) << std::fixed << std::endl;

          for (auto iter3 = iter2->_points.begin(); iter3 != iter2->_points.end(); ++iter3)
          {
            std::cout << "        "
                      << std::setw(10) << iter3->_lat
                      << std::setw(10) << iter3->_lon
                      << ' ' << std::left << std::setw(24) << iter3->_time << std::right
                      << std::setw(10) << iter3->_ele << std::endl;
          }
        }
      }
    }
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
      _waypoint.reset();

      _waypoint._lat = getDoubleAttribute(atts, "lat");
      _waypoint._lon = getDoubleAttribute(atts, "lon");
    }
    else if (_path == "/gpx/rte")
    {
      _route.reset();
    }
    else if (_path == "/gpx/rte/rtept")
    {
      _routepoint.reset();

      _routepoint._lat = getDoubleAttribute(atts, "lat");
      _routepoint._lon = getDoubleAttribute(atts, "lon");
    }
    else if (_path == "/gpx/trk")
    {
      _track.reset();
    }
    else if (_path == "/gpx/trk/trkseg")
    {
      _trackSegment.reset();
    }
    else if (_path == "/gpx/trk/trkseg/trkpt")
    {
      _trackpoint.reset();

      _trackpoint._lat = getDoubleAttribute(atts, "lat");
      _trackpoint._lon = getDoubleAttribute(atts, "lon");
    }
  }

  virtual void characterData(const std::string &data)
  {
    if (_path == "/gpx/wpt/name")
    {
      _waypoint._name.append(data);
    }
    else if (_path == "/gpx/wpt/ele")
    {
      _waypoint._ele = getDouble(data);
    }
    else if (_path == "/gpx/wpt/time")
    {
      _waypoint._time.append(data);
    }
    else if (_path == "/gpx/rte/name")
    {
      _route._name.append(data);
    }
    else if (_path == "/gpx/trk/name")
    {
      _track._name.append(data);
    }
    else if (_path == "/gpx/trk/trkseg/trkpt/ele")
    {
      _trackpoint._ele = getDouble(data);
    }
    else if (_path == "/gpx/trk/trkseg/trkpt/time")
    {
      _trackpoint._time.append(data);
    }
  }

  virtual void endElement(const std::string &)
  {
    if (_path == "/gpx/wpt")
    {
      _waypoints.push_back(_waypoint);
    }
    else if (_path == "/gpx/rte/rtept")
    {
      _route._points.push_back(_routepoint);
    }
    else if (_path == "/gpx/rte")
    {
      _routes.push_back(_route);
    }
    else if (_path == "/gpx/trk")
    {
      _tracks.push_back(_track);
    }
    else if (_path == "/gpx/trk/trkseg")
    {
      _track._segments.push_back(_trackSegment);
    }
    else if (_path == "/gpx/trk/trkseg/trkpt")
    {
      _trackSegment._points.push_back(_trackpoint);
    }

    size_t i;
    if ((i = _path.find_last_of('/')) != std::string::npos) _path.erase(i);
  }

private:
  double getDouble(const std::string &value)
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

  double getDoubleAttribute(const Attributes &atts, const std::string &key)
  {
    auto iter = atts.find(key);

    return iter != atts.end() ? getDouble(iter->second) : std::numeric_limits<double>::min();
  }

  // Members
  std::string   _path;

  struct Waypoint
  {
    void reset()
    {
      _name.clear();
      _lat = std::numeric_limits<double>::min();
      _lon = std::numeric_limits<double>::min();
      _ele = std::numeric_limits<double>::min();
      _time.clear();
    }

    std::string _name;
    double      _lat;
    double      _lon;
    double      _ele;
    std::string _time;
  };
  Waypoint _waypoint;

  std::list<Waypoint> _waypoints;

  struct Routepoint
  {
    void reset()
    {
      _lat = std::numeric_limits<double>::min();
      _lon = std::numeric_limits<double>::min();
    }

    double      _lat;
    double      _lon;
  };
  Routepoint _routepoint;

  struct Route
  {
    void reset()
    {
      _name.clear();
      _points.clear();
    }

    std::string           _name;
    std::list<Routepoint> _points;
  };
  Route _route;

  std::list<Route> _routes;

  struct Trackpoint
  {
    void reset()
    {
      _lat = std::numeric_limits<double>::min();
      _lon = std::numeric_limits<double>::min();
      _ele = std::numeric_limits<double>::min();
      _time.clear();
    }

    double      _lat;
    double      _lon;
    double      _ele;
    std::string _time;
  };
  Trackpoint _trackpoint;

  struct TrackSegment
  {
    void reset()
    {
      _points.clear();
    }

    std::list<Trackpoint> _points;
  };
  TrackSegment _trackSegment;

  struct Track
  {
    void reset()
    {
      _name.clear();
      _segments.clear();
    }

    std::string             _name;
    std::list<TrackSegment> _segments;
  };
  Track _track;

  std::list<Track> _tracks;
};

// -- Main program ------------------------------------------------------------

int main(int argc, char *argv[])
{
  GpxLs::Mode mode = GpxLs::SUMMARY;

  for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-?") == 0)
    {
      std::cout << "Usage: gpxls [-H] [-V] [-s] [-f] <file.gpx> .." << std::endl;
      std::cout << "  -h           help" << std::endl;
      std::cout << "  -v           show version" << std::endl;
      std::cout << "  -s           show summary (default)" << std::endl;
      std::cout << "  -f           show full info" << std::endl;
      std::cout << " file.gpx ..   the input gpx files" << std::endl << std::endl;
      std::cout << "   List gpx file" << std::endl;
      break;
    }
    else if (strcmp(argv[i], "-v") == 0)
    {
      std::cout << "gpxls v0.1.0" << std::endl;
      break;
    }
    else if (strcmp(argv[i], "-s") == 0)
    {
      mode = GpxLs::SUMMARY;
    }
    else if (strcmp(argv[i], "-f") == 0)
    {
      mode = GpxLs::FULL;
    }
    else if (argv[i][0] != '-')
    {
      GpxLs gpxls;

      if (gpxls.parseFile(argv[i]))
      {
        gpxls.report(mode);
      }
      else
      {
        std::cerr << "Unable to open: " << argv[i] << std::endl;
      }
    }
    else
    {
      std::cerr << "Unknown option:" << argv[i] << std::endl;
    }
  }

  return 0;
}

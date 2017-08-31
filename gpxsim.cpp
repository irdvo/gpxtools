#include <iostream>
#include <cstring>
#include <fstream>
#include <cmath>

#include "XMLParser.h"

const std::string version= "0.1.0";
// ----------------------------------------------------------------------------

class GpxSim : public XMLParserHandler
{
public:
  // -- Constructor -----------------------------------------------------------
  GpxSim() :
    _outputFile(&std::cout)
  {
  }

  // -- Deconstructor ---------------------------------------------------------
  virtual ~GpxSim()
  {
  }

  // -- Properties ------------------------------------------------------------

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
  static double distance(double lat1deg, double lon1deg, double lat2deg, double lon2deg)
  {
    const double R = 6371E3; // metres
    
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

  // bearing in rads
  static double bearing(double lat1deg, double lon1deg, double lat2deg, double lon2deg)
  {
    const double R = 6371E3; // metres
    
    double lat1rad = deg2rad(lat1deg);
    double lon1rad = deg2rad(lon1deg);
    double lat2rad = deg2rad(lat2deg);
    double lon2rad = deg2rad(lon2deg);
    
    double dlon    = lon2rad - lon1rad;
    
    double y       = sin(dlon) * cos(lat2rad);
    double x       = cos(lat1rad) * sin(lat2rad) - sin(lat1rad) * cos(lat2rad) * cos(dlon);
    
    return atan2(y, x);

  }

  // crosstrack distance in metres from point3 to the point1-point2 line
  static double crosstrack(double lat1deg, double lon1deg, double lat2deg, double lon2deg, double lat3deg, double lon3deg) 
  {
    const double R = 6371E3; // metres
    
    double distance13 = distance(lat1deg, lon1deg, lat3deg, lon3deg) / R;
    double bearing13  = bearing(lat1deg, lon1deg, lat3deg, lon3deg);
    double bearing12  = bearing(lat1deg, lon1deg, lat2deg, lon2deg);
    
    return asin(sin(distance13) * sin(bearing13 - bearing12)) * R;
  }

private:
  void log(const std::string &text)
  {
    if (false) /// TODO
    {
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

    if (_path == "/gpx/trk/trkseg")
    {
      /// TODO
    }
  }

  void doEndElement()
  {
    if (_path == "/gpx/trk/trkseg")
    {
      /// TODO
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

  std::string   _path;
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
      std::cout << "  -i              report the results of the simplification" << std::endl;
      std::cout << "  -d <distance>   remove route or track points within distance of a point (in cm)" << std::endl;
      std::cout << "  -n <number>     remove route or track points until the route or track contains <number> points" << std::endl;
      std::cout << "  -x <distance>   remove route or track points with a cross track distance less than <distance> (in cm)" << std::endl;
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
    else if (strcmp(argv[i], "-i") == 0 && i < argc)
    {
      /// TODO
    }
    else if (strcmp(argv[i], "-d") == 0 && i < argc)
    {
      /// TODO
    }
    else if (strcmp(argv[i], "-n") == 0 && i < argc)
    {
      /// TODO
    }
    else if (strcmp(argv[i], "-x") == 0 && i < argc)
    {
      /// TODO
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
  
  std::cout << "Distance:  " << GpxSim::distance(50.06639, -5.71472, 58.64389, -3.07000) << std::endl;
  std::cout << "Bearing:   " << GpxSim::bearing(50.06639, -5.71472, 58.64389, -3.07000) << std::endl;
  std::cout << "Crosstrack:" << GpxSim::crosstrack(53.3206, -1.7297, 53.1887, 0.1334, 53.2611, -0.7972) << std::endl;

  return 0;
}

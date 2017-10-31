#include <iostream>
#include <sstream>
#include <cstring>
#include <cctype>
#include <iomanip>
#include <cmath>

const std::string tool    = "gpxformat";
const std::string version = "0.1.0";

// -- Is the next char present in matchers ------------------------------------

bool isChars(char ch, const std::string &matchers)
{
  for (auto iter = matchers.begin(); iter != matchers.end(); ++iter)
  {
    if (ch == *iter) return true;
  }

  return false;
}

// -- Skip spaces -------------------------------------------------------------

void skipSpaces(const std::string &input, std::size_t &i)
{
  const std::size_t length = input.length();

  while (i < length && isspace(input[i]))
  {
    i++;
  }
}

// -- Scan a value ------------------------------------------------------------

bool scanValue(const std::string &input, std::size_t &i, double &value, bool &withPoint)
{
  const std::size_t length = input.length();

  if (i < length && !isdigit(input[i])) return false;

  std::string text;

  while (i < length && isdigit(input[i]))
  {
    text += input[i++];
  }

  withPoint = (i < length && input[i] == '.');

  if (withPoint)
  {
    text += input[i++];

    while (i < length && isdigit(input[i]))
    {
      text += input[i++];
    }
  }

  value = std::stod(text);

  return true;
}

// -- Scan the coordinate -----------------------------------------------------

bool scanCoordinate(const std::string &input, std::size_t &i, const std::string &positives, const std::string &negatives, double &degrees)
{
  bool        negative  = false;
  bool        withSign  = false;
  const
  std::size_t length    = input.length();

  degrees = 0.0;

  skipSpaces(input, i);

  if (i < length && isChars(input[i], positives))
  {
    i++;
    withSign = true;

    skipSpaces(input, i);
  }
  else if (i < length && isChars(input[i], negatives))
  {
    i++;
    negative = true;
    withSign = true;

    skipSpaces(input, i);
  }
  else if (i < length && input[i] == '-')
  {
    i++;
    negative = true;
    withSign = true;
  }

  double value = 0.0;
  bool   withPoint = false;

  if (!scanValue(input, i, value, withPoint)) return false;

  degrees = value;

  skipSpaces(input, i);

  if (!withPoint && scanValue(input, i, value, withPoint))
  {
    degrees += (value / 60.0);
  }

  skipSpaces(input, i);

  if (!withPoint && scanValue(input, i, value, withPoint))
  {
    degrees += (value / 3600.0);
  }

  skipSpaces(input, i);

  if (!withSign && i < length)
  {
    if (isChars(input[i], positives))
    {
      i++;
    }
    else if (isChars(input[i], negatives))
    {
      i++;
      negative = true;
    }
  }

  skipSpaces(input, i);

  if (i < length)
  {
    if (input[i] != ',' && input[i] != ';') return false;

    i++;
  }

  if (negative) degrees = -degrees;

  return true;
}

// -- Output ------------------------------------------------------------------

void output(double latitude, double longitude, const std::string &browser)
{
  std::cout << "Results:" << std::endl;

  std::cout << std::setprecision(8) << std::fixed;
  std::cout << "  " << latitude << ", " << longitude << std::endl;

  std::string ns = (latitude >= 0  ? "N" : "S");
  std::string ew = (longitude >= 0 ? "E" : "W");

  double degLat, degLon;
  double minLat, minLon;

  minLat = std::modf(std::abs(latitude), &degLat) * 60.0;
  minLon = std::modf(std::abs(longitude), &degLon) * 60.0;

  std::cout << std::setprecision(6) << std::fixed;
  std::cout << "  " << ns << " " << (int) degLat << " " << minLat << ", " << ew << " " << (int) degLon << " " << minLon << std::endl;

  double secLat, secLon;

  secLat = std::modf(minLat, &minLat) * 60.0;
  secLon = std::modf(minLon, &minLon) * 60.0;

  std::cout << std::setprecision(4) << std::fixed;
  std::cout << "  " << ns << " " << (int) degLat << " " << (int) minLat << " " << secLat << ", " << ew << " " << (int) degLon << " " << (int) minLon << " " << secLon << std::endl;

  if (!browser.empty())
  {
    std::stringstream cmd;

    cmd << std::fixed;

    cmd << browser
        << " \"http://www.openstreetmap.org/?mlat=" << std::setprecision(6) << latitude
        << "&mlon=" << longitude
        << "#map=16/" << std::setprecision(4) << latitude << "/" << longitude << "\"";

    system(cmd.str().c_str());
  }
}

// -- Process the input -------------------------------------------------------

bool process(const std::string input, const std::string &browser)
{
  double latitude  = 0.0;

  double longitude = 0.0;

  std::size_t i    = 0;

  if (!scanCoordinate(input, i, "Nn", "Ss", latitude)) return false;

  if (!scanCoordinate(input, i, "Ee", "Ww", longitude)) return false;

  if (latitude < -90.0   || latitude >  90.0)  return false;

  if (longitude < -180.0 || longitude > 180.0) return false;

  output(latitude, longitude, browser);

  return true;
}

// -- Main program ------------------------------------------------------------

int main(int argc, char *argv[])
{
  std::string browser;

  int i = 1;
  while (i < argc)
  {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-?") == 0)
    {
      std::cout << "Usage: " << tool << " [-h] [-v] [-b browser] \"<lat>,<lon>\" .." << std::endl;
      std::cout << "  -h                   help" << std::endl;
      std::cout << "  -v                   show version" << std::endl;
      std::cout << "  -b browser           set the browser (def. none)" << std::endl;
      std::cout << "  \"lat, lon\"           the gps coordinates" << std::endl << std::endl;
      std::cout << "   Show all formats for gps coordinates and optional the location in a browser." << std::endl << std::endl;
      std::cout << "Examples for gps coordinate formats:" << std::endl;
      std::cout << "         51.90540,     4.46660" << std::endl;
      std::cout << "        -51.90540,    -4.46660" << std::endl;
      std::cout << "      N 51 54.324,  E 4 27.996" << std::endl;
      std::cout << "      51 54.324 S,  4 27.996 W" << std::endl;
      std::cout << "     N 51 54 19.4, E 4 27 59.8" << std::endl;
      std::cout << "     51 54 19.4 S, 4 27 59.8 W" << std::endl;
      return 0;
    }
    else if (strcmp(argv[i], "-v") == 0)
    {
      std::cout << tool << " v" << version << std::endl;
      return 0;
    }
    else if (strcmp(argv[i], "-b") == 0 && i+1 < argc)
    {
      browser = argv[++i];
    }
    else
    {
      if (!process(argv[i], browser))
      {
        std::cerr << "Error: " << argv[i] << " has not a valid format." << std::endl;
      }
    }

    i++;
  }

  return 0;
}

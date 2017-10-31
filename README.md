# gpxtools
A collection of c++ tools for using GPX files 

## gpxls

A c++ tool for showing a summary of the contents of a GPX file.

Syntax:
```
  Usage: gpxls [-h] [-v] [-s] [-f] <file.gpx> ..
    -h           help
    -v           show version
    -s           show summary (default)
    -f           show full info
   file.gpx ..   the input gpx files
```

Examples:
```
  gpxls -s track.gpx

    Show the waypoint, route, track and track segment information in the track.gpx file.

  gpxls -f track.gpx

    Show the waypoints, route, route points, track, tracksegment and track points in the track.gpx file.
```

Requirements:
  * [cmake](https://cmake.org/) for building

---

## gpxrm

A c++ tool for removing a waypoint, a route or track (with optional a segment) from a GPX file. Use with
gpxls for looking up the names of waypoints, routes and tracks.

Syntax:
```
  Usage: gpxrm [-h] [-v] [-w "<name>"] [-t "<name>" [-s <segment>]] [-r "<name>"] [-o <out.gpx>] <file.gpx>
    -h              help
    -v              show version
    -w "name"       remove the waypoint with <name>
    -t "<name>"     remove the track with <name>
    -s <segment>    remove only segment in track (1..)
    -r "<name>"     remove the route with <name>
    -o <out.gpx>    the output gpx file (overwrites existing file)
   file.gpx         the input gpx file

     Remove a waypoint, track or route from a gpx file
```

Examples:
```
  gpxrm -w "Waypoint1" -o output.gpx track.gpx
  
    Copy the contents of track.gpx to output.gpx without waypoint "Waypoint1".
    
  gpxrm -t "Track1" track.gpx
  
    Copy the contents of track.gpx to the console without track "Track1".
    
  gpxrm -t "Track2" -s 3 -o output.gpx track.gpx
  
    Copy the contents of track.gpx to output.gpx without the third segment of the track "Track1."
```

Requirements:
  * [cmake](https://cmake.org/) for building

## gpxsim

A c++ tool for simplifing track segments or routes in a GPX file using the distance threshold and/or the
Douglas-Peucker algorithm.

Syntax:
```
  Usage: gpxsim [-h] [-v] [-i] [-d <distance>] [-n <number> | -t <distance>] [-o <out.gpx>] <file.gpx>
    -h              help
    -v              show version
    -i              report the results of the simplification (only with -o)
    -d <distance>   remove route or track points within distance of the previous point (in m)
    -n <number>     remove route or track points until the route or track contains <number> points (2..)
    -x <distance>   remove route or track points with a cross track distance less than <distance> (in m)
    -o <out.gpx>    the output gpx file (overwrites existing file)
   file.gpx         the input gpx file
   
    Simplify a route or track using the distance threshold and/or the Douglas-Peucker algorithm.
```

Examples:
```
  gpxsim -i -d 2 -o output.gpx track.gpx
  
    Simplify the routes and tracks segments in track.gpx by removing all points closer than 2 metres from 
    its predecessor point and store the result in output.gpx. Report the result of the simplification.
    
  gpxsim -n 100 route.gpx
  
    Simplify the routes in route.gpx by removing points with the smallest crosstrack error until the routes 
    contains 100 points. The resulting output is copied to the console.
    
  gpxsim -i -x 2.1 -o output.gpx track.gpx
  
    Simplify the tracks segments in track.gpx by removing points with a crosstrack error smaller than 2.1 metres.
    Store the result in output.gpx and report the result of the simplification.
```

Requirements:
  * [cmake](https://cmake.org/) for building

## gpxjson

A c++ tool for converting the waypoints, routes and/or tracks in a GPX file to a GeoJson file.

Syntax:
```
  Usage: gpxjson [-h] [-v] [-w] [-r] [-t] [-m compact|normal] [-n <number>] [-o <out.json>] [<file.gpx>]
    -h                   help
    -v                   show version
    -w                   convert the waypoints
    -t                   convert the tracks
    -r                   convert the routes
    -m compact|normal    set the output mode
    -n <number>          set the number of points per line (in normal mode) (def. 4)
    -o <out.json>        the output json file (overwrites existing file)
   file.gpx              the input gpx file

     Convert a gpx file to GeoJson.
```

Examples:
```
  gpxjson -w -m compact -o output.json track.gpx
  
    Convert the waypoins in the track.gpx file to Multipoints in the output.json file. All points are on the 
    same line (compact mode).
    
  gpxjson -t -m normal -n -o output.json track.gpx
  
    Convert the track points in the track.gpx to MultiLinestrings in the output.json file. All points are 
    grouped by 6 per line (normal mode).
    
  gpxjson -r -m normal 
  
    Read the track points from standard in and convert them to MultiLinestrings that are written to 
    standard out in normal mode (4 points per line).
```

Requirements:
  * [cmake](https://cmake.org/) for building

## gpxformat

A c++ tool for formatting gpx coordinates and optionally showing the location in a browser.

Syntax:
```
  Usage: gpxformat [-h] [-v] [-b browser] "<lat>,<lon>" ..
    -h                   help
    -v                   show version
    -b browser           set the browser (def. none)
    "lat, lon"           the gps coordinates

     Show all formats for gps coordinates and optional the location in a browser.
     
  Examples for gps coordinate formats:
           51.90540,     4.46660
          -51.90540,    -4.46660
        N 51 54.324,  E 4 27.996
        51 54.324 S,  4 27.996 W
       N 51 54 19.4, E 4 27 59.8
       51 54 19.4 S, 4 27 59.8 W
```

Examples:
```
  gpxformat "51.90540,4.46660"
  
    Show all formats for the gps coordinates 51.90540,4.46660.
    
  gpxformat -b firefox "51 54.324 N,4 27.996 E"
  
    Show all formats for the gps coordinates 51 54.324 N,4 27.996 E and show the location in the browser.
```

Requirements:
  * [cmake](https://cmake.org/) for building

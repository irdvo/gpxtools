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

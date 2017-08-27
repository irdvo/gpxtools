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


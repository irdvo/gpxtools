# gpxtools

This project is *discontinued* here. See [gpxtools](https://notabug.org/irdvo/gpxtools) for the continuation.

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
    
  gpxjson -t -m normal -n 6 -o output.json track.gpx
  
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

## gpxcat

A c++ tool for concatenating the track segments in a track in a GPX file. Output is written to standard output.

Syntax:
```
   Usage: gpxcat [-h] [-v] [-d <distance>] <file.gpx> ..
    -h              help
    -v              show version
    -d <distance>   concatenate only if the distance between the end and
                    the start of the segments are less than distance (metres)
   file.gpx         the input gpx file

     Concatenate the segments in the track in the GPX input file.
```

Examples:
```
  gpxcat input.gpx
  
    All track segments are concatenated per track and written to standard output.
    
  gpxcat -d 12.0 input.gpx
  
    All track segments for which the distance between the end of the segment and the start
    of the next segment is not bigger than the distance parameter are concatenated per track.
```

Requirements:
  * [cmake](https://cmake.org/) for building

## gpxsplit

A c++ tool for splitting a track segment in multiple track segements based on distance or time.

Syntax:
```
  Usage: gpxsplit [-h] [-v] [-a] [-d <distance>] [-t "<time>"] [-s <seconds>] [-m <minutes>] [-u <hours>] [-o <out.gpx>] [<file.gpx>]
    -h                   help
    -v                   show version
    -a                   analyse the file for splitting
    -d <distance>        split based on distance in metres
    -t "<time>"          split based on time, format: yyyy-mm-dd hh:mm:ss
    -s <duration>        split based on time duration, in seconds
    -m <duration>        split based on time duration, in minutes
    -u <duration>        split based on time duration, in hours
    -o <out.gpx>         the output gpx file (overwrites existing file)
   file.gpx              the input gpx file

     Split the track segments in a gpx in multiple track segments based on distance or time.
```

Examples:
```
  gpxsplit -a -d 4000 input.gpx
  
    Analyse the input.gpx file for splitting the track segments based on the distance between two track points.
    If the distance is more than 4km a message is written. No output.gpx is written.
    
  gpxsplit -t "2018-01-01 14:30:20" -o output.gpx input.gpx
  
    Split the track segments in input.gpx. The segment is splitted based on time: points before the time and points after
    the time. The splitted track segments are written to output.gpx
    
  gpxsplit -m 90 input.gpx
  
    The track points in the track segments in the input.gpx file are splitted based on the time duration between
    two points. If this is more than 90 minutes, the track segment is splitted in two. Output is written to
    standard out.
```

Requirements:
  * [cmake](https://cmake.org/) for building

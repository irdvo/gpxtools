# gpxtools
A collection of c++ tools for using GPX files 

## gpxls

A c++ tool for showing a summary of the contents of a GPX file

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
--
    Show the waypoint, route, track and track segment information in the track.gpx file.
--
  gpxls -f track.gpx
--
    Show the waypoints, route, route points, track, tracksegment and track points in the track.gpx file.
```

Requirements:
  * [cmake](https://cmake.org/) for building
  * [expat](https://libexpat.github.io/)

---


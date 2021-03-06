# cdrone
Drone logic in C/C++. This project is composed of two sub projects: cdrone and gobase. cdrone is written in C/C++ to be efficient. gobase is written in go, and does not need to be as efficient as its cdrone counterpart as it runs on a capable PC. This project has been tested on a minimal Arch linux for raspberry PI.

## Build
The recommended build is through a docker container that already has the
dependencies. Just run 
```
make
```
from the top directory.

## Get Dependencies
Most likely the libraries on the drone will not exist or they will be on the
wrong version. To copy the needed library files from the docker container, run 
```
make pkg
```
and copy the files from `cdrone/pkg` to the drone. A good place to copy the
files is `/usr/lib`.

## References
 * `cdrone/json/` contains files from the jsoncpp project.
 * `cdrone/hardware` contains objects that hook up to hardware on the drone.
 * `cdrone/misc` contains uncategorized functions/objects that are used
   throughout the project.
 * `cdrone/program` holds testing code and the main cdrone/ runtime code.
 * `cdrone/proto` holds protoc generated files.
 * `cdrone/controller` contains the main controller code for the drone.
 * `cdrone/wire` holds wire protocols, and syscall nightmares.
 * `proto` holds the message structure of the messages passed between the drone
   and the base station.

## TODO 
 * clean up the gobase project.
 * move to c++ time from c time.
 * use https://bitbucket.org/eigen/eigen for quaternions.
 * write a more efficient Watchdog library. Right now it has its own thread that sleeps and wakes. This library would be way more efficient if it could interrupt and signal when it needed to, not every 100ms. 
 * add better dependency graph to the cdrone makefile.
 * better algorithms for control.
 * tear apart mmal because it is a resource heavy library. It creates new
   threads for components that probably don't need threads.
 * add TLS support.

# cdrone
Drone logic in C/C++. This project is composed of two sub projects: cdrone and gobase. cdrone is written in C/C++ to be efficient, especially for fewer resources. run on the drone for co.

## Depends
 * spdlog for logging
 * protobuf

In Tree:
 * jsoncpp

## References
 * `json/` contains files from the jsoncpp project.

## TODO 
 * use https://bitbucket.org/eigen/eigen for quaternions.
 * write a more efficient Watchdog library. Right now it has its own thread that sleeps and wakes. This library would be way more efficient if it could interrupt and signal when it needed to, not every 100ms. 
 * use a logging library that meshes better with std::atomics... i.e. does not
   overwrite arbitrary memory on the stack. 

# atmotube-reader

This is an application which can be used to get the status from an
Atmotube (http://atmotube.com/) and save it into a database.

Currently this is a development project and it does not work outside
of my testing environment.

# Dependencies

- CMake for building.
- LibConfuse for configuration files.
- GattLib for accessing BLE devices.

# Configuration

Example file: test/config.txt

The application uses: ~/.atmotube/config.

# TODO

- Write more unittests.
- Implement DB plugin using some simple database which can be accessed from JS.
- Clean up the API.
- Implement public vs private API (for internal use and for testing).

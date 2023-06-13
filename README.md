# cpp-simple-vector

Final student project: own container vector

# About

In this project, I worked on creating my own vector class, using constructors and destructors, templates, interaction with memory and move semantics, and covering all methods with tests.

# Deployment Instructions and System Requirements

The project is written in VS Code on Windows and uses C++17, so to run it you need:
1. IDE
2. Compiler

Deployment example:
* Download and install VS Code [code.visualstudio.com](https://code.visualstudio.com/)
* Install the C++ extension in VS Code
* Install the latest MinGW release [winlibs.com](https://winlibs.com/#download-release) and add it to PATH
* Clone the repository
* Run main.cpp file with g++ build
* Output should look like this:

```

    Test with temporary object, copy elision
    Done!

    Test with temporary object, operator=
    Done!

    Test with named object, move constructor
    Done!

    Test with named object, operator=
    Done!

    Test noncopiable object, move constructor
    Done!

    Test noncopiable push back
    Done!

    Test noncopiable insert
    Done!

    Test noncopiable erase
    Done!

```
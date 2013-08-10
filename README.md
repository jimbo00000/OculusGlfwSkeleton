# OculusGlfwSkeleton

## Description

This app skeleton assuages some of the difficulties inherent in developing graphical apps for HMDs. Running a typical Rift app with cloned displays outputs barrel-distorted stereo pairs to the primary monitor, introduces latency, confounds debugging, and makes the Rift display that annoying blue square resolution message for five seconds on every mode change.

This multi-window skeleton allows a programmer to run and debug using their normal workflow, displaying the same scene visible to the Rift user in an operator window from a third-person perspective. This operator window is resizable and includes an AntTweakBar for modifying variables on the fly. Scene display in the operator window can be toggled with the z key. Barrel-distorted post-processed video is displayed to the Rift throughout the duration of the app, allowing the developer to quickly glance into the VR space and verify correct display.


## Setup

### Windows

Set your display to "Extend" using WinKey + P. Position your Rift DK monitor immediately to the right of your primary display, with the top edges of the desktops aligned. The skeleton creates a non-fullscreen window to fill the desktop area of the extended display(using a fullscreen window on an extended display, Windows will automatically minimize that window on the first input event outside of that window).


## Build

### Windows

    Create the directory build/ in project's home(alongside CMakeLists.txt)
    Shift+right-click it in Explorer->"Open command window here"
    ...\build> cmake .. -G "Visual Studio 10"
    Double-click the only .sln file in build to open it in Visual Studio
    Right-click the GLSkeleton project in Solution Explorer, "Set as StartUp Project"
    F7 to build, F5 to build and run

### Linux

    $> mkdir build
    $> cd build
    $> cmake .. && make
    $> ./GLSkeleton


### Thanks

elmindreda for the awesome Glfw3 framework which makes multi-window GL apps possible https://github.com/glfw/glfw
Palmer Luckey for the Oculus Rift
Philip Rideout for the excellent CMake/OpenGL code

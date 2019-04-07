# libazul

![AppVeyor build status](https://ci.appveyor.com/api/projects/status/github/MichaelE1000/libazul?branch=master&svg=true "AppVeyor Build Status")

libazul is a collection of C++ framework components.

Main goals of this framework are: 
* Applying best practices to a small, manageable project
* Implementing modern C++ features while keeping it as portable as possible
* Target a modern C++ standard (currently C++17)
* Supporting all important platforms: Windows (x86_64, x86), Linux (x86_64), OSX (x86_64), iOS (arm64), Android (arm64)

# Framework Components

#### IPC

The inter process component contains a robust mutex and condition variable implementation. For a project I needed to synchronize two processes and found a lot of wrong information online. This is a new implementation based on [a paper written by Andrew D. Birrell (Microsoft Research, 2003)](http://birrell.org/andrew/papers/ImplementingCVs.pdf). This paper explains perfectly what issues one needs to consider.

Since a lot of this code depends on operating system features, different implementations for the supported platforms had to be added. The Windows implementation is based on the mentioned paper. The linux implementation is a wrapper around the pthread mutex and pthread condition variable. The OSX implementation is slightly more complicated but followes the basic algorithm of the windows variant.

Note: Obviously this component is not availalbe on iOS/Android since it is not very useful there.

#### COMPUTE

This component contains some experiments related to opencl. The current clcpp subfolder contains some headers which implement a simple approach to compile opencl code as C++. This approach only works with very simple kernels (e.g. no memory barries) but that should already be enough for a wide range of algorithms. Use the set_global_id function to set the global index and afterwards call the kernel function. The global index is stored in a thread local variable and the kernel functions can be called simultaneously from multiple threads. To automate the process of calling a kernel function, the OpenClComputeExecutor combined with the StaticThreadPool from the async component can be used. This automates the whole process of scheduling all work items manually.

#### ASYNC

A library providing components for async programming. Currently contains a future with some extensions from the concurrency TS and a thread pool.

#### More to come ...

...

# FAQ

* *What about the Raspberry PI?* Even so the RPI is a widely used platform, supporting it is tough. The official cross compiler is still stuck at gcc 4.9, meaning C++11. To build this project a newer toolchain would be required. That said a modern enough compiler should be all that's needed to compile this code for the raspberry PI.

## Installing Build Tools & Downloading Dependencies

#### Linux
   
    python3 3rdparty/download.py          # Download 3rd-party libraries (googletest, ios-cmake, android-ndk)
    python3 3rdparty/download.py -m       # (alternative) -m tells the script to download the android ndk
    sudo dnf install cmake gcc-c++        # Installing compiler, cmake and assembler

#### OSX

    python3 3rdparty/download.py -m       # The -m argument is optional, it downloads some ios cmake files.
    brew install gcc
    brew upgrade cmake

    # optional, in case you get an error message about the command line tools being selected
    sudo xcode-select -s /Applications/Xcode.app/Contents/Developer

#### Windows

    python 3rdparty/download.py

## Compilation

#### Linux

    mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF ..
    make -j8

#### Android (on Linux)

    mkdir build_android && cd build_android
    cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF .. -DANDROID_NDK=`pwd`/../3rdparty/android-ndk-r19c/ -DCMAKE_TOOLCHAIN_FILE=`pwd`/../3rdparty/android-ndk-r19c/build/cmake/android.toolchain.cmake -DANDROID_TOOLCHAIN=clang -DANDROID_ABI=arm64-v8a -DANDROID_NATIVE_API_LEVEL=24
    make -j8

#### OSX

    mkdir build && cd build
    cmake -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF ..
    make -j8

#### iOS (on OSX)

    mkdir build_ios && cd build_ios
    cmake -GXcode -DBUILD_SHARED_LIBS=OFF .. -DCMAKE_TOOLCHAIN_FILE=`pwd`/../3rdparty/ios-cmake/ios.toolchain.cmake -DPLATFORM=OS64 -DENABLE_VISIBILITY=1 -DDISABLE_TESTS=ON
    xcodebuild -project azul.xcodeproj build -configuration Release

#### Windows

    mkdir build && cd build
    cmake -G"Visual Studio 16 2019" -Ax64 -DBUILD_SHARED_LIBS=OFF ..
    cmake --build . --config Release --target ALL_BUILD



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

## Downloading Dependencies

    # this will clone the submodules pointing to google/googletest and leetal/ios-cmake
    git submodule init
    git submodule update

## Compilation

#### Linux

    mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF ..
    make -j8

#### Android (on Linux)

    export ANDROID_NDK_HOME=/your/ndk/path/
    mkdir build_android && cd build_android
    cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF .. -DANDROID_NDK=$ANDROID_NDK_HOME -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake -DANDROID_TOOLCHAIN=clang -DANDROID_ABI=arm64-v8a -DANDROID_NATIVE_API_LEVEL=28
    make -j8

#### OSX

    brew install gcc
    brew upgrade cmake

    # optional, in case you get an error message about the command line tools
    sudo xcode-select -s /Applications/Xcode.app/Contents/Developer

    mkdir build && cd build
    cmake -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF ..
    make -j8

#### iOS (on OSX)

    mkdir build_ios && cd build_ios
    cmake -GXcode -DBUILD_SHARED_LIBS=OFF .. -DCMAKE_TOOLCHAIN_FILE=`pwd`/../3rdparty/ios-cmake/ios.toolchain.cmake -DPLATFORM=OS64 -DENABLE_VISIBILITY=1
    xcodebuild -project azul.xcodeproj build -configuration Release

#### Windows

    mkdir build && cd build
    cmake -G"Visual Studio 16 2019" -Ax64 -Thost=x64 -DBUILD_SHARED_LIBS=OFF ..
    cmake --build . --config Release --target ALL_BUILD



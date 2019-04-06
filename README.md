# libimpulso

![AppVeyor build status](https://ci.appveyor.com/api/projects/status/github/MichaelE1000/libimpulso?branch=master&svg=true "AppVeyor Build Status")

libimpulso is a collection of C++ framework components.

Main goals of this framework are: 
* Applying best practices to a small, manageable project
* Implementing modern C++ features while keeping it as portable as possible
* Target a modern C++ standard (currently C++17)
* Supporting all important platforms: Windows (x86_64, x86), Linux (x86_64), OSX (x86_64), iOS (arm64), Android (arm64, arm)

# Framework Components

#### IPC

The inter process component contains a robust mutex and condition variable implementation. For a project I needed to synchronize two processes and found a lot of wrong information online. This is a new implementation based on [a paper written by Andrew D. Birrell (Microsoft Research, 2003)](http://birrell.org/andrew/papers/ImplementingCVs.pdf). This paper explains perfectly what issues one needs to consider.

Since a lot of this code depends on operating system features, different implementations for the supported platforms had to be added. The Windows implementation is based on the mentioned paper. The linux implementation is a wrapper around the pthread mutex and condition_variable.

#### COMPUTE

This component contains some experiments related to opencl. The current clcpp subfolder contains some headers which implement a simple approach to compile opencl code as C++. This approach only works with very simple kernels (e.g. no memory barries) but that should already be enough for a wide range of algorithms. Use the set_global_id function to set the global index and afterwards call the kernel function. The global index is stored in a thread local variable and the kernel functions can be called simultaneously from multiple threads. To automate the process of calling a kernel function, the opencl_kernel_executor combined with the fair_executor from the async component can be used. This automates the whole process of scheduling all work items using a thread pool.

#### More to come ...

...

# FAQ

* *What about the Raspberry PI?* Even so the RPI is a widely used platform, supporting it is tough. The official cross compiler is still stuck at gcc 4.9, meaning C++11. To build this project a newer toolchain would be required. That said a modern enough compiler should be all that's needed to compile this code for the raspberry PI.

## Installing Build Tools & Downloading Dependencies

#### Linux
   
    python3 3rdparty/download.py          # Download 3rd-party libraries (googletest, ios-cmake, android-ndk)
    sudo dnf install cmake gcc-c++        # Installing compiler, cmake and assembler

#### OSX

    python3 3rdparty/download.py
    brew install gcc
    brew upgrade cmake

#### Windows

    python 3rdparty/download.py

## Compilation

#### Linux

    mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF ..
    make -j8

#### OSX

    mkdir build && cd build
    cmake -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF ..
    make -j8

#### Windows

    mkdir build && cd build
    cmake -G "Visual Studio 15 2017 Win64" -DBUILD_SHARED_LIBS=OFF ..
    cmake --build . --config Release --target ALL_BUILD

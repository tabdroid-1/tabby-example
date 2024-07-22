Tabby engine example
--------------------

Supported Graphics Apis
-----------------------
- OpenGL 3.3
- OpenGL 3.0 es

Supported Platforms
-------------------
- Linux
- Windows (not tested)
- MacOS
- Web (not tested in a while)
- Android (not tested in a while)

Compiling
---------

You might have to install some libraries for building.
For building web install emscriptem from their git repo. Installing from package manager may give compilation errors.

- Native
  - For Unix:  

    1. ```shell
        git clone -b customRenderer-dev https://github.com/tabdroid-1/TabbyEngine.git && mkdir TabbyEngine/build && cd TabbyEngine/build
        ```

    2. ```shell
        cmake .. && make
        ```
    
  - For Windows:  
    - mingw 
     
      1. ```shell
         git clone https://github.com/tabdroid-1/MaineCoon.git && mkdir MaineCoon/build && cd MaineCoon/build
          ```
      2. ```shell
         cmake -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_C_COMPILER=i686-w64-mingw32-gcc -DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++ ..
         make -j4
          ```

- For Web 

  1. ```shell
     git clone -b customRenderer https://github.com/tabdroid-1/TabbyEngine.git && mkdir TabbyEngine/build && cd TabbyEngine/build
     ```

  2. ```shell
     emcmake cmake .. && make
     ```

  3. ```shell
     python3 -m http.server 8080
     ```
     
- For Android 

  1. ```shell
     # It should build out of the box when built from Android Studio
     ```

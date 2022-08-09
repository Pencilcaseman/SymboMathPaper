# Install script for directory: /mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/build/librapid/src/librapid/vendor/Vc/libVc.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/Vc" TYPE DIRECTORY FILES "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/Vc/" FILES_MATCHING REGEX "/*.(h|tcc|def)$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/Vc" TYPE FILE FILES
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/Vc/Allocator"
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/Vc/IO"
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/Vc/Memory"
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/Vc/SimdArray"
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/Vc/Utils"
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/Vc/Vc"
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/Vc/algorithm"
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/Vc/array"
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/Vc/iterators"
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/Vc/limits"
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/Vc/simdize"
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/Vc/span"
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/Vc/type_traits"
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/Vc/vector"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Vc/VcTargets.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Vc/VcTargets.cmake"
         "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/build/librapid/src/librapid/vendor/Vc/CMakeFiles/Export/lib/cmake/Vc/VcTargets.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Vc/VcTargets-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/Vc/VcTargets.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Vc" TYPE FILE FILES "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/build/librapid/src/librapid/vendor/Vc/CMakeFiles/Export/lib/cmake/Vc/VcTargets.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Vc" TYPE FILE FILES "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/build/librapid/src/librapid/vendor/Vc/CMakeFiles/Export/lib/cmake/Vc/VcTargets-release.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/Vc" TYPE FILE FILES
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/cmake/UserWarning.cmake"
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/cmake/VcMacros.cmake"
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/cmake/AddCompilerFlag.cmake"
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/cmake/CheckCCompilerFlag.cmake"
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/cmake/CheckCXXCompilerFlag.cmake"
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/cmake/OptimizeForArchitecture.cmake"
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/librapid/src/librapid/vendor/Vc/cmake/FindVc.cmake"
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/build/librapid/src/librapid/vendor/Vc/cmake/VcConfig.cmake"
    "/mnt/c/Users/penci/OneDrive/Desktop/Toby/School/Hampton/A-Levels/HELP/SymboMath/build/librapid/src/librapid/vendor/Vc/cmake/VcConfigVersion.cmake"
    )
endif()


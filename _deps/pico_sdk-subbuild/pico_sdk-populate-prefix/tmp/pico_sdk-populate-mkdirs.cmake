# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/santa/Desktop/SIT/INF2004/INF2004/_deps/pico_sdk-src"
  "C:/Users/santa/Desktop/SIT/INF2004/INF2004/_deps/pico_sdk-build"
  "C:/Users/santa/Desktop/SIT/INF2004/INF2004/_deps/pico_sdk-subbuild/pico_sdk-populate-prefix"
  "C:/Users/santa/Desktop/SIT/INF2004/INF2004/_deps/pico_sdk-subbuild/pico_sdk-populate-prefix/tmp"
  "C:/Users/santa/Desktop/SIT/INF2004/INF2004/_deps/pico_sdk-subbuild/pico_sdk-populate-prefix/src/pico_sdk-populate-stamp"
  "C:/Users/santa/Desktop/SIT/INF2004/INF2004/_deps/pico_sdk-subbuild/pico_sdk-populate-prefix/src"
  "C:/Users/santa/Desktop/SIT/INF2004/INF2004/_deps/pico_sdk-subbuild/pico_sdk-populate-prefix/src/pico_sdk-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/santa/Desktop/SIT/INF2004/INF2004/_deps/pico_sdk-subbuild/pico_sdk-populate-prefix/src/pico_sdk-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/santa/Desktop/SIT/INF2004/INF2004/_deps/pico_sdk-subbuild/pico_sdk-populate-prefix/src/pico_sdk-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()

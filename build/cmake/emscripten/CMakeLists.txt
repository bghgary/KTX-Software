cmake_minimum_required(VERSION 3.0)

project(ktx)

if (EMSCRIPTEN)
  add_subdirectory(../web/Release libktx EXCLUDE_FROM_ALL)

  set(CMAKE_CXX_STANDARD 11)

  add_executable(libktx.js
    ../../../emscripten/ktx_wrappers.cpp
  )

  target_include_directories(libktx.js PRIVATE ../../../include)

  target_link_libraries(libktx.js PRIVATE libktx_es3)

  set_target_properties(libktx.js PROPERTIES
      OUTPUT_NAME "libktx"
      SUFFIX ".js"
      LINK_FLAGS "--bind -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=2 -s MALLOC=emmalloc -s MODULARIZE=1 -s EXPORT_NAME=libktx -s EXTRA_EXPORTED_RUNTIME_METHODS=['GL'] -s FULL_ES3=1"
  )
endif()

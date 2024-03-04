find_external_library(DEPENDENCY_NAME e2sim
                        HEADER_NAME e2sim.hpp
                        LIBRARY_NAME e2sim
                        SEARCH_PATHS /usr/local/lib)

message("The directory found ${e2sim_INCLUDE_DIRS} ${e2sim_FOUND}")

if(${e2sim_FOUND})
  set(e2sim_include_dirs ${e2sim_INCLUDE_DIRS})
  include_directories("${e2sim_include_dirs}")
endif()

build_example(
  NAME e2-message-encoding
  SOURCE_FILES e2-message-encoding.cc
  LIBRARIES_TO_LINK
    ${libcore}
    ${liboran-interface}
)

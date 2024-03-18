find_external_library(DEPENDENCY_NAME e2sim
                    HEADER_NAME e2sim.hpp
                      LIBRARY_NAME e2sim
                      SEARCH_PATHS /usr/local/lib)

message("The value in src of ${e2sim_INCLUDE_DIRS}")

if(${e2sim_FOUND})
  set(e2sim_libraries ${e2sim_LIBRARIES})
  set(e2sim_include_dirs ${e2sim_INCLUDE_DIRS})
  # target_include_directories(libmmwave PUBLIC ${e2sim_include_dirs})
  include_directories(${e2sim_include_dirs})
  link_libraries(${e2sim_libraries})
endif()

message(STATUS "Project source dir ${PROJECT_SOURCE_DIR}")

set(libs "${contrib_libs_to_build}")

# Process subdirectories
foreach(libname ${libs})
  if(EXISTS ${PROJECT_SOURCE_DIR}/contrib/${libname}/CMakeLists.txt)
    message(STATUS "Processing contrib/${libname}")
    # add_subdirectory(${libname})
  # else()
  #   message(
  #     ${HIGHLIGHTED_STATUS}
  #     "Skipping contrib/${libname} : it does not contain a CMakeLists.txt file"
  #   )
  endif()
endforeach()

# add_subdirectory(oran-interface)
# add_subdirectory(nr)

# add_subdirectory("contrib/oran-interface")
# add_subdirectory("millicar")


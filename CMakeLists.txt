# CMakeLists files in this project can
# refer to the root source directory of the project as ${COMPWA_SOURCE_DIR} and
# to the root binary directory of the project as ${COMPWA_BINARY_DIR}.
cmake_minimum_required (VERSION 3.0)
#set(CMAKE_BUILD_TYPE Debug) 
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project( COMPWA )

# set the default path for built executables to the "bin" directory
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
# set the default path for built libraries to the "lib" directory
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set (CMAKE_CXX_STANDARD 11)

#${CMAKE_CXX_COMPILER_ID} can be one of {GNU Clang AppleClang Intel MSVC}
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  # Using Clang or AppleClang -> no python for now
  find_package(Boost COMPONENTS
               unit_test_framework
               #python
               system
               filesystem
               serialization
               thread
               chrono
               date_time
               log
               log_setup
               program_options
               regex
               REQUIRED
              )
else ()
  find_package(PythonLibs 2.7 EXACT)
  #message(${PYTHON_LIBRARIES})
  INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
  LINK_DIRECTORIES(${PYTHON_LIBRARIES})
  find_package(Boost COMPONENTS
               unit_test_framework
               python
               system
               filesystem
               serialization
               thread chrono
               date_time
               log
               log_setup
               program_options
               regex
               REQUIRED
              )
endif()

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

INCLUDE_DIRECTORIES(${COMPWA_SOURCE_DIR})
#LINK_DIRECTORIES(${COMPWA_SOURCE_DIR}/lib)

#Setup CMake to run tests
enable_testing()

set( CMAKE_SHARED_LIBRARY_PREFIX "libComPWA_" )

add_definitions(-DBOOST_LOG_DYN_LINK)
add_definitions(-DBOOST_TEST_DYN_LINK)
add_definitions(-DBOOST_SERIALIZATION_DYN_LINK)

# Recurse into subdirectories.
add_subdirectory(Core)
add_subdirectory(DataReader)
add_subdirectory(Estimator)
add_subdirectory(Optimizer)
add_subdirectory(Physics)
add_subdirectory(Tools)
add_subdirectory(Examples)
add_subdirectory(executables)

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile
                 ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile @ONLY)

    # Adding custom command to generate documenation
	# The file doxygen.stamp is generated to keep track
	# if documentation was generated. Unfortunatly cmake copy 
	# command does not support wildcartds, so we have to copy 
	# file-by-file.
	add_custom_command(
        OUTPUT
            doc/doxygen.stamp
        DEPENDS
            doc/Doxyfile
		COMMAND 
		    ${CMAKE_COMMAND} -E make_directory
            ${CMAKE_CURRENT_BINARY_DIR}/doc/images/
		COMMAND 
			${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_SOURCE_DIR}/doc/images/ComPWA_Modules.jpg
			${CMAKE_CURRENT_BINARY_DIR}/doc/images
		COMMAND 
			${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_SOURCE_DIR}/doc/images/HelicityAngle.png
			${CMAKE_CURRENT_BINARY_DIR}/doc/images
        COMMAND 
		    ${CMAKE_COMMAND} -E make_directory
            ${CMAKE_CURRENT_BINARY_DIR}/doc/doxygen-theme/
		COMMAND 
			${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen-theme/header.html
			${CMAKE_CURRENT_BINARY_DIR}/doc/doxygen-theme/
		COMMAND 
			${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen-theme/footer.html
			${CMAKE_CURRENT_BINARY_DIR}/doc/doxygen-theme/
		COMMAND 
			${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen-theme/customdoxygen.css
			${CMAKE_CURRENT_BINARY_DIR}/doc/doxygen-theme/
		COMMAND 
			${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen-theme/DoxygenLayout.xml
			${CMAKE_CURRENT_BINARY_DIR}/doc/doxygen-theme/
		COMMAND 
			${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen-theme/doxy-boot.js
			${CMAKE_CURRENT_BINARY_DIR}/doc/doxygen-theme/
        COMMAND
            doxygen doc/Doxyfile
        COMMAND
            cmake -E touch doc/doxygen.stamp

        COMMENT
            "Generating API documentation with Doxygen"
        VERBATIM
    )

    # Actual doc target depends on doxygen.stamp and runs 
	# custom command if needed
	add_custom_target(
        #doc ALL # run the target with make
        doc # run it separatly
        DEPENDS doc/doxygen.stamp
    )

    # Old solution for doxygen
	#add_custom_target(doc
          #${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile
          #WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          #COMMENT "Generating API documentation with Doxygen" VERBATIM)

    # Copy images
	#file(GLOB img
    #    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/doc/images *
    #)

    #add_custom_command(
      #TARGET doc PRE_BUILD
      #COMMAND ${CMAKE_COMMAND} -E make_directory
      #${CMAKE_CURRENT_BINARY_DIR}/doc/images/ )
    #add_custom_command(
      #TARGET doc PRE_BUILD
      #COMMAND ${CMAKE_COMMAND} -E make_directory
      #${CMAKE_CURRENT_BINARY_DIR}/doc/doxygen-theme/ )
    #add_custom_command(
      #TARGET doc PRE_BUILD
      #COMMAND ${CMAKE_COMMAND} -E copy
      #${CMAKE_CURRENT_SOURCE_DIR}/doc/images/*
      #${CMAKE_CURRENT_BINARY_DIR}/doc/images/ )
    #add_custom_command(
      #TARGET doc PRE_BUILD
      #COMMAND ${CMAKE_COMMAND} -E copy
      #${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen-theme/*
      #${CMAKE_CURRENT_BINARY_DIR}/doc/doxygen-theme/ )

endif(DOXYGEN_FOUND)
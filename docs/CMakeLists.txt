find_package(Doxygen REQUIRED)
find_package(Sphinx REQUIRED)

set(DOXYGEN_INPUT_DIR_SRC ${PROJECT_SOURCE_DIR}/src)
set(DOXYGEN_INPUT_DIR_INCLUDE ${PROJECT_SOURCE_DIR}/include)
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/xml/index.xml)
set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

# Set the Doxygen input and output directories in the Doxyfile
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

# Doxygen won't create this for us
file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})

# Find all the public headers
file(GLOB_RECURSE project_include_headers ${PROJECT_SOURCE_DIR}/include/*.h ${PROJECT_SOURCE_DIR}/include/*.hpp)
file(GLOB_RECURSE project_src_headers ${PROJECT_SOURCE_DIR}/src/*.h ${PROJECT_SOURCE_DIR}/src/*.hpp)
# Filter out headers files in src/**/internal/ details/ private/ subfolders
list(FILTER project_src_headers EXCLUDE REGEX "\/internal\/")
list(FILTER project_src_headers EXCLUDE REGEX "\/details\/")
list(FILTER project_src_headers EXCLUDE REGEX "\/private\/")
set(project_headers ${project_include_headers} ${project_src_headers})
message("project_include_headers=${project_include_headers}")
message("project_headers=${project_headers}")

# Only regenerate Doxygen when the Doxyfile or public headers change
add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
		DEPENDS ${project_include_headers}
        	COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
		MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
        	COMMENT "Generating docs"
		VERBATIM)

# Nice named target so we can run the job easily
if (BUILD_DOCS_EXCLUDE_FROM_ALL)
	add_custom_target(Doxygen DEPENDS ${DOXYGEN_INDEX_FILE})
else()
	add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})
endif()

set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sphinx)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

# Only regenerate Sphinx when:
#  - Doxygen has rerun
#  - Our doc files have been updated
#  - The Sphinx config has been updated
add_custom_command(OUTPUT ${SPHINX_INDEX_FILE}
	COMMAND
		${SPHINX_EXECUTABLE} -b html
		# Tell Breathe where to find the Doxygen output
		-Dbreathe_projects.main_project=${DOXYGEN_OUTPUT_DIR}/xml
		${SPHINX_SOURCE} ${SPHINX_BUILD}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	DEPENDS
		# Other docs files you want to track should go here (or in some variable)
		${CMAKE_CURRENT_SOURCE_DIR}/index.rst
		${DOXYGEN_INDEX_FILE}
	MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
	COMMENT "Generating documentation with Sphinx")

# Nice named target so we can run the job easily
if (HELLOIMGUI_BUILD_DOCS_EXCLUDE_FROM_ALL)
	add_custom_target(Sphinx DEPENDS ${SPHINX_INDEX_FILE})
else()
	add_custom_target(Sphinx ALL DEPENDS ${SPHINX_INDEX_FILE})
endif()

include(GNUInstallDirs)
install(DIRECTORY ${SPHINX_BUILD}
	DESTINATION ${CMAKE_INSTALL_DOCDIR})

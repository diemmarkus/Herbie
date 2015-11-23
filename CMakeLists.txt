
# check cmake requirements
cmake_minimum_required(VERSION 2.6)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	  
	IF(CMAKE_VERSION VERSION_GREATER 2.8.10)
		cmake_policy(SET CMP0020 NEW)	# qt warnings
	ENDIF()
endif(COMMAND cmake_policy)

######################## begin of project
project(Herbie)

# load paths from the user file if exists 
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/CMakeUser.cmake)
	include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeUser.cmake)
endif()

option(DISABLE_QT_DEBUG "Disable Qt Debug Messages" OFF)

set(CMAKE_AUTOMOC TRUE)
 
# find Qt
unset(QT_QTCORE_LIBRARY CACHE)
unset(QT_QTGUI_LIBRARY CACHE)
unset(QT_QTMAIN_LIBRARY CACHE)

# set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
if(NOT QT_QMAKE_EXECUTABLE)
	find_program(QT_QMAKE_EXECUTABLE NAMES "qmake" "qmake-qt5" "qmake.exe")
endif()
if(NOT QT_QMAKE_EXECUTABLE)
	message(FATAL_ERROR "you have to set the path to the Qt5 qmake executable")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
# message(STATUS "qmake path: ${QT_QMAKE_EXECUTABLE}")
GET_FILENAME_COMPONENT(QT_QMAKE_PATH ${QT_QMAKE_EXECUTABLE} PATH)
set(CMAKE_PREFIX_PATH ${QT_QMAKE_PATH}\\..\\lib\\cmake\\Qt5Widgets ${QT_QMAKE_PATH}\\..\\lib\\cmake\\Qt5OpenGL ${QT_QMAKE_PATH}\\..\\lib\\cmake\\Qt5Concurrent ${QT_QMAKE_PATH})
# message(STATUS "path: ${QT_QMAKE_PATH}")
# message(STATUS "cmake path: ${CMAKE_PREFIX_PATH}")

unset(Qt5Widgets_FOUND CACHE)
find_package(Qt5Widgets)
find_package(Qt5Concurrent)

set(QT_INCLUDES ${Qt5Widgets_INCLUDE_DIRS} 	${Qt5Concurrent_INCLUDE_DIRS})
set(QT_LIBRARIES ${Qt5Widgets_LIBRARIES} 	${Qt5Concurrent_LIBRARIES} ${Qt5Core_LIBARIES} ${Qt5Gui_LIBRARIES})

if (NOT Qt5Widgets_FOUND)
	message(FATAL_ERROR "Qt5Widgets not found. Check your QT_QMAKE_EXECUTABLE path and set it to the correct location")
endif()

# message(STATUS "QT_INCLUDES: ${QT_INCLUDES}")
add_definitions(-DQT5)

if (DISABLE_QT_DEBUG)
	message (STATUS "disabling qt debug messages")
	add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DQT_NO_DEBUG)
endif()

# gather information for building
include_directories (
	${QT_INCLUDES}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_SOURCE_DIR}/src
)


file(GLOB HERBIE_SOURCES "src/*.cpp")
file(GLOB HERBIE_HEADERS "src/*.h")

set (HERBIE_QRC src/Herbie.qrc)
set (HERBIE_RC src/Herbie.rc) #add resource file when compiling with MSVC 

QT5_ADD_RESOURCES(HERBIE_RCC ${HERBIE_QRC})
# QT5_WRAP_CPP(HERBIE_MOC_SRC ${HERBIE_HEADERS})

set(VERSION_LIB Version.lib)
set(LIBRARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/libs) #add libs directory to library dirs 

# create the targets
set(BINARY_NAME ${CMAKE_PROJECT_NAME})

# binary
link_directories(${LIBRARY_DIR})
add_executable(${BINARY_NAME} WIN32 ${HERBIE_SOURCES} ${HERBIE_HEADERS} ${HERBIE_QRC} ${HERBIE_RC})
target_link_libraries(${BINARY_NAME} ${VERSION_LIB} ${LIB_NAME})

set_target_properties(${BINARY_NAME} PROPERTIES COMPILE_FLAGS "-DDK_DLL_IMPORT -DNOMINMAX")
set_target_properties(${BINARY_NAME} PROPERTIES LINK_FLAGS_REALLYRELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /LARGEADDRESSAWARE")
set_target_properties(${BINARY_NAME} PROPERTIES LINK_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:CONSOLE /LARGEADDRESSAWARE")
set_target_properties(${BINARY_NAME} PROPERTIES LINK_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SUBSYSTEM:CONSOLE /LARGEADDRESSAWARE")
set_target_properties(${BINARY_NAME} PROPERTIES IMPORTED_IMPLIB "")

# add new modules here to link the libs
qt5_use_modules(${BINARY_NAME} Widgets Gui Concurrent)

SET(CMAKE_SHARED_LINKER_FLAGS_REALLYRELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /LARGEADDRESSAWARE") # /subsystem:windows does not work due to a bug in cmake (see http://public.kitware.com/Bug/view.php?id=12566)

set(QTLIBLIST Qt5Core Qt5Gui Qt5Network Qt5Widgets Qt5PrintSupport Qt5Concurrent Qt5Sensors Qt5MultimediaWidgets Qt5Multimedia Qt5OpenGL Qt5Positioning Qt5WebChannel Qt5Sql)
	
foreach(qtlib ${QTLIBLIST})
	get_filename_component(QT_DLL_PATH_tmp ${QT_QMAKE_EXECUTABLE} PATH)
	file(COPY ${QT_DLL_PATH_tmp}/${qtlib}.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
	file(COPY ${QT_DLL_PATH_tmp}/${qtlib}.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)
	file(COPY ${QT_DLL_PATH_tmp}/${qtlib}d.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
endforeach(qtlib)

# create settings file for portable version while working
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/Release/settings.nfo)
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/Release/settings.nfo "")
endif()
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/Debug/settings.nfo)
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/Debug/settings.nfo "")
endif()

# copy translation files after each build
add_custom_command(TARGET ${BINARY_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory \"${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/translations/\")
foreach(QM ${HERBIE_QM})
	add_custom_command(TARGET ${BINARY_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy \"${QM}\" \"${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/translations/\")
endforeach(QM)


# set properties for Visual Studio Projects
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;ReallyRelease" CACHE STRING "limited configs" FORCE)
add_definitions(/Zc:wchar_t-)
set(CMAKE_CXX_FLAGS_DEBUG "/W3 ${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "/W3 /O2 ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_REALLYRELEASE "${CMAKE_CXX_FLAGS_RELEASE}  /DQT_NO_DEBUG_OUTPUT")

set(CMAKE_EXE_LINKER_FLAGS_REALLYRELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /LARGEADDRESSAWARE") # /subsystem:windows does not work due to a bug in cmake (see http://public.kitware.com/Bug/view.php?id=12566)

set_target_properties(${EXE_NAME} PROPERTIES LINK_FLAGS_REALLYRELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /LARGEADDRESSAWARE")
set_target_properties(${EXE_NAME} PROPERTIES LINK_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:CONSOLE /LARGEADDRESSAWARE")
set_target_properties(${EXE_NAME} PROPERTIES LINK_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SUBSYSTEM:CONSOLE /LARGEADDRESSAWARE")

#set(_moc ${CMAKE_CURRENT_BINARY_DIR}/GeneratedFiles)
#file(GLOB HERBIE_AUTOMOC "${CMAKE_BINARY_DIR}/*_automoc.cpp")
source_group("Generated Files" FILES ${HERBIE_QRC} ${HERBIE_RC})
set_source_files_properties(${HERBIE_TRANSLATIONS} PROPERTIES HEADER_FILE_ONLY TRUE)
source_group("Translations" FILES ${HERBIE_TRANSLATIONS})

# copy some files
file(COPY ${CMAKE_SOURCE_DIR}/src/img/herbie.ico DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ReallyRelease)
file(COPY ${CMAKE_SOURCE_DIR}/src/img/herbie.ico DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
file(COPY ${CMAKE_SOURCE_DIR}/src/img/herbie.ico DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)


# generate configuration file
if(DLL_NAME)
	get_property(HERBIE_DEBUG_NAME TARGET ${DLL_NAME} PROPERTY DEBUG_OUTPUT_NAME)
	get_property(HERBIE_RELEASE_NAME TARGET ${DLL_NAME} PROPERTY RELEASE_OUTPUT_NAME)
	set(HERBIE_LIBS optimized ${CMAKE_BINARY_DIR}/libs/${HERBIE_RELEASE_NAME}.lib debug  ${CMAKE_BINARY_DIR}/libs/${HERBIE_DEBUG_NAME}.lib)
endif()
set(HERBIE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(HERBIE_BUILD_DIRECTORY ${CMAKE_BINARY_DIR})

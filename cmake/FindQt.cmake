if(_FIND_QT_INCLUDED)
	return()
endif()
set(_FIND_QT_INCLUDED TRUE)

set(QT_DIR "c:/Qt/Qt5.8.0/5.8/msvc2015_64" CACHE PATH "Path to Qt directory")
if(QT_DIR)
	string(REGEX MATCH "Qt(5.[0-9]*.[0-9]*)" MATCH_RESULT ${QT_DIR})
	set(QT_VERSION ${CMAKE_MATCH_1})
endif()
if(NOT EXISTS "${QT_DIR}/include/QtCore/${QT_VERSION}/QtCore/private/qobject_p.h")
	message(FATAL_ERROR "Qt not found. Set 'QT_DIR' variable ('c:/Qt/Qt5.8.0/5.8/msvc2015_64' for example).")
endif()

function(get_required_qt_dir_name result)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(bitness 64)
	else()
		set(bitness 86)
	endif()
	
	if(MSVC10)
		set(vs_version "2010")
	elseif(MSVC11)
		set(vs_version "2012")	
	elseif(MSVC12)
		set(vs_version "2013")
	elseif(MSVC14)
		set(vs_version "2015")
	endif()
	
	if(vs_version)
		set(${result} "msvc${vs_version}_${bitness}" PARENT_SCOPE)
	else()
		message(FATAL_ERROR "Qt: unsupported compiler.")
	endif()
endfunction()

get_filename_component(QT_DIR_NAME ${QT_DIR} NAME)
get_required_qt_dir_name(QT_REQUIRED_DIR_NAME)
if(NOT ${QT_DIR_NAME} STREQUAL ${QT_REQUIRED_DIR_NAME})
	message(FATAL_ERROR "Qt(${QT_DIR_NAME}) is incompatible with compiler(${QT_REQUIRED_DIR_NAME}).")
endif()

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_DIR})

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

function(install_qt_library name)
	if(WIN32)
		set(prefix "${QT_DIR}/bin/${name}")
		install(FILES "${prefix}.dll" CONFIGURATIONS Release RelWithDebInfo MinSizeRel DESTINATION ".")
		install(FILES "${prefix}d.dll" "${prefix}d.pdb" CONFIGURATIONS Debug DESTINATION ".")
	endif()
endfunction()

# install_qt_directory(srcDir, dstDir, options)
function(install_qt_directory srcDir dstDir)
	if(WIN32)
		install(DIRECTORY "${QT_DIR}/${srcDir}" CONFIGURATIONS Release RelWithDebInfo MinSizeRel DESTINATION ${dstDir} 
			PATTERN "*d.dll" EXCLUDE
			PATTERN "*.pdb" EXCLUDE
			PATTERN "*.qmlc" EXCLUDE
			${ARGN}
		)
		install(DIRECTORY "${QT_DIR}/${srcDir}" CONFIGURATIONS Debug DESTINATION ${dstDir}
			REGEX "[^d].dll$" EXCLUDE
			PATTERN "*.qmlc" EXCLUDE
			${ARGN}
		)
	endif()
endfunction()

find_package(Qt5Core ${QT_VERSION})
find_package(Qt5Gui ${QT_VERSION})

install_qt_library(Qt5Core)
install_qt_library(Qt5Gui)
install_qt_directory("plugins/imageformats" ".")



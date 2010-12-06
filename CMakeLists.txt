
cmake_minimum_required (VERSION 2.6)

project (guibuilder)

add_definitions(-D__CMAKE_PROJECT__)

set (GUIBUILDER_MAJOR_VERSION 1)
set (GUIBUILDER_MINOR_VERSION 0)
set (GUIBUILDER_MICRO_VERSION 6)

set (GUIBUILDER_NAME "guibuilder")
set (GUIBUILDER_VERSION ${GUIBUILDER_MAJOR_VERSION}.${GUIBUILDER_MINOR_VERSION}.${GUIBUILDER_MICRO_VERSION})
set (GUIBUILDER_LIBS "")

set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include (common)

include (CheckSymbolExists)
## check for test
macro(find_and_add_lib _libname)
    unset(_lib_find CACHE)
    find_library(_lib_find ${_libname})
    if (NOT _lib_find)
        message(FATAL_ERROR "cannot find the libraries ${_libname} for guibuilder")
    endif (NOT _lib_find)
    if (GUIBUILDER_LIBS)
        set (GUIBUILDER_LIBS "${GUIBUILDER_LIBS} -l${_libname}")
    else()
        set (GUIBUILDER_LIBS "-l${_libname}")
    endif (GUIBUILDER_LIBS)
endmacro(find_and_add_lib)

macro(check_and_add_lib _libappname _symbol _file)
    unset(_sym_find CACHE)
    check_symbol_exists (${_symbol} "${_file}" _sym_find)
    #not found
    if (_sym_find)
        find_and_add_lib (${_libappname})
    else ()
        message("-- cannot find ${_symbol} in file ${_file}")
    endif (_sym_find)
endmacro(check_and_add_lib)

macro(check_support_guibuilder _symbol  _file)
    unset(_check_ret CACHE)
    check_symbol_exists(${_symbol} ${_file} _check_ret)
    if (NOT _check_ret)
        message (FATAL_ERROR "guibuilder need support in ${_file} with ${_symbol} defined")
    endif(NOT _check_ret)
endmacro(check_support_guibuilder)

####################################################
# check minigui
check_symbol_exists(_MGRM_THREADS "minigui/common.h" MINIGUI_THREAD)

if ( NOT MINIGUI_THREAD)
    message (FATAL_ERROR "guibuilder need minigui with thread version")
endif ( NOT MINIGUI_THREAD)

check_support_guibuilder(__TARGET_MSTUDIO__ "minigui/common.h")
find_and_add_lib("minigui_msd")

# add the libriase for minigui
check_and_add_lib(m _HAVE_MATH_LIB "minigui/common.h")
check_and_add_lib(png _MGIMAGE_PNG "minigui/common.h")
check_and_add_lib(jpeg _MGIMAGE_JPG "minigui/common.h")
check_and_add_lib(ttf _MGFONT_TTF "minigui/common.h")
check_and_add_lib(freetype _MGFONT_FT2 "minigui/common.h")
check_and_add_lib(dl _MGIAL_DLCUSTOM "minigui/common.h")

#################################
# check and add mgutils
find_and_add_lib("mgutils_msd")
# check and add mgutils
find_and_add_lib("mgplus_msd")

# check and add the mgncs
check_support_guibuilder(_MGNCS_GUIBUILDER_SUPPORT "mgncs/mgncsconfig.h")
find_and_add_lib("mgncs_msd")

#########################################################
# add the xml support
include (FindLibXml2)
if (NOT LIBXML2_FOUND)
    message (FATAL_ERROR "cannot find libxml2")
endif(NOT LIBXML2_FOUND)

include_directories(${LIBXML2_INCLUDE_DIR})
set (GUIBUILDER_LIBS "${GUIBUILDER_LIBS} ${LIBXML2_LIBRARIES}")

# find lua 5.1
include (FindLua51)
if (NOT LUA51_FOUND)
    message (FATAL_ERROR "cannot find the lua5.1")
endif (NOT LUA51_FOUND)

include_directories(${LUA_INCLUDE_DIR})
set (GUIBUILDER_LIBS "${GUIBUILDER_LIBS} ${LUA_LIBRARY}")

# find curl
include (FindCURL)
if (NOT CURL_FOUND)
    message (FATAL_ERROR "cannot found the curl")
endif(NOT CURL_FOUND)
include_directories(${CURL_INCLUDE_DIRS})
set (GUIBUILDER_LIBS "${GUIBUILDER_LIBS} ${CURL_LIBRARIES}")

mg_declare_option_and_set_value(_MSTUDIO_OFFICIAL_RELEASE offical_release  BOOL OFF "build the offical release version")

if (offical_release)
    mg_declare_option_and_set_value(_MSTUDIO_MAXNUM_LIMITWNDS maxnum_limit_windows STRING "1" "configure the max number of limit window")
    if(WIN32)
    else()
        include_directories(${PROJECT_SOURCE_DIR}/softdog/32bit/2.6.x/include)
        set (GUIBUILDER_LIBS "${GUIBUILDER_LIBS}  ${PROJECT_SOURCE_DIR}/softdog/32bit/2.6.x/lib/libsense4.a -lusb")
    endif(WIN32)
endif(offical_release)

if (NOT ${CMAKE_REQUIRED_INCLUDES} STREQUAL "")
    include_directories(${CMAKE_REQUIRED_INCLUDES})
endif (NOT ${CMAKE_REQUIRED_INCLUDES} STREQUAL "")

if (NOT ${CMAKE_LIBRARY_PATH} STREQUAL "")
    link_directories (${CMAKE_LIBRARY_PATH})
endif (NOT ${CMAKE_LIBRARY_PATH} STREQUAL "")

mg_declare_option (with_customversion STRING "" "Configure the special package version(separated strings using the underscore) in about dialog.")
if (NOT with_customversion STREQUAL "")
    set (_MSTUDIO_PACKAGE_STRING "${with_customversion}")
endif (NOT with_customversion STREQUAL "")


############## check header files ################
include (CheckIncludeFiles) 
check_include_files(alloca.h HAVE_ALLOCA_H)
check_include_files(dirent.h HAVE_DIRENT_H)
check_include_files(dlfcn.h HAVE_DLFCN_H)
check_include_files(inttypes.h HAVE_INTTYPES_H)
check_include_files(locale.h HAVE_LOCALE_H)
check_include_files(math.h HAVE_MATH_H)
check_include_files(memory.h HAVE_MEMORY_H)
check_include_files(ndir.h HAVE_NDIR_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(sys/dir.h HAVE_SYS_DIR_H)
check_include_files(sys/ndir.h HAVE_SYS_NDIR_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(sys/time.h HAVE_SYS_TIME_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(termio.h HAVE_TERMIO_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(fcntl.h HAVE_FCNTL_H)
check_include_files(inttypes.h HAVE_INTTYPES_H)
check_include_files(limits.h HAVE_LIMITS_H)
check_include_files(netdb.h HAVE_NETDB_H)
check_include_files(netinet/in.h HAVE_NETINET_IN_H)

############## check functions ################
include(CheckFunctionExists)
check_function_exists (alloca HAVE_ALLOCA)
check_function_exists (time HAVE_TIME)
check_function_exists (getpagesize HAVE_GETPAGESIZE)
check_function_exists (mktime HAVE_MKTIME)
check_function_exists (localtime HAVE_LOCALTIME)
check_function_exists (strdup HAVE_STRDUP)
check_function_exists (strcasecmp HAVE_STRCASECMP)
check_function_exists (strncasecmp HAVE_STRNCASECMP)
check_function_exists (strerror HAVE_STRERROR)
check_function_exists (getpt HAVE_GETPT)
check_function_exists (mmap HAVE_MMAP)
check_function_exists (setlocale HAVE_SETLOCALE)
check_function_exists (vprintf HAVE_VPRINTF)
check_function_exists (atexit HAVE_ATEXIT)
check_function_exists (bzero HAVE_BZERO)
check_function_exists (fork HAVE_FORK)
check_function_exists (gethostbyname HAVE_GETHOSTBYNAME)
check_function_exists (memset HAVE_MEMSET)
check_function_exists (mkdir HAVE_MKDIR)
check_function_exists (mkfifo HAVE_MKFIFO)
check_function_exists (select HAVE_SELECT)
check_function_exists (setenv HAVE_SETENV)

##################################################################################
## gbconfig.h
configure_file(
    "${PROJECT_SOURCE_DIR}/cmake_gbconfig.h.in"
    "${PROJECT_BINARY_DIR}/gbconfig.h")

set (etc_dir "${CMAKE_INSTALL_PREFIX}/etc/${GUIBUILDER_NAME}")
## guibuilder.cfg
configure_file("${PROJECT_SOURCE_DIR}/etc/cmake_guibuilder.cfg.in"
    "${PROJECT_BINARY_DIR}/guibuilder.cfg")
## lang.cfg
configure_file("${PROJECT_SOURCE_DIR}/etc/cmake_lang.cfg.in"
    "${PROJECT_BINARY_DIR}/lang.cfg")


###################################################################################
## add the guibuilder execute
add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/build_version.h
    COMMAND ${PROJECT_SOURCE_DIR}/rebuild-version 
    DEPENDS ${PROJECT_SOURCE_DIR}/rebuild-version
    COMMENT "generate build version automatically") 

## collect the source file
list(APPEND main_srcs main.cpp func-win.cpp ${PROJECT_BINARY_DIR}/build_version.h)
mg_add_source_files(${main_srcs})

mg_add_sub_source_dirs(
    include 
    common
    mgfc
    mainframe
    uieditor
    rdreditor
    imgeditor
    texteditor
    resideditor
    )

include_directories("${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/mgfc"
    "${PROJECT_SOURCE_DIR}/mainframe"
    "${PROJECT_SOURCE_DIR}/uieditor"
    "${PROJECT_SOURCE_DIR}/rdreditor"
    "${PROJECT_SOURCE_DIR}/texteditor"
    "${PROJECT_SOURCE_DIR}/texteditor/json"
    "${PROJECT_SOURCE_DIR}/imgeditor"
    )

add_executable(${GUIBUILDER_NAME} ${_source_list_})
# target link
target_link_libraries(${GUIBUILDER_NAME} ${GUIBUILDER_LIBS})

################################ target to install ############################
# install guibuilder
install(TARGETS ${GUIBUILDER_NAME} RUNTIME DESTINATION bin)
# install the etc
install(DIRECTORY "${PROJECT_SOURCE_DIR}/etc/"
    DESTINATION etc/${GUIBUILDER_NAME}
    FILES_MATCHING PATTERN "*"
                   PATTERN ".svn" EXCLUDE
                   PATTERN "Makefile*" EXCLUDE
                   PATTERN "*.sh" EXCLUDE
                   PATTERN "test-*" EXCLUDE
                   PATTERN "msd" EXCLUDE
                   PATTERN "Emulator" EXCLUDE
                   PATTERN "*~$" EXCLUDE
                   PATTERN "*.old" EXCLUDE
                   PATTERN "*.bak" EXCLUDE
                   PATTERN "*.in" EXCLUDE
   )

install (FILES "${PROJECT_BINARY_DIR}/guibuilder.cfg" 
        "${PROJECT_BINARY_DIR}/lang.cfg"
        DESTINATION etc/${GUIBUILDER_NAME})

##############################################
## message  info
message ("=======================================================")
message ("GUIBUILDER Version   : ${GUIBUILDER_VERSION} ")
message ("GUIBUILDER Name      : ${GUIBUILDER_NAME}" )
message ("MStudio Include Path : ${CMAKE_REQUIRED_INCLUDES}")
message ("Libraries : ${GUIBUILDER_LIBS}")
message ("")
if (_MSTUDIO_OFFICIAL_RELEASE)
    message ("Offical Release Enable : TRUE")
    message ("Max Window Limit : ${_MSTUDIO_MAXNUM_LIMITWNDS}")
    
else()
    message ("Offical Release Enable : FALSE")
endif (_MSTUDIO_OFFICIAL_RELEASE)

if (_MSTUDIO_PACKAGE_STRING)
    message ("MStudio Version String : ${_MSTUDIO_PACKAGE_STRING}")
else()
    message ("MStudio Version String : ${GUIBUILDER_VERSION}")
endif (_MSTUDIO_PACKAGE_STRING)

message ("======================================================")



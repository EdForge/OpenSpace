##########################################################################################
#                                                                                        #
# OpenSpace                                                                              #
#                                                                                        #
# Copyright (c) 2014-2021                                                                #
#                                                                                        #
# Permission is hereby granted, free of charge, to any person obtaining a copy of this   #
# software and associated documentation files (the "Software"), to deal in the Software  #
# without restriction, including without limitation the rights to use, copy, modify,     #
# merge, publish, distribute, sublicense, and/or sell copies of the Software, and to     #
# permit persons to whom the Software is furnished to do so, subject to the following    #
# conditions:                                                                            #
#                                                                                        #
# The above copyright notice and this permission notice shall be included in all copies  #
# or substantial portions of the Software.                                               #
#                                                                                        #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,    #
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A          #
# PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT     #
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF   #
# CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE   #
# OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                          #
##########################################################################################

include(${OPENSPACE_CMAKE_EXT_DIR}/set_openspace_compile_settings.cmake)

# System libraries
if (APPLE)
  begin_dependency("Core Libraries")
  add_library(external-system-apple INTERFACE)

  target_include_directories(external-system-apple INTERFACE "/Developer/Headers/FlatCarbon")
  find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
  find_library(CARBON_LIBRARY Carbon REQUIRED)
  find_library(COCOA_LIBRARY Cocoa REQUIRED) # This was Carbon before
  find_library(APP_SERVICES_LIBRARY ApplicationServices REQUIRED)
  find_library(APPKIT_LIBRARY AppKit REQUIRED)
  mark_as_advanced(COREFOUNDATION_LIBRARY CARBON_LIBRARY COCOA_LIBRARY
    APP_SERVICES_LIBRARY APPKIT_LIBRARY
  )
  target_link_libraries(external-system-apple INTERFACE # This was PUBLIC before
    ${CARBON_LIBRARY} ${COREFOUNDATION_LIBRARY}
    ${COCOA_LIBRARY} ${APP_SERVICES_LIBRARY}
    ${APPKIT_LIBRARY}
  )
  end_dependency()
endif ()

# Ghoul
add_subdirectory(ghoul)
set_folder_location(Lua "External")
set_folder_location(lz4 "External")
set_folder_location(GhoulTest "Unit Tests")

# Spice
begin_dependency("Spice")
add_subdirectory(spice)
set_folder_location(spice "External")
end_dependency()

# Curl
begin_dependency("CURL")
add_library(external-curl INTERFACE)
if (WIN32)
  target_include_directories(external-curl INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/curl/include")
  target_link_libraries(external-curl INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/curl/lib/libcurl.lib")
  target_compile_definitions(external-curl INTERFACE "OPENSPACE_CURL_ENABLED" "CURL_STATICLIB")
else ()
  find_package(CURL)
  if (CURL_FOUND)
    target_include_directories(external-curl INTERFACE ${CURL_INCLUDE_DIRS})
    target_link_libraries(external-curl INTERFACE ${CURL_LIBRARIES})
    target_compile_definitions(external-curl INTERFACE "OPENSPACE_CURL_ENABLED")
  endif ()
endif ()
end_dependency()

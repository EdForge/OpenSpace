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

set(HEADER_FILES
  include/filesystemaccess.h
  include/launcherwindow.h
  include/profile/additionalscriptsdialog.h
  include/profile/assetsdialog.h
  include/profile/assettreeitem.h
  include/profile/assettreemodel.h
  include/profile/cameradialog.h
  include/profile/deltatimesdialog.h
  include/profile/keybindingsdialog.h
  include/profile/line.h
  include/profile/marknodesdialog.h
  include/profile/metadialog.h
  include/profile/modulesdialog.h
  include/profile/timedialog.h
  include/profile/profileedit.h
  include/profile/propertiesdialog.h
)

set(SOURCE_FILES
  src/launcherwindow.cpp
  src/filesystemaccess.cpp
  src/profile/additionalscriptsdialog.cpp
  src/profile/assetsdialog.cpp
  src/profile/assettreeitem.cpp
  src/profile/assettreemodel.cpp
  src/profile/cameradialog.cpp
  src/profile/deltatimesdialog.cpp
  src/profile/keybindingsdialog.cpp
  src/profile/line.cpp
  src/profile/marknodesdialog.cpp
  src/profile/metadialog.cpp
  src/profile/modulesdialog.cpp
  src/profile/timedialog.cpp
  src/profile/profileedit.cpp
  src/profile/propertiesdialog.cpp
)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(MOC_FILES "")
qt5_wrap_cpp(
  MOC_FILES
  include/launcherwindow.h
  include/profile/additionalscriptsdialog.h
  include/profile/assetsdialog.h
  include/profile/assettreemodel.h
  include/profile/cameradialog.h
  include/profile/deltatimesdialog.h
  include/profile/keybindingsdialog.h
  include/profile/marknodesdialog.h
  include/profile/metadialog.h
  include/profile/modulesdialog.h
  include/profile/timedialog.h
  include/profile/profileedit.h
  include/profile/propertiesdialog.h
)

set(RESOURCE_FILES "")
qt5_add_resources(RESOURCE_FILES resources/resources.qrc)

add_library(openspace-ui-launcher STATIC ${HEADER_FILES} ${SOURCE_FILES} ${MOC_FILES} ${RESOURCE_FILES})
set_openspace_compile_settings(openspace-ui-launcher)
target_include_directories(openspace-ui-launcher PUBLIC include)
target_link_libraries(openspace-ui-launcher PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets openspace-core)

if (MSVC)
set(MSVC_WARNINGS
  "/wd4619"   # #pragma warning: there is no warning number (raised by Qt headers)
  "/wd4946"   # reinterpret_cast used between related classes: 
)
target_compile_options(openspace-ui-launcher INTERFACE ${MSVC_WARNINGS})

endif ()

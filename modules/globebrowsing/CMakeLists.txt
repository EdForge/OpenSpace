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

include(${OPENSPACE_CMAKE_EXT_DIR}/module_definition.cmake)

set(HEADER_FILES
  globebrowsingmodule.h
  src/asynctiledataprovider.h
  src/basictypes.h
  src/dashboarditemglobelocation.h
  src/ellipsoid.h
  src/gdalwrapper.h
  src/geodeticpatch.h
  src/globelabelscomponent.h
  src/globetranslation.h
  src/gpulayergroup.h
  src/layer.h
  src/layeradjustment.h
  src/layergroup.h
  src/layergroupid.h
  src/layermanager.h
  src/layerrendersettings.h
  src/lrucache.h
  src/lrucache.inl
  src/lruthreadpool.h
  src/lruthreadpool.inl
  src/memoryawaretilecache.h
  src/prioritizingconcurrentjobmanager.h
  src/prioritizingconcurrentjobmanager.inl
  src/rawtile.h
  src/rawtiledatareader.h
  src/renderableglobe.h
  src/ringscomponent.h
  src/shadowcomponent.h
  src/skirtedgrid.h
  src/tileindex.h
  src/tileloadjob.h
  src/tileprovider.h
  src/tiletextureinitdata.h
  src/timequantizer.h
)

set(SOURCE_FILES
  globebrowsingmodule.cpp
  globebrowsingmodule_lua.inl
  src/asynctiledataprovider.cpp
  src/dashboarditemglobelocation.cpp
  src/ellipsoid.cpp
  src/gdalwrapper.cpp
  src/geodeticpatch.cpp
  src/globelabelscomponent.cpp
  src/globetranslation.cpp
  src/gpulayergroup.cpp
  src/layer.cpp
  src/layeradjustment.cpp
  src/layergroup.cpp
  src/layermanager.cpp
  src/layerrendersettings.cpp
  src/memoryawaretilecache.cpp
  src/rawtiledatareader.cpp
  src/renderableglobe.cpp
  src/ringscomponent.cpp
  src/shadowcomponent.cpp
  src/skirtedgrid.cpp
  src/tileindex.cpp
  src/tileloadjob.cpp
  src/tileprovider.cpp
  src/tiletextureinitdata.cpp
  src/timequantizer.cpp
)
source_group("Source Files" FILES ${SOURCE_FILES})

set(SHADER_FILES
  shaders/blending.hglsl
  shaders/globalrenderer_vs.glsl
  shaders/localrenderer_vs.glsl
  shaders/renderer_fs.glsl
  shaders/rings_vs.glsl
  shaders/rings_fs.glsl
  shaders/rings_geom_vs.glsl
  shaders/rings_geom_fs.glsl
  shaders/texturetilemapping.hglsl
  shaders/tile.hglsl
  shaders/tileheight.hglsl
  shaders/tilevertexskirt.hglsl
)

source_group("Shader Files" FILES ${SHADER_FILES})

create_new_module(
  "GlobeBrowsing"
  globebrowsing_module
  STATIC
  ${HEADER_FILES} ${SOURCE_FILES} ${SHADER_FILES}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/gdal_data DESTINATION modules/globebrowsing)

if (WIN32)
  target_include_directories(openspace-module-globebrowsing SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ext/gdal/include)
  target_link_libraries(openspace-module-globebrowsing PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ext/gdal/lib/gdal_i.lib)
  register_external_libraries("${CMAKE_CURRENT_SOURCE_DIR}/ext/gdal/lib/gdal241.dll")
else (WIN32)
  find_package(GDAL REQUIRED)

  target_include_directories(openspace-module-globebrowsing SYSTEM PRIVATE ${GDAL_INCLUDE_DIR})
  target_link_libraries(openspace-module-globebrowsing PRIVATE ${GDAL_LIBRARY})
  mark_as_advanced(GDAL_CONFIG GDAL_INCLUDE_DIR GDAL_LIBRARY)
endif () # WIN32

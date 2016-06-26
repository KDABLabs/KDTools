# - Find KDTools

# Copyright (C) 2011-2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
# All rights reserved.
#
# This file is part of the KD Tools library.
#
# Licensees holding valid commercial KD Tools licenses may use this file in
# accordance with the KD Tools Commercial License Agreement provided with
# the Software.
#
# This file may be distributed and/or modified under the terms of the
# GNU Lesser General Public License version 2.1 and version 3 as published by the
# Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
#
# This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
# WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
# Contact info@kdab.com if any conditions of this licensing are not
# clear to you.
#

#
# KDTools_FOUND         - Set to TRUE if the requested KDTools modules were found.
# KDTools_MODULES_ALL   - All the available KDTools modules.
# KDTools_MODULES       - All the found KDTools modules' names.
# KDTools_LIBRARIES     - Path to KDTools libraries.
# KDTools_INCLUDE_DIR   - Path to KDTools base include directory.
# KDTools_INCLUDE_DIRS  - Path to KDTools include directories.
#

include(FindPackageMessage)

# Clear variables in case the module is loaded more than once.
set(KDTools_FOUND FALSE)
set(KDTools_MODULES_ALL KDToolsCore KDToolsGui KDUnitTest KDUpdater)
set(KDTools_MODULES "")
set(KDTools_LIBRARIES "")
set(KDTools_INCLUDE_DIRS "")
set(_kdtools_module_not_found FALSE)

# Look for all KDTools modules unless some were specifically requested.
if(NOT KDTools_FIND_COMPONENTS)
        set(KDTools_FIND_COMPONENTS ${KDTools_MODULES_ALL})
endif()

# Find the root include path
find_path(KDTools_INCLUDE_DIR "KDToolsCore")
set(KDTools_INCLUDE_DIRS ${KDTools_INCLUDE_DIR})

foreach(_module ${KDTools_FIND_COMPONENTS})
        find_library(${_module}_LIBRARY ${_module})
        find_file(${_module}_INCLUDE_DIR ${_module})

        mark_as_advanced(${_module}_LIBRARY ${_module}_INCLUDE_DIR)

        if(${_module}_INCLUDE_DIR AND ${_module}_LIBRARY)
                set(${_module}_FOUND TRUE)
                set(KDTools_MODULES ${KDTools_MODULES} ${_module})
                set(KDTools_LIBRARIES ${KDTools_LIBRARIES} ${${_module}_LIBRARY})
                set(KDTools_INCLUDE_DIRS ${KDTools_INCLUDE_DIRS} ${${_module}_INCLUDE_DIR})
        else()
                set(_kdtools_module_not_found TRUE)
                set(KDTools_${_module}_FOUND FALSE)
        endif()
endforeach()

if(NOT _kdtools_module_not_found)
        set(KDTools_FOUND TRUE)
        find_package_message(KDTools "Found KDTools modules: ${KDTools_MODULES}" "[${KDTools_MODULES}][${KDTools_LIBRARIES}][${KDTools_INCLUDE_DIRS}]")
endif()

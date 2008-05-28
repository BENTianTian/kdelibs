# Find libphonon
# Once done this will define
#
#  PHONON_FOUND    - system has Phonon Library
#  PHONON_INCLUDES - the Phonon include directory
#  PHONON_LIBS     - link these to use Phonon

# Copyright (c) 2008, Matthias Kretz <kretz@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(PHONON_FOUND)
   # Already found, nothing more to do
else(PHONON_FOUND)
   if(PHONON_INCLUDE_DIR AND PHONON_LIBRARY)
      set(PHONON_FIND_QUIETLY TRUE)
   endif(PHONON_INCLUDE_DIR AND PHONON_LIBRARY)

   find_library(PHONON_LIBRARY NAMES phonon PATHS ${KDE4_LIB_INSTALL_DIR} ${KDE4_LIB_DIR} ${CMAKE_SYSTEM_LIBRARY_PATH} ${QT_LIBRARY_DIR} ${LIB_INSTALL_DIR} NO_DEFAULT_PATH)

   find_path(PHONON_INCLUDE_DIR NAMES phonon/phonon_export.h PATHS ${KDE4_INCLUDE_INSTALL_DIR} ${KDE4_INCLUDE_DIR} ${CMAKE_SYSTEM_INCLUDE_PATH} ${QT_INCLUDE_DIR} ${INCLUDE_INSTALL_DIR} NO_DEFAULT_PATH)

   if(PHONON_INCLUDE_DIR AND PHONON_LIBRARY)
      set(PHONON_LIBS ${phonon_LIB_DEPENDS} ${PHONON_LIBRARY})
      set(PHONON_INCLUDES ${PHONON_INCLUDE_DIR}/KDE ${PHONON_INCLUDE_DIR})
      set(PHONON_FOUND TRUE)
   else(PHONON_INCLUDE_DIR AND PHONON_LIBRARY)
      set(PHONON_FOUND FALSE)
   endif(PHONON_INCLUDE_DIR AND PHONON_LIBRARY)

   if(PHONON_FOUND)
      if(NOT PHONON_FIND_QUIETLY)
         message(STATUS "Found Phonon: ${PHONON_LIBRARY}")
         message(STATUS "Found Phonon Includes: ${PHONON_INCLUDES}")
      endif(NOT PHONON_FIND_QUIETLY)
   else(PHONON_FOUND)
      if(Phonon_FIND_REQUIRED)
         if(NOT PHONON_INCLUDE_DIR)
            message(STATUS "Phonon includes NOT found!")
         endif(NOT PHONON_INCLUDE_DIR)
         if(NOT PHONON_LIBRARY)
            message(STATUS "Phonon library NOT found!")
         endif(NOT PHONON_LIBRARY)
         message(FATAL_ERROR "Phonon library or includes NOT found!")
      else(Phonon_FIND_REQUIRED)
         message(STATUS "Unable to find Phonon")
      endif(Phonon_FIND_REQUIRED)
   endif(PHONON_FOUND)


   mark_as_advanced(PHONON_INCLUDE_DIR PHONON_LIBRARY PHONON_INCLUDES)
endif(PHONON_FOUND)

# Search xmms
#
# Copyright (c) 2006, Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


OPTION(KDE4_XMMS  "enable support for XMMS")

if(KDE4_XMMS)
	FIND_PROGRAM(XMMSCONFIG_EXECUTABLE NAMES xmms-config PATHS
   		/usr/bin
   		/usr/local/bin
	)
	#reset vars
	set(XMMS_LIBRARIES)
	set(XMMS_INCLUDE_DIR)

	if(XMMSCONFIG_EXECUTABLE)
  		EXEC_PROGRAM(${XMMSCONFIG_EXECUTABLE} ARGS --libs RETURN_VALUE _return_VALUE OUTPUT_VARIABLE XMMS_LIBRARIES)

  		EXEC_PROGRAM(${XMMSCONFIG_EXECUTABLE} ARGS --cflags RETURN_VALUE _return_VALUE OUTPUT_VARIABLE XMMS_INCLUDE_DIR)
		set(XMMS_DEFINITIONS ${XMMS_INCLUDE_DIR})
  	
		IF(XMMS_LIBRARIES AND XMMS_INCLUDE_DIR)
    			SET(XMMS_FOUND TRUE)
		ENDIF(XMMS_LIBRARIES AND XMMS_INCLUDE_DIR)

	MARK_AS_ADVANCED(XMMS_INCLUDE_DIR XMMS_LIBRARIES)
	endif(XMMSCONFIG_EXECUTABLE)

  	if (XMMS_FOUND)
    		if (NOT Xmms_FIND_QUIETLY)
      			message(STATUS "Found Xmms: ${XMMS_LIBRARIES}")
    		endif (NOT Xmms_FIND_QUIETLY)
  	else (XMMS_FOUND)
    		if (Xmms_FIND_REQUIRED)
      			message(FATAL_ERROR "Could NOT find Xmms")
    		endif (Xmms_FIND_REQUIRED)
  	endif (XMMS_FOUND)
endif(KDE4_XMMS)


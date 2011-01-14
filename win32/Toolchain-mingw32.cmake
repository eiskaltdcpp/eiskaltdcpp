# the name of the target operating system
set(CMAKE_SYSTEM_NAME Windows)

set(MINGW32_NAME i486-mingw32)

set(MINGW32_PREFIX /usr/${MINGW32_NAME})
#set(QT_WIN32_PREFIX ${MINGW32_PREFIX})
set(QT_WIN32_PREFIX /home/user_name/.wine/drive_c/Qt/2010.05/qt)
#set(QT_WIN32_PREFIX /home/user_name/.wine/drive_c/Qt/4.7.1/)

# which compilers to use for C and C++
set(CMAKE_C_COMPILER ${MINGW32_NAME}-gcc)
set(CMAKE_CXX_COMPILER ${MINGW32_NAME}-g++)
#set(CMAKE_RC_COMPILER ${MINGW32_NAME}-windres)
 
# here is the target environment located
set(CMAKE_FIND_ROOT_PATH ${MINGW32_PREFIX} ${QT_WIN32_PREFIX})

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#set(QT_X11_DIR /usr/bin)
#set(QT_MOC_EXECUTABLE ${QT_X11_DIR}/moc)
#set(QT_QMAKE_EXECUTABLE ${QT_X11_DIR}/qmake)
#set(QT_UIC_EXECUTABLE ${QT_X11_DIR}/uic)

#set(QT_LIBRARY_DIR ${QT_WIN32_PREFIX}/lib)
#set(QT_INCLUDE_DIR ${QT_WIN32_PREFIX}/include)
#set(QT_MKSPECS_DIR ${QT_WIN32_PREFIX}/mkspecs)
#set(QT_QTCORE_LIBRARY ${QT_WIN32_PREFIX}/lib/libQtCore4.a)
#set(QT_QTCORE_INCLUDE_DIR ${QT_WIN32_PREFIX}/include/QtCore)
#set(QT_QTNETWORK_LIBRARY ${QT_WIN32_PREFIX}/lib/libQtNetwork4.a)
#set(QT_QTNETWORK_INCLUDE_DIR ${QT_WIN32_PREFIX}/include/QtGui)
#set(QT_QTGUI_LIBRARY ${QT_WIN32_PREFIX}/lib/libQtGui4.a)
#set(QT_QTGUI_INCLUDE_DIR ${QT_WIN32_PREFIX}/include/QtNetwork)
#set(QT_QTXML_LIBRARY ${QT_WIN32_PREFIX}/lib/libQtXml4.a)
#set(QT_QTXML_INCLUDE_DIR ${QT_WIN32_PREFIX}/include/QtXml)
#set(QT_QTSCRIPT_LIBRARY ${QT_WIN32_PREFIX}/lib/libQtScript4.a)
#set(QT_QTSCRIPT_INCLUDE_DIR ${QT_WIN32_PREFIX}/include/QtScript)
#set(QT_QTDECLARATIVE_LIBRARY ${QT_WIN32_PREFIX}/lib/libQtDeclarative4.a)
#set(QT_QTDECLARATIVE_INCLUDE_DIR ${QT_WIN32_PREFIX}/include/QtDeclarative)
#set(QT_QTSQL_LIBRARY ${QT_WIN32_PREFIX}/lib/libQtSql4.a)
#set(QT_QTSQL_INCLUDE_DIR ${QT_WIN32_PREFIX}/include/QtSql)
#set(QT_QTXMLPATTERNS_LIBRARY ${QT_WIN32_PREFIX}/lib/libQtXmlPatterns4.a)
#set(QT_QTXMLPATTERNS_INCLUDE_DIR ${QT_WIN32_PREFIX}/include/QtXmlPatterns)

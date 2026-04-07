# GTK models library — widget-independent, testable business logic
# Depends on dcpp only — NO GTK dependency

set(GTKMODELS_SOURCES
    GtkFormatters.cpp
    GtkWulforUtil.cpp
    EmoticonLoader.cpp
    HubUserParams.cpp
    SearchResultParams.cpp
    DownloadQueueParams.cpp
    FavoriteHubParams.cpp
    FinishedTransferParams.cpp
    TransferParams.cpp
    FavoriteHubListModel.cpp
    HubUserListModel.cpp
    TransferListModel.cpp
    DownloadQueueModel.cpp
    SearchResultsModel.cpp
    GtkSettingsModel.cpp
    GtkSettingsDefaults.cpp
    ChatFormatter.cpp
    PublicHubFilter.cpp
    AdlSearchDisplay.cpp
    ShareBrowserModel.cpp
    SettingsValidation.cpp
)

set(GTKMODELS_HEADERS
    GtkFormatters.h
    GtkWulforUtil.h
    GtkMessageTypes.h
    EmoticonLoader.h
    SoundDispatch.h
    NotifyDispatch.h
    HubUserParams.h
    SearchResultParams.h
    DownloadQueueParams.h
    FavoriteHubParams.h
    FinishedTransferParams.h
    TransferParams.h
    FavoriteHubListModel.h
    HubUserListModel.h
    TransferListModel.h
    DownloadQueueModel.h
    SearchResultsModel.h
    GtkSettingsModel.h
    GtkSettingsDefaults.h
    ChatFormatter.h
    PublicHubFilter.h
    AdlSearchDisplay.h
    ShareBrowserModel.h
    SettingsValidation.h
)

add_library(gtkmodels STATIC ${GTKMODELS_SOURCES} ${GTKMODELS_HEADERS})

target_include_directories(gtkmodels PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(gtkmodels PRIVATE
    ${CMAKE_SOURCE_DIR}          # for "dcpp/..." and "extra/..." includes
    ${CMAKE_SOURCE_DIR}/dcpp     # for direct includes like "Util.h"
    ${BZIP2_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${GETTEXT_INCLUDE_DIR}
)

if (LUA_SCRIPT)
    target_include_directories(gtkmodels PRIVATE ${LUA_INCLUDE_DIR})
endif()
if (USE_IDN2)
    target_include_directories(gtkmodels PRIVATE ${IDN2_INCLUDE_DIR})
endif()

target_link_libraries(gtkmodels PUBLIC dcpp)

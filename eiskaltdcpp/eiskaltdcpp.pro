QT += xml
CONFIG += link_pkgconfig
PKGCONFIG += libupnp
INCLUDEPATH += ../dcpp
HEADERS = MainWindow.h \
    Func.h \
    ../dcpp/HashBloom.h \
    ../dcpp/FavoriteUser.h \
    ../dcpp/File.h \
    ../dcpp/CriticalSection.h \
    ../dcpp/SearchManager.h \
    ../dcpp/FastAlloc.h \
    ../dcpp/UploadManagerListener.h \
    ../dcpp/MerkleTree.h \
    ../dcpp/TigerHash.h \
    ../dcpp/Pointer.h \
    ../dcpp/BufferedSocket.h \
    ../dcpp/DirectoryListing.h \
    ../dcpp/DownloadManagerListener.h \
    ../dcpp/version.h \
    ../dcpp/Download.h \
    ../dcpp/HubEntry.h \
    ../dcpp/QueueManager.h \
    ../dcpp/Thread.h \
    ../dcpp/Encoder.h \
    ../dcpp/FilteredFile.h \
    ../dcpp/TaskQueue.h \
    ../dcpp/FinishedManagerListener.h \
    ../dcpp/Upload.h \
    ../dcpp/ClientManagerListener.h \
    ../dcpp/FinishedItem.h \
    ../dcpp/CryptoManager.h \
    ../dcpp/ResourceManager.h \
    ../dcpp/UserCommand.h \
    ../dcpp/SFVReader.h \
    ../dcpp/BloomFilter.h \
    ../dcpp/QueueItem.h \
    ../dcpp/NmdcHub.h \
    ../dcpp/forward.h \
    ../dcpp/HashValue.h \
    ../dcpp/QueueManagerListener.h \
    ../dcpp/Transfer.h \
    ../dcpp/BufferedSocketListener.h \
    ../dcpp/Streams.h \
    ../dcpp/AdcCommand.h \
    ../dcpp/ClientManager.h \
    ../dcpp/DownloadManager.h \
    ../dcpp/LogManagerListener.h \
    ../dcpp/MerkleCheckOutputStream.h \
    ../dcpp/HashManager.h \
    ../dcpp/ConnectionManager.h \
    ../dcpp/SimpleXML.h \
    ../dcpp/Speaker.h \
    ../dcpp/FinishedManager.h \
    ../dcpp/ADLSearch.h \
    ../dcpp/UserConnection.h \
    ../dcpp/User.h \
    ../dcpp/ClientListener.h \
    ../dcpp/stdinc.h \
    ../dcpp/StringTokenizer.h \
    ../dcpp/Exception.h \
    ../dcpp/Socket.h \
    ../dcpp/FavoriteManagerListener.h \
    ../dcpp/FavoriteManager.h \
    ../dcpp/ServerSocket.h \
    ../dcpp/LogManager.h \
    ../dcpp/Text.h \
    ../dcpp/SSL.h \
    ../dcpp/SettingsManager.h \
    ../dcpp/SearchManagerListener.h \
    ../dcpp/TimerManager.h \
    ../dcpp/UserConnectionListener.h \
    ../dcpp/ConnectionManagerListener.h \
    ../dcpp/Segment.h \
    ../dcpp/BitOutputStream.h \
    ../dcpp/BitInputStream.h \
    ../dcpp/Semaphore.h \
    ../dcpp/HttpConnection.h \
    ../dcpp/Client.h \
    ../dcpp/Flags.h \
    ../dcpp/ShareManager.h \
    ../dcpp/BZUtils.h \
    ../dcpp/SearchResult.h \
    ../dcpp/ZUtils.h \
    ../dcpp/DCPlusPlus.h \
    ../dcpp/StringSearch.h \
    ../dcpp/Singleton.h \
    ../dcpp/UploadManager.h \
    ../dcpp/SSLSocket.h \
    ../dcpp/Util.h \
    ../dcpp/CID.h \
    ../dcpp/AdcHub.h \
    ../dcpp/HashManagerListener.h \
    hubframe.h \
    HubFrame.h \
    UserListModel.h \
    WulforUtil.h \
    WulforSettings.h \
    ArenaWidget.h \
    PMWindow.h \
    TransferView.h \
    ShareBrowser.h \
    FileBrowserModel.h \
    SearchFrame.h \
    QuickConnect.h \
    SearchModel.h \
    Version.h \
    Settings.h \
    SettingsPersonal.h \
    SettingsConnection.h \
    SettingsInterface.h \
    FavoriteHubModel.h \
    FavoriteHubs.h \
    SettingsDownloads.h \
    SettingsSharing.h \
    HashProgress.h \
    HistoryInterface.h \
    UPnP.h \
    UPnPMapper.h \
    DownloadQueueModel.h \
    DownloadQueue.h \
    ToolBar.h \
    TransferViewModel.h \
    Magnet.h \
    IPFilter.h \
    Antispam.h \
    FinishedTransfers.h \
    FinishedTransfersModel.h \
    AntiSpamFrame.h \
    IPFilterFrame.h \
    IPFilterModel.h \
    HubManager.h \
    PoolAlloc.h \
    Notification.h \
    FavoriteUsers.h \
    SettingsGUI.h \
    SingleInstanceRunner.h \
    SettingsNotification.h \
    ShellCommandRunner.h \
    FavoriteUsersModel.h \
    SettingsLog.h \
    SpyFrame.h \
    SpyModel.h \
    EmoticonObject.h \
    EmoticonFactory.h \
    EmoticonDialog.h \
    SpellCheck.h \
    PublicHubModel.h \
    PublicHubs.h \
    PublicHubsList.h \
    SideBar.h \
    ArenaWidgetContainer.h \
    LineEdit.h \
    SettingsUC.h \
    UCModel.h \
    CustomFontModel.h \
    ScriptEngine.h
SOURCES = main.cpp \
    MainWindow.cpp \
    ../dcpp/HashManager.cpp \
    ../dcpp/SettingsManager.cpp \
    ../dcpp/DCPlusPlus.cpp \
    ../dcpp/SimpleXML.cpp \
    ../dcpp/HttpConnection.cpp \
    ../dcpp/Client.cpp \
    ../dcpp/AdcHub.cpp \
    ../dcpp/DownloadManager.cpp \
    ../dcpp/ConnectionManager.cpp \
    ../dcpp/QueueManager.cpp \
    ../dcpp/Transfer.cpp \
    ../dcpp/BZUtils.cpp \
    ../dcpp/UploadManager.cpp \
    ../dcpp/File.cpp \
    ../dcpp/Thread.cpp \
    ../dcpp/Text.cpp \
    ../dcpp/stdinc.cpp \
    ../dcpp/ResourceManager.cpp \
    ../dcpp/StringTokenizer.cpp \
    ../dcpp/AdcCommand.cpp \
    ../dcpp/FinishedItem.cpp \
    ../dcpp/CryptoManager.cpp \
    ../dcpp/DirectoryListing.cpp \
    ../dcpp/ADLSearch.cpp \
    ../dcpp/QueueItem.cpp \
    ../dcpp/SSL.cpp \
    ../dcpp/FinishedManager.cpp \
    ../dcpp/SearchManager.cpp \
    ../dcpp/LogManager.cpp \
    ../dcpp/NmdcHub.cpp \
    ../dcpp/TimerManager.cpp \
    ../dcpp/Upload.cpp \
    ../dcpp/Encoder.cpp \
    ../dcpp/TigerHash.cpp \
    ../dcpp/FavoriteManager.cpp \
    ../dcpp/SSLSocket.cpp \
    ../dcpp/Socket.cpp \
    ../dcpp/UserConnection.cpp \
    ../dcpp/Util.cpp \
    ../dcpp/Download.cpp \
    ../dcpp/version.cpp \
    ../dcpp/SFVReader.cpp \
    ../dcpp/SearchResult.cpp \
    ../dcpp/Exception.cpp \
    ../dcpp/ClientManager.cpp \
    ../dcpp/BufferedSocket.cpp \
    ../dcpp/User.cpp \
    ../dcpp/HashBloom.cpp \
    ../dcpp/ServerSocket.cpp \
    ../dcpp/ShareManager.cpp \
    ../dcpp/ZUtils.cpp \
    HubFrame.cpp \
    UserListModel.cpp \
    WulforUtil.cpp \
    WulforSettings.cpp \
    ArenaWidget.cpp \
    PMWindow.cpp \
    TransferView.cpp \
    ShareBrowser.cpp \
    FileBrowserModel.cpp \
    SearchFrame.cpp \
    QuickConnect.cpp \
    SearchModel.cpp \
    Settings.cpp \
    SettingsPersonal.cpp \
    SettingsConnection.cpp \
    FavoriteHubModel.cpp \
    FavoriteHubs.cpp \
    SettingsDownloads.cpp \
    SettingsSharing.cpp \
    HashProgress.cpp \
    UPnP.cpp \
    UPnPMapper.cpp \
    DownloadQueueModel.cpp \
    DownloadQueue.cpp \
    ToolBar.cpp \
    TransferViewModel.cpp \
    Magnet.cpp \
    IPFilter.cpp \
    Antispam.cpp \
    FinishedTransfers.cpp \
    FinishedTransfersModel.cpp \
    AntiSpamFrame.cpp \
    IPFilterModel.cpp \
    IPFilterFrame.cpp \
    HubManager.cpp \
    Notification.cpp \
    FavoriteUsers.cpp \
    SettingsGUI.cpp \
    SingleInstanceRunner.cpp \
    SettingsNotification.cpp \
    ShellCommandRunner.cpp \
    FavoriteUsersModel.cpp \
    SettingsLog.cpp \
    SpyFrame.cpp \
    SpyModel.cpp \
    EmoticonFactory.cpp \
    EmoticonDialog.cpp \
    SpellCheck.cpp \
    PublicHubModel.cpp \
    PublicHubs.cpp \
    PublicHubsList.cpp \
    SideBar.cpp \
    ArenaWidgetContainer.cpp \
    LineEdit.cpp \
    SettingsUC.cpp \
    UCModel.cpp \
    CustomFontModel.cpp \
    ScriptEngine.cpp
FORMS = ui/HubFrame.ui \
    ui/UIHashProgressDialog.ui \
    ui/PrivateMessage.ui \
    ui/UITransferView.ui \
    ui/UIShareBrowser.ui \
    ui/UISearchFrame.ui \
    ui/UISearchFrame.ui \
    ui/UIQuickConnect.ui \
    ui/UISettingsPersonal.ui \
    ui/UISettingsConnection.ui \
    ui/UISettings.ui \
    ui/UIFavoriteHubs.ui \
    ui/UIFavoriteHubEditor.ui \
    ui/UISettingsDownloads.ui \
    ui/UISettingsSharing.ui \
    ui/UIDownloadQueue.ui \
    ui/UIMagnet.ui \
    ui/UIFinishedTransfers.ui \
    ui/UIAntiSpam.ui \
    ui/UIIPFilter.ui \
    ui/UIFavoriteUsers.ui \
    ui/UISettingsGUI.ui \
    ui/UIAbout.ui \
    ui/UISettingsNotification.ui \
    ui/UISettingsLog.ui \
    ui/UISpy.ui \
    ui/UIPublicHubs.ui \
    ui/UIPublicHubsList.ui \
    ui/UIUserCommands.ui \
    ui/UISettingsUC.ui

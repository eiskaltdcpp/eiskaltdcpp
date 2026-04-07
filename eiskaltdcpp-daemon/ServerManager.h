/***************************************************************************
*                                                                         *
*   Copyright (C) 2009-2010  Alexandr Tkachev <tka4ev@gmail.com>          *
*   Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>                *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <atomic>
#include <memory>
#include <string>

namespace dcpp { class DCContext; }
class ServerThread;

/// Daemon configuration populated from command-line arguments.
struct DaemonConfig {
    unsigned short port = 3121;
    std::string ip = "127.0.0.1";
    std::string xmlrpcLog = "/tmp/eiskaltdcpp-daemon.xmlrpc.log";
    std::string xmlrpcUriPath = "/eiskaltdcpp";
    bool verbose = false;
    bool debug = false;
    bool daemon = false;
    bool useSyslog = false;
};

/// Manages the daemon's ServerThread lifecycle and shutdown coordination.
class ServerManager {
public:
    ServerManager(dcpp::DCContext& ctx, DaemonConfig config);
    ~ServerManager();

    ServerManager(const ServerManager&) = delete;
    ServerManager& operator=(const ServerManager&) = delete;

    bool start();
    void stop();
    void configReload();

    bool isRunning()  const noexcept { return running_; }
    void requestTermination() noexcept { terminated_ = true; }
    bool isTerminated() const noexcept { return terminated_; }

    dcpp::DCContext&       dcCtx()  noexcept { return dcCtx_; }
    DaemonConfig&          config() noexcept { return config_; }
    const DaemonConfig&    config() const noexcept { return config_; }

private:
    dcpp::DCContext& dcCtx_;
    DaemonConfig config_;
    std::unique_ptr<ServerThread> server_;
    std::atomic<bool> running_{false};
    std::atomic<bool> terminated_{false};
};

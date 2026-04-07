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

#include "stdafx.h"
#include "dcpp/DCContext.h"
#include "dcpp/DCPlusPlus.h"
#include "ServerManager.h"
#include "ServerThread.h"
#include "utility.h"

ServerManager::ServerManager(dcpp::DCContext& ctx, DaemonConfig config)
    : dcCtx_(ctx), config_(std::move(config)) {}

ServerManager::~ServerManager() {
    if (running_)
        stop();
}

bool ServerManager::start() {
    server_ = std::make_unique<ServerThread>(*this);
    server_->Resume();
    running_ = true;
    return true;
}

void ServerManager::stop() {
    if (!server_) return;
    server_->Close();
    logging(config_.daemon, config_.useSyslog, true, "server stops");
    logging(config_.daemon, config_.useSyslog, true, "waiting");
    server_->WaitFor();
    logging(config_.daemon, config_.useSyslog, true, "waiting finished");
    server_.reset();
    logging(config_.daemon, config_.useSyslog, true, "library stops");
    dcCtx_.shutdown();
    dcpp::setContext(nullptr);
    logging(config_.daemon, config_.useSyslog, true, "library was stopped");
    running_ = false;
}

void ServerManager::configReload() {
    if (server_) {
        server_->configReload();
        logging(config_.daemon, config_.useSyslog, true, "reload configuration success");
    } else {
        logging(config_.daemon, config_.useSyslog, true, "reload configuration failed");
    }
}

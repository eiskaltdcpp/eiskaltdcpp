--// log.lua
--// Release 004/20100312

logfunctions = {}

--// Utility functions

function logfunctions.tokenize(text)
	local ret = {}
	string.gsub(text, "([^%s]+)", function(s) table.insert(ret, s) end )
	return ret
end

function logfunctions.decide(value, restrue, resfalse)
	local ret = restrue
	if not value then
		ret = resfalse
	elseif value == "0" then
		ret = resfalse
	end
	return ret
end

--// Settings

function logfunctions.updateConfig()
	-- initialization if any settings altered/no settings file loaded
	if not logsettings then logsettings = {} end
	if not logsettings.hubs then logsettings.hubs = {} end
	if not logsettings.config then logsettings.config = {} end
	if not logsettings.config.lognoise then logsettings.config.lognoise = "off" end
	logfunctions.resetNoise()
end

function logfunctions.resetNoise()
	logsettings.noise = {}
	table.insert(logsettings.noise, "^<[^> ]+> [^ ]+ is kicking [^ ]+ because")
	table.insert(logsettings.noise, "^<[^> ]+> is kicking [^ ]+ because")
	table.insert(logsettings.noise, "^<[^> ]+> [^ ]+ banned [^ ]+ by ip [^ ]+ and nick %(")
	table.insert(logsettings.noise, "^<[^> ]+> [^ ]+ banned [0-9.]+ %(")
	table.insert(logsettings.noise, "^<[^> ]+> [^ ]+ banned [^ ]+ by nick %(")
	table.insert(logsettings.noise, "^<[^> ]+> [^ ]+, IP: [0-9.]+ was kicked")
end

function logfunctions.set(variable, value)
	local ret = false
	if logsettings.config[variable] then
		logsettings.config[variable] = value
		logfunctions.saveSettings()
		ret = true
	end
	return ret
end

function logfunctions.getSetting(variable)
	return logsettings.config[variable]
end

function logfunctions.loadSettings()
	local o = io.open( DC():GetConfigScriptsPath() .. "log_settings.txt", "r" )
	if o then
		o:close()
		dofile( DC():GetConfigScriptsPath() .. "log_settings.txt" )
	end
	logfunctions.updateConfig()
end

function logfunctions.saveSettings()
	pickle.store(DC():GetConfigScriptsPath() .. "log_settings.txt", { logsettings = logsettings })
end

--// Other

function logfunctions.isHubAdded(url)
	local added = false
	for k in pairs(logsettings.hubs) do
		if logsettings.hubs[k] == url then
			added = true
		end
	end
	return added
end

function logfunctions.addHub(hub, url)
	if not url then
		url = hub:getUrl()
	end
	if logfunctions.isHubAdded(url) then
		hub:addLine("Hub (" .. url .. ") is already added")
	else
		table.insert(logsettings.hubs, url)
		logfunctions.saveSettings()
		hub:addLine("Hub (" .. url .. ") added")
	end
end

function logfunctions.rmHub(hub, url)
	if not url then url = hub:getUrl() end
	if logfunctions.isHubAdded(url) then
		for k in pairs(logsettings.hubs) do
			if logsettings.hubs[k] == url then
				table.remove(logsettings.hubs, k)
				break
			end
		end
		logfunctions.saveSettings()
		hub:addLine("Hub (" .. url .. ") removed")
	else
		hub:addLine("Hub (" .. url .. ") is not added to the list, see /loglisthubs")
	end
end

function logfunctions.listAddedHubs(hub)
	hub:addLine("      ---------------------------------------------------------------", true)
	hub:addLine("      Currently logged hubs", true)
	hub:addLine("      ---------------------------------------------------------------", true)
	for k in pairs(logsettings.hubs) do
		hub:addLine("      " .. logsettings.hubs[k], true )
	end
	hub:addLine("      ---------------------------------------------------------------", true)
	return true
end

function logfunctions.toggleHub(hub)
	local url = hub:getUrl()
	if logfunctions.isHubAdded( url ) then
		logfunctions.rmHub( hub, url )
	else
		logfunctions.addHub( hub, url )
	end
end

function logfunctions.injectSmallHelp(hub, command)
	if command then
		-- maybe later
		-- hub:addLine("(log.lua) no help available for command")
		hub:addLine("(log.lua) /loghelp for full command listing")
	else
		hub:addLine("(log.lua) /loghelp for full command listing")
	end
	return true
end

function logfunctions.injectFullHelp(hub)
	hub:addLine("       ---------------------------------------------------------------------------------------------------------------------------- ", true)
	hub:addLine("       Help                                                                                                         Log.lua", true)
	hub:addLine("       ----------------------------------------------------------------------------------------------------------------------------", true)
	hub:addLine("       /logstatus                                                                 Shows the current settings", true)
	hub:addLine("       /logtoggle                                               Toggles the logging of the current hub", true)
	hub:addLine("       /loglisthubs                                  Lists hubs where mainchat logging is enabled", true)
	hub:addLine("       /logaddhub [address]                     Adds the current or given hub to the hub-list", true)
	hub:addLine("       /logrmhub [address]             Removes the current or given hub from the hub-list", true)
	hub:addLine("       /lognoise <on/off>                          Turns on the logging of kick/ban messages", true)
	hub:addLine("       ----------------------------------------------------------------------------------------------------------------------------", true)
	hub:addLine("       Note: to enable per-hub logging, disable the \"Log main chat\" function", true)
	hub:addLine("       ----------------------------------------------------------------------------------------------------------------------------", true)
	return true
end

function logfunctions.getLogFileName(hub)
	local filename = DC():GetSetting("LogFileMainChat")
	local logdir = DC():GetSetting("LogDirectory")
	filename = string.gsub( filename, "%%%[hubURL%]", string.gsub( hub:getUrl(), "[^%w-]", "_" ) )
	filename = string.gsub( filename, "%%%[hubNI%]", string.gsub( hub:getHubName(), "[^%w-]", "_" ) )
	if hub:getOwnNick() then
		filename = string.gsub( filename, "%%%[myNI%]", string.gsub( hub:getOwnNick(), "[^%w-]", "_" ) )
	end
	-- filename = string.gsub( filename, "[^%w.]", "_" )
	return logdir .. filename
end

function logfunctions.formatMessage(text, isutf)
	local message = DC():GetSetting("LogFormatMainChat")
	message = string.gsub( message, "%%%[message%]", "%%%%[message]" )
	message = os.date(message)
	text = string.gsub( text, "%%", "%%%%" )
	text = string.gsub( text, "\r", "" )
	message = string.gsub( message, "%%%[message%]", text )
	if not isutf then
		message = DC():ToUtf8(message)
	end
	return message
end

function logfunctions.log(hub, text, isutf)
	if logfunctions.isNoise(text) and logfunctions.getSetting("lognoise") == "off" then
		return false
	end
	local logfile = logfunctions.getLogFileName(hub)
	local o = io.open( logfile, "a+" )
	if o then
		o:write(  logfunctions.formatMessage(text, isutf) .. "\n" )
		o:close()
	else
		DC():PrintDebug("Can't open logfile: " .. logfile)
	end
end

function logfunctions.injectConfig(hub)
	hub:addLine("      ---------------------------------------------------------------------------------------", true)
	hub:addLine("      BCDC++ configuration (see File > Settings > Advanced)", true)
	hub:addLine("      ---------------------------------------------------------------------------------------", true)
	hub:addLine("      Log dir: " .. DC():GetSetting("LogDirectory"), true)
	hub:addLine("      Per-hub logging: " .. 1 - DC():GetSetting("LogMainChat"), true)
	hub:addLine("      Log format: " .. DC():GetSetting("LogFormatMainChat"), true)
	hub:addLine("      Logfile name: " .. DC():GetSetting("LogFileMainChat"), true)
	hub:addLine("      ---------------------------------------------------------------------------------------", true)
	hub:addLine("      Configuration", true)
	hub:addLine("      --------------------------------------------------------------------------------", true)
	for k,l in pairs(logsettings.config) do
		hub:addLine("      " .. k .. ": " .. l, true)
	end
	hub:addLine("      ---------------------------------------------------------------------------------------", true)
	hub:addLine("      Current hub settings", true)
	hub:addLine("      ---------------------------------------------------------------------------------------", true)
	hub:addLine("      Log this hub: " .. logfunctions.decide(logfunctions.isHubAdded( hub:getUrl() ), "yes", "no"), true)
	hub:addLine("      Filename: " .. logfunctions.getLogFileName(hub), true)
	hub:addLine("      ---------------------------------------------------------------------------------------", true)
end

function logfunctions.isNoise(text)
	local ret = false
	for k in pairs(logsettings.noise) do
		if string.find(text, logsettings.noise[k]) then
			ret = true
		end
	end
	return ret
end

--// Initializing //--

dofile(DC():GetScriptsPath() ..  "libsimplepickle.lua")
logfunctions.loadSettings()

dcpp:setListener("disconnected", "log_disc",
	function (hub)
		if (DC():GetSetting("LogMainChat") == 0) and logfunctions.isHubAdded(hub:getUrl()) then
			text = "*** Disconnected from " .. hub:getUrl()
			logfunctions.log(hub, text, false)
		end
	end
)

dcpp:setListener("connected", "log_connected",
	function (hub)
		if (DC():GetSetting("LogMainChat") == 0) and logfunctions.isHubAdded(hub:getUrl()) then
			text = "*** Connected to " .. hub:getUrl()
			logfunctions.log(hub, text, false)
		end
	end
)

dcpp:setListener( "chat", "log_nmdcchat",
	function( hub, user, text )
		if (DC():GetSetting("LogMainChat") == 0) and logfunctions.isHubAdded(hub:getUrl()) then
			if user then
				text = "<" .. user:getNick() .. "> " .. text
			end
			logfunctions.log(hub, text, false)
		end
	end
)

dcpp:setListener( "adcChat", "log_adcchat",
	function( hub, user, text, me_msg )
		if (DC():GetSetting("LogMainChat") == 0) and logfunctions.isHubAdded(hub:getUrl()) then
			if me_msg then
				text = "* " .. user:getNick() .. " " .. text
			else
				text = "<" .. user:getNick() .. "> " .. text
			end
			logfunctions.log(hub, text, true)
		end
	end
)

dcpp:setListener( "unknownchat", "log_statusmsg",
	function( hub, text )
		if (DC():GetSetting("LogMainChat") == 0) and logfunctions.isHubAdded(hub:getUrl()) then
			logfunctions.log(hub, text, false)
		end
	end
)


dcpp:setListener( "ownChatOut", "log_ownchat",
	function( hub, text )
		if string.sub(text, 1, 1) == "/" then
			local params = logfunctions.tokenize(text)
			if params[1] == "/help" then
				logfunctions.injectSmallHelp(hub, params[2])
			elseif params[1] == "/loghelp" then
				logfunctions.injectFullHelp(hub)
				return true
			elseif params[1] == "/logstatus" then
				logfunctions.injectConfig(hub)
				return true
			elseif params[1] == "/logtoggle" then
				logfunctions.toggleHub(hub)
				return true
			elseif params[1] == "/loglisthubs" then
				logfunctions.listAddedHubs(hub)
				return true
			elseif params[1] == "/logaddhub" then
				logfunctions.addHub(hub, params[2])
				return true
			elseif params[1] == "/logrmhub" then
				logfunctions.rmHub(hub, params[2])
				return true
			elseif params[1] == "/lognoise" then
				if not params[2] then
					hub:addLine("Missing parameter. Usage: /lognoise <on/off>")
				elseif params[2] == "on" or params[2] == "off" then
					if logfunctions.set("lognoise", params[2]) then
						hub:addLine("OK", true)
					else
						hub:addLine("Some crazy-type error. Bzz.", true)
					end
				else
					hub:addLine("Wrong parameter. Usage: /lognoise <on/off>")
				end
				return true
			end
			return nil
		else
			if (DC():GetSetting("LogMainChat") == 0) and logfunctions.isHubAdded(hub:getUrl()) then
				text = "<" .. hub:getOwnNick() .. "> " .. text
				logfunctions.log(hub, text, true)
			end
		end
		-- hub:addLine("*** " .. logfunctions.formatMessage(text, isutf) )
		return nil
	end
)

DC():PrintDebug( "  ** Loaded log.lua **" )

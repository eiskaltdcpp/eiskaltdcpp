--// ignore.lua -- makes possible to (un)ignore someone from the chat
--// ignore.lua -- Version 0.9c; Rev 004/20100312
--// ignore.lua -- Szabolcs Molnar, 2006-2010

--[[

	LOG:
	004/20100312: Settings is loaded from config directory now
	003/20070104: Fixed so nick with any non-space character can be added to the ignorelist
	002/20060830: Auto-reset settingsfile path every startup to avoid a bug; Added option to ignore bots/chatrooms; New messages
	001:          Previous version

]]

if not ignoretable then
	ignoretable = {}
	ignoretable.settingsfile = DC():GetConfigScriptsPath() .. "ignoretable.txt"
	-- in UTF8
	ignoretable.users = {}
	ignoretable.permament = 1
	ignoretable.private = 1
end

if not ignorefunctions then
	ignorefunctions = {}
end

function ignorefunctions.updateConfig()
	ignoretable.settingsfile = DC():GetConfigScriptsPath() .. "ignoretable.txt"
	if not ignoretable.ignorebots then
		ignoretable.ignorebots = 1
	end
end

function ignorefunctions.SaveSettings()
	pickle.store(ignoretable.settingsfile, { ignoretable = ignoretable })
end

function ignorefunctions.ClearUsers()
	-- clears all users from the table
	while table.getn(ignoretable.users) > 0 do
		table.remove(ignoretable.users)
	end
	ignorefunctions.SaveSettings()
end

function ignorefunctions.LoadSettings()
	local o = io.open( ignoretable.settingsfile, "r" )
	if o then
		dofile( ignoretable.settingsfile )
		o:close()
	end
	-- "ignoretable.permament" decides whether to store the ignored messages after restarting the program
	if (ignoretable.permament == 0) then
		ignorefunctions.ClearUsers()
	end
end

dofile( DC():GetScriptsPath() .. "libsimplepickle.lua" )
ignorefunctions.LoadSettings()
ignorefunctions.updateConfig()

function ignorefunctions.OnOff(variable)
	local ret = "on"
	if (variable == 0) or (variable == nil) then
		ret = "off"
	end
	return ret
end

-- nick in Utf8
function ignorefunctions.IsIgnored(nick)
	local userignored = false
	for k in pairs(ignoretable.users) do
		if (ignoretable.users[k] == nick) then
			userignored = true
			break
		end
	end
	return userignored
end

function ignorefunctions.ListIgnore(hub)
	local listofusers=""
	local first = true
	for k in pairs(ignoretable.users) do
		if first == true then
			first = false
		else
			listofusers = listofusers..", "
		end
		listofusers = listofusers .. ignoretable.users[k]
	end
	hub:injectChat("*** Currently ignored users: " .. DC():FromUtf8(listofusers))
end

function ignorefunctions.AddUser(nick)
	local ret = false
	if ignorefunctions.IsIgnored(nick) then
		DC():PrintDebug("[ignore.lua] " .. nick .. " is already ignored")
	else
		table.insert(ignoretable.users, nick)
		DC():PrintDebug("[ignore.lua] " .. nick .. " is ignored now")
		ignorefunctions.SaveSettings()
		ret = true
	end
	return ret
end

function ignorefunctions.RmUser(nick)
	local managed = false
	if ignorefunctions.IsIgnored(nick) then
		for k in pairs(ignoretable.users) do
			if (ignoretable.users[k] == nick) then
				table.remove(ignoretable.users, k)
				managed = true
				DC():PrintDebug("[ignore.lua] " .. nick .. " is no more ignored from the chat")
				ignorefunctions.SaveSettings()
			end
		end
	else
		-- he's not ignored
		managed = false
	end
	return managed
end

-- This is similar to formatting.lua's, but needed a little change
function ignorefunctions.tokenize( str )
	local ret = {}
	string.gsub( str, "([^ ]+)", function( s ) table.insert( ret, s ) end )
	return ret
end

dcpp:setListener("connected", "usercommands_ignore", function(hub)
	if (hub:getProtocol() == "adc") then
		-- what about adc hub?
		-- DC():PrintDebug("ADC hub..")
	else
		-- DC():PrintDebug("NMDC hub..")
		-- and what about this? :)
		hub:injectChat("$UserCommand 1 2 Ignore$%[lua:ignorefunctions.AddUser(\"!%!{userNI!}\")]&#124;|")
		hub:injectChat("$UserCommand 1 2 Unignore$%[lua:ignorefunctions.RmUser(\"!%!{userNI!}\")]&#124;|")
	end
end)

dcpp:setListener("ownChatOut", "entered_ignore", function(hub, message, ret)
	if string.sub( message, 1, 1) ~= "/" then
		return nil
	end
	local params = ignorefunctions.tokenize( message )
	if params[1] == "/help" then
		hub:injectChat( "*** (ignore.lua) /ignore <nick>, /unignore <nick>, /listignore, /ignorepm <on/off> (curr: ".. ignorefunctions.OnOff(ignoretable.private).."), /permament <on/off> (curr: ".. ignorefunctions.OnOff(ignoretable.permament) .."), /ignorebots <on/off> (curr: " .. ignorefunctions.OnOff(ignoretable.ignorebots) .. "), /purgelist" )
		return nil
	elseif params[1] == "/ignore" then
		if params[2] then
			if ignorefunctions.AddUser(params[2]) then
				hub:injectChat("*** " .. DC():FromUtf8(params[2]) .. " ignored")
			else
				hub:injectChat("*** " .. DC():FromUtf8(params[2]) .. " is already ignored")
			end
		else
			hub:injectChat("*** Usage: /ignore <nick>")
		end
		return 1
	elseif params[1] == "/unignore" then
		if params[2] then
			if ignorefunctions.RmUser(params[2]) then
				hub:injectChat("*** " .. DC():FromUtf8(params[2]) .. " unignored")
			else
				hub:injectChat("*** " .. DC():FromUtf8(params[2]) .. " couldn't be removed, please check /listignore")
			end
		else
			hub:injectChat("*** Usage: /unignore <nick>")
		end
		return 1
	elseif params[1] == "/listignore" then
		ignorefunctions.ListIgnore(hub)
		return 1
	elseif params[1] == "/ignorepm" then
		if params[2] == "on" then
			ignoretable.private = 1
			hub:injectChat("*** Private messages are ignored from the users on the ignore-list")
			ignorefunctions.SaveSettings()
		elseif params[2] == "off" then
			ignoretable.private = 0
			hub:injectChat("*** Private messages no more ignored")
			ignorefunctions.SaveSettings()
		else
			hub:injectChat("*** Usage: /ignorepm <on/off>. Currently PM ignoring is turned "..ignorefunctions.OnOff(ignoretable.private)..".")
		end
		return 1
	elseif params[1] == "/ignorebots" then
		if params[2] == "on" then
			ignoretable.ignorebots = 1
			hub:injectChat("*** Private messages from chatrooms/bots added to the ignore-list are ignored")
			ignorefunctions.SaveSettings()
		elseif params[2] == "off" then
			ignoretable.ignorebots = 0
			hub:injectChat("*** Chatrooms/bots are no more ignored")
			ignorefunctions.SaveSettings()
		else
			hub:injectChat("*** Usage: /ignorebots <on/off>. Currently Chat room/bot ignoring is turned "..ignorefunctions.OnOff(ignoretable.ignorebots)..".")
		end
		return 1
	elseif params[1] == "/permament" then
		if params[2] == "on" then
			ignoretable.permament = 1
			hub:injectChat("*** Ignored users' list will be keeped after program restart")
			ignorefunctions.SaveSettings()
		elseif params[2] == "off" then
			ignoretable.permament = 0
			hub:injectChat("*** Ignored users' list will be cleared after program restart")
			ignorefunctions.SaveSettings()
		else
			hub:injectChat("*** Usage: /permament <on/off>. Currently ignorelist saving is turned "..ignorefunctions.OnOff(ignoretable.permament)..".")
		end
		return 1
	elseif params[1] == "/purgelist" then
		ignorefunctions.ClearUsers()
		hub:injectChat("*** Ignored users' list cleared")
		return 1
	end
end)

dcpp:setListener("chat", "chat_ignore", function( hub, user, text )
	local nick = user:getNick()
	if ignorefunctions.IsIgnored(DC():ToUtf8(nick)) then
		DC():PrintDebug( "[" .. hub:getUrl() .. "] Ignored message from " .. DC():ToUtf8(nick).. " [main chat]")
		return 1
	end
	return nil
end)

dcpp:setListener("pm", "pm_ignore", function( hub, user, message )
	local nick = user:getNick()
	if ignorefunctions.IsIgnored(DC():ToUtf8(nick)) and (ignoretable.private == 1) then
		DC():PrintDebug( "[" .. hub:getUrl() .. "] Ignored message from " .. DC():ToUtf8(nick) .. " [pm]")
		return 1
	end
	return nil
end)

dcpp:setListener("hubPm", "pm_ignore", function( hub, user, message )
	local nick = user:getNick()
	if ignorefunctions.IsIgnored(DC():ToUtf8(nick)) and (ignoretable.ignorebots == 1) then
		DC():PrintDebug( "[" .. hub:getUrl() .. "] Ignored message from " .. DC():ToUtf8(nick) .. " [chatroom/bot]")
		return 1
	end
	return nil
end)

DC():PrintDebug( "  ** Loaded ignore.lua **" )

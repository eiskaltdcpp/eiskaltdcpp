--// vim:ts=4:sw=4:noet
--// p2pblock.lua -- Allow/Block p2p connections by nick

dofile( DC():GetScriptsPath() .. "libsimplepickle.lua" )

p2pblock = {}
p2pblock.file = DC():GetConfigScriptsPath() .. "p2pblock.dat"

dcpp:setListener( "ownChatOut", "p2pblock",
	function( hub, msg, ret )
		if string.sub( msg, 1, 1 ) ~= "/" then
			return nil
		elseif string.sub( msg, 2, 5 ) == "help" then
			hub:injectChat( "*** (p2pblock.lua) /p2pblock <list/add/del/invert/load> [nick]" )
		elseif string.sub( msg, 2, 10 ) == "p2pblock " then
			local rest = string.sub( msg, 11 )
			if rest == "list" then
				p2pblock.list( hub )
			elseif rest == "load" then
				p2pblock.load( hub )
			elseif rest == "invert" then
				p2pblock.invert( hub )
			elseif string.sub( rest, 1, 4 ) == "add " then
				p2pblock.add( hub, string.sub( rest, 5 ) )
			elseif string.sub( rest, 1, 4 ) == "del " then
				p2pblock.del( hub, string.sub( rest, 5 ) )
			else
				return nil
			end
			return 1
		end
	end
)

dcpp:setListener( "clientIn", "p2pblock",
	function( userp, line )
		-- NMDC protocol, identify user by nickname
		if string.sub( line, 1, 8 ) == "$MyNick " then
			local nick = string.sub( line, 9 )
			nick = DC():ToUtf8(nick)
			-- DC():PrintDebug( "p2pblock: nick = ^" .. nick.."$" )
			if p2pblock.isblocked(nick) then
				DC():PrintDebug( "[p2pblock.lua] match: " .. nick )
				return not p2pblock.settings.allow_only
			else
				-- DC():PrintDebug( "p2pblock: no match" )
				return p2pblock.settings.allow_only
			end
		end
	end
)

function p2pblock.load( hub )
	pickle.restore( p2pblock.file, p2pblock.error )
	p2pblock.settings = p2pblock_opt
	if not p2pblock.settings then p2pblock.settings = {} end
	if not p2pblock.settings.nicks then p2pblock.settings.nicks = {} end
	p2pblock_opt = nil
	if hub then
		hub:injectChat( "*** p2pblock.lua: data file loaded" )
	end
end

function p2pblock.save()
	pickle.store( p2pblock.file, { p2pblock_opt = p2pblock.settings },
			p2pblock.error )
end

function p2pblock.isblocked(nick)
	for k in pairs(p2pblock.settings.nicks) do
		if p2pblock.settings.nicks[k] == nick then
			return true
		end
	end
	return false
end

function p2pblock.error( e )
	DC():PrintDebug( "*** p2pblock.lua: file load/save error: " .. e )
end

function p2pblock.add( hub, nick )
	hub:injectChat( "*** p2pblock.lua: adding nick \"" .. DC():FromUtf8(nick) .. "\"" )
	table.insert(p2pblock.settings.nicks, nick)
	-- p2pblock.settings.nicks[nick] = 1
	p2pblock.save()
end

function p2pblock.del( hub, nick )
	if not p2pblock.isblocked(nick) then
		hub:injectChat( "*** p2pblock.lua: nick is not blocked. You can't remove it." )
	else
		hub:injectChat( "*** p2pblock.lua: removing nick \"" .. DC():FromUtf8(nick) .. "\"" )
		for k in pairs(p2pblock.settings.nicks) do
			if p2pblock.settings.nicks[k] == nick then
				table.remove(p2pblock.settings.nicks, k)
			end
		end
		p2pblock.save()
	end
end

function p2pblock.invert( hub )
	if p2pblock.settings.allow_only then
		if hub then
			hub:injectChat( "*** p2pblock.lua: switching to Allow Everyone except nicks in list" )
		end
		p2pblock.settings.allow_only = nil
	else
		if hub then
			hub:injectChat( "*** p2pblock.lua: switching to Allow Only nicks in list" )
		end
		p2pblock.settings.allow_only = 1
	end
	p2pblock.save()
end

function p2pblock.list( hub )
	if hub then
		local f,e = io.open( p2pblock.file, "r" )
		local out = ""
		if e then
			out = "error opening " .. p2pblock.file .. ": " .. e
		else
			for l in f:lines() do
				out = out .. "\r\n" .. l
			end
			f:close()
		end
		hub:injectChat( "*** p2pblock.lua: " .. out )
	end
end

p2pblock.load()

DC():PrintDebug( "  ** Loaded p2pblock.lua **" )

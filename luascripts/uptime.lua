--// vim:ts=4:sw=4:noet
--// uptime.lua -- Record uptime of script (and most likely, BCDC++)

if not uptime then
	uptime = {}
	uptime.start = os.time()
	uptime.diff = function()
		return os.difftime( os.time(), uptime.start )
	end
	uptime.format = function()
		local s = uptime.diff()
		local t = os.date( "!*t", s )
		return string.format( "%i day(s) %02i:%02i:%02i",
				s / 86400, t.hour, t.min, t.sec )
	end
end

dcpp:setListener( "ownChatOut", "up",
	function( hub, text )
		local s = string.lower( text )
		if text == "/up" then
			PrintDebug( "Uptime: "..uptime.format() )
			return 1
		elseif text == "/up show" then
			hub:sendChat( "My EiskaltDC++ uptime is: "..uptime.format() )
			return 1
		elseif string.sub(s, 1, 5) == "/help" then
			hub:addLine( "(uptime.lua) /up [show]" )
			return nil
		end
	end
)

PrintDebug( "  ** Loaded uptime.lua **" )

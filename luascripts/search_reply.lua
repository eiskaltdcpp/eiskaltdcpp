dcpp:setListener( "Raw", "search_reply",
	function(hub, msg)
		r,_,ip_port,filename = string.find(msg, "^%$Search ([0-9.:]*) F%?T%?0%?[^9]%?(.*)")
		if r then
			sr = GetSR(" ()", ip_port, "foo", string.gsub(filename, "%$", " "), "0", "0", "0")
			DC():PrintDebug(sr)
		end
	end
	-- DC():SendUDP(ip:port, msg)
)

DC():PrintDebug("  ** Loaded search_reply.lua **")
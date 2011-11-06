--// adccommands.lua: rev005/20080510
--// adccommands.lua: Let you change your nick on ADC hubs

local adccommands = {}

function adccommands.tokenize(text)
	local ret = {}
	string.gsub(text, "([^ ]+)", function(s) table.insert(ret, s) end )
	return ret
end

function adccommands.changeNick(hub, newnick)
	local sid = hub:getOwnSid()
	newnick = dcu:AdcEscape(newnick)
	DC():SendHubMessage( hub:getId(), "BINF " .. sid .. " NI" .. newnick .. "\n" )
end

dcpp:setListener( "ownChatOut", "adccommands",
	function( hub, text )
		if (hub:getProtocol() == "adc" and string.sub(text, 1, 1) == "/") then
			local params = adccommands.tokenize(text)
			if params[1] == "/help" then
				hub:addLine( "(adccommands.lua) /nick <nick>", true )
				return nil
			elseif params[1] == "/nick" then
				if params[2] then
					adccommands.changeNick(hub, string.sub(text, 7))
					return true
				else
					hub:addLine("Usage: /nick <nick>")
					return true
				end
			end
		end
	end
)

DC():PrintDebug( "  ** Loaded adccommands.lua **" )

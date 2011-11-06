--// kickfilter.lua
--// Filters kick messages from the chat if Filtering is turned on in the Settings

kickfilter = {}
kickfilter._noise = {}
-- If you want, you can add more kick-messages here:
table.insert(kickfilter._noise, "^[^ ]+ is kicking [^ ]+ because")
table.insert(kickfilter._noise, "^is kicking [^ ]+ because")

function kickfilter.isNoise(text)
	if (DC():GetSetting("FilterMessages") == 0) then
		return false
	end
	local ret = false
	for k in pairs(kickfilter._noise) do
		if string.find(text, kickfilter._noise[k]) then
			ret = true
		end
	end
	return ret
end

dcpp:setListener( "chat", "kickfilter",
	function( hub, user, text )
		if kickfilter.isNoise(text) then
			if hub:getProtocol() == "nmdc" then
				DC():PrintDebug("[" .. hub:getUrl() .. "] <" .. DC():ToUtf8(user:getNick()) .. "> " .. DC():ToUtf8(text) )
			else
				DC():PrintDebug("[" .. hub:getUrl() .. "] <" .. user:getNick() .. "> " .. text )
			end
				
				return true
		end
		return false
	end
)

DC():PrintDebug( "  ** Loaded kickfilter.lua **" )

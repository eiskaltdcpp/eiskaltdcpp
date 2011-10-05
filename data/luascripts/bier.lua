--// vim:ts=4:sw=4:noet
--// bier.lua -- Send some reply when someone is talking about bier

dcpp:setListener( "chat", "bier",
	function( hub, user, text )
		local s = string.lower( text )
		if string.find( s, "[^a-z]bier+[^a-z]" ) or string.find( s, "[^a-z]biertje[^a-z]" ) then
			hub:sendChat( "bier? ja lekker! :)" )
		end
	end																			
)

dcpp:setListener( "adcChat", "bier",
	function( hub, user, text, me_msg )
		local s = string.lower( text )
		if string.find( s, "[^a-z]bier+[^a-z]" ) or string.find( s, "[^a-z]biertje[^a-z]" ) then
			hub:sendChat( "bier? ja lekker! :)" )
		end
	end																			
)

DC():PrintDebug( "  ** Loaded bier.lua **" )

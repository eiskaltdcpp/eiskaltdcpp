--// vim:ts=4:sw=4:noet
--// onjoin.lua -- Send a private message to a user when he/she joins

dcpp:setListener( "userConnected", "onjoin",
	function( hub, user )
		-- the 2nd argument hides the message from BCDC++
		-- (i.e. doesn't show that you're sending the message)
		user:sendPrivMsgFmt( "Hi! Welcome to the hub :)\r\n"..
				"(this is an automated message, please do not reply)", 1 )
	end
)

DC():PrintDebug( "  ** Loaded onjoin.lua **" )

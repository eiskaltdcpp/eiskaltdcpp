--// vim:ts=4:sw=4:noet
--// monologue.lua -- Interrupt peoples monologues

monologue = {}
monologue.user = ""
monologue.count = 0
monologue.max = 6

dcpp:setListener( "chat", "monologue",
	function( hub, user, text )
		if (user:getNick() == monologue.user) and (string.find(text, "is kicking .+ because:")==nil) then
			monologue.count = monologue.count + 1
			if monologue.count >= monologue.max then
				hub:sendChat( user:getNick().." voert een diepzinnige monoloog!")
				monologue.count = 0
			end
		else
			monologue.count = 1
			monologue.user = user:getNick()
		end
	end
)

dcpp:setListener( "ownChatOut", "monologue",
	-- make sure we count ourself when checking for monologues
	function( hub, text )
		monologue.count = 0
	end
)

DC():PrintDebug( "  ** Loaded monologue.lua **" )

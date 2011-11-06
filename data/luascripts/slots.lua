--// vim:ts=4:sw=4:noet
--// slots.lua -- Inform people that it is nice to tell _what_ they need a slot for

dcpp:setListener( "pm", "slots",
	function( hub, user, text )	
		if not user:isOp() and not user:msgHandled( "slots" ) then
			local s = string.lower( text )
			if string.find( s, "[^a-z]slots?[^a-z]" ) then				
				othernick = user:getNick()
				hub:injectPrivMsgFmt( othernick, hub:getOwnNick(), text )
				user:sendPrivMsgFmt( "regarding slots: if you want something _rare_ (NOT "..
						"something _new_), please specify what" )
				user:setMsgHandled( "slots" )
				return 1 -- make bcdc++ forget the original message
			end
		end
	end
)

DC():PrintDebug( "  ** Loaded slots.lua **" )

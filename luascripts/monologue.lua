--// vim:ts=4:sw=4:noet
--// monologue.lua -- Interrupt peoples monologues

monologue = {}
monologue.user = ""
monologue.count = 0
monologue.max = 4
monologue.test = {}
monologue.test["1"]="Ahoj"
monologue.test["2"]="Hi"
monologue.test["3"]="Hello"
monologue.test["4"]="haha"
monologue.test["5"]="voer en deepzing!!"
monologue.test["6"]="zdrastvuj"


dcpp:setListener( "chat", "monologue",
	function( hub, user, text )
		if (user:getNick() == monologue.user) and (string.find(text, "is kicking .+ because:")==nil) then
			monologue.count = monologue.count + 1
			if monologue.count >= monologue.max then
				local s=math.random(1,7)
				local textx=monologue.test[tostring(s)]
				hub:sendChat( user:getNick().." "..text.." haha!")
				monologue.count = 0
			end
		else
			monologue.count = 1
			monologue.user = user:getNick()
		end
	end
)

dcpp:setListener( "adcChat", "monologue",
			function( hub ,user , text , metext)
				if(user:getNick() == monologue.user) then
					monologue.count = monologue.count + 1
					if monologue.count >= monologue.max then
						local s=math.random(1,7)
						local text = monologue.test[tostring(s)]
						hub:sendChat ( user:getNick().." "..text.."haha !")
						monologue.count=0
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

dcpp:setListener( "adcOwnChat", "monologue",
	-- make sure we count ourself when checking for monologues
	function( hub, text, metext )
		monologue.count = 0
	end
)

PrintDebug( "  ** Loaded monologue.lua **" )

--// vim:ts=4:sw=4:noet
--// mytimer.lua -- Do something at regular intervals

myTimer = {}
myTimer.interval = 120 * 60 -- 2h
myTimer.next = 0
myTimer.test = {}
myTimer.test["1"]="-A"
myTimer.test["2"]="-B"
myTimer.test["3"]="-C"
myTimer.test["4"]="-D"
myTimer.test["5"]="-E"
myTimer.test["6"]="-F"

function myTimer.doSomething( hub )
	hub:sendChat( "Type /fav for add hub to favorite" )
end

dcpp:setListener( "timer", "myTimer",
	function()
		if os.time() > myTimer.next then
			for k,hub in pairs(dcpp:getHubs()) do
				if hub:getAddress() == "92.240.248.81:411" then
					myTimer.doSomething( hub )
					break
				end
			end
			myTimer.next = os.time() + myTimer.interval
		end
	end																			
)

PrintDebug( "  ** Loaded mytimer.lua **" )

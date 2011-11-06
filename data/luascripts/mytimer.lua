--// vim:ts=4:sw=4:noet
--// mytimer.lua -- Do something at regular intervals

myTimer = {}
myTimer.interval = 10 * 60 -- 10 mins
myTimer.next = 0

function myTimer.doSomething( hub )
	hub:sendChat( "." )
end

dcpp:setListener( "timer", "myTimer",
	function()
		if os.time() > myTimer.next then
			for k,hub in pairs(dcpp:getHubs()) do
				if hub:getAddress() == "10.102.221.1:1416" then
					myTimer.doSomething( hub )
					break
				end
			end
			myTimer.next = os.time() + myTimer.interval
		end
	end																			
)

DC():PrintDebug( "  ** Loaded mytimer.lua **" )

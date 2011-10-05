--// quiet_login.lua -- stop spamming logfiles with MOTDs

if not dcpp._chat_ready then
	math.randomseed( os.time() )
	dcpp._chat_ready = {}
end	

dcpp._chat_ready.func =
	function(hub, user, msg, me_msg)
		if string.find(msg, dcpp._chat_ready[hub]._cookie) then
			dcpp._chat_ready[hub]._ready = 1
			DC():PrintDebug("Enabled mainchat for "..hub:getHubName())
			return 1
		end
	end

dcpp:setListener("pm", "quiet_login", dcpp._chat_ready.func)
dcpp:setListener("adcPm", "quiet_login", dcpp._chat_ready.func)

dcpp:setListener( "userInf", "quiet_login",
	function( hub, user, flags )
		if hub:getOwnSid() == user:getSid() and not dcpp._chat_ready[hub]._ready then
			DC():SendHubMessage( hub:getId(), string.format("DMSG %s %s %s PM%s\n", hub:getOwnSid(), hub:getOwnSid(), dcpp._chat_ready[hub]._cookie, hub:getOwnSid()))
		end
	end
)

dcpp:setListener( "userMyInfo", "quiet_login",
	function( hub, user, message )
		if hub:getOwnNick() == user:getNick() then
			-- Some Ptokax hubs seem not to reflect PMs or $SR's back.
			-- For them, detect 2nd myinfo. However: this only works
			-- If someone is an operator, because otherwise there's
			-- no update post-$OpList.
			dcpp._chat_ready[hub]._myInfoCount = dcpp._chat_ready[hub]._myInfoCount + 1
			if dcpp._chat_ready[hub]._myInfoCount == 2 then
				DC():PrintDebug("Enabled mainchat for "..hub:getHubName())
				dcpp._chat_ready[hub]._ready = 1
			else
				user:sendPrivMsgFmt(dcpp._chat_ready[hub]._cookie, 1)
			end
		end
	end
)

chatHandler = 
	function( hub, user, text )
		if not dcpp._chat_ready[hub]._ready then
			-- Other half of Ptokax workaround: the second post-$OpList chat
			-- message is always a non-MOTD message (the first might be, but
			-- only if the hub has disabled the MOTD). Risks delaying a chat
			-- message, but allows non-op users to squelch Ptokax MOTDs.
			if hub.gotOpList and hub:gotOpList() then
				cc = dcpp._chat_ready[hub]._postOpListChatCount
				dcpp._chat_ready[hub]._postOpListChatCount = cc + 1
				if cc == 1 then
					dcpp._chat_ready[hub]._ready = 1
					-- Don't block this message.
					return nil
				end
			end

			dcpp._chat_ready[hub]._text = dcpp._chat_ready[hub]._text ..
			                              text .. "\n"
			return 1
		end
	end

dcpp:setListener( "chat", "quiet_login", chatHandler)
dcpp:setListener( "adcChat", "quiet_login", chatHandler)

dcpp:setListener( "unknownchat", "quiet_login",
	function( hub, text )
		if not dcpp._chat_ready[hub]._ready then
			dcpp._chat_ready[hub]._text = dcpp._chat_ready[hub]._text ..
			                              text .. "\n"
			return 1
		end
	end							
)

dcpp:setListener( "connected", "quiet_login",
	function( hub )
		rand = function() return math.random(1000000000,2100000000) end
		cookie = string.format("%d%d%d", rand(), rand(), rand())
		dcpp._chat_ready[hub] = { _ready = nil, _text = "", _cookie = cookie, _myInfoCount = 0, _postOpListChatCount = 0 }
	end
)

-- However, if login failed, best know why.
dcpp:setListener( "disconnected", "quiet_login",
	function( hub )
		-- If disconnected prematurely, show archived messages.
		-- They probably include error messages.
		if not dcpp._chat_ready[hub]._ready then
			DC():PrintDebug(dcpp._chat_ready[hub]._text)
		end

		dcpp._chat_ready[hub] = nil
	end
)

DC():PrintDebug( "  ** Loaded quiet_login.lua **" )

--// vim:ts=4:sw=4:noet

-- use a namespace because we are in a global environment
formatting = {}

-- load pickle functions
dofile( DC():GetScriptsPath() .. "libsimplepickle.lua" )
formatting.settings_file = DC():GetConfigScriptsPath() .. "formatting_settings.txt"

-- every new line starts with the following, because of the multiline-URL-fix-hack
formatting.input_start = ""

-- regexes that should be considered noise
formatting.noise = {
	-- NMDC Kick
	"^"..formatting.input_start.."\r\n[0-9:%[%] ]*<[^> ]+> [^ ]+ is kicking [^ ]+ because",
	-- Verlihub Kick
	"^"..formatting.input_start.."\r\n[0-9:%[%] ]*<[^> ]+> is kicking [^ ]+ because",
	-- DCH++ Banuser/Bannick/Banip
	"^"..formatting.input_start.."\r\n[0-9:%[%] ]*<[^> ]+> [^ ]+ banned [^ ]+ by ip [^ ]+ and nick %(",
	"^"..formatting.input_start.."\r\n[0-9:%[%] ]*<[^> ]+> [^ ]+ banned [0-9.]+ %(",
	"^"..formatting.input_start.."\r\n[0-9:%[%] ]*<[^> ]+> [^ ]+ banned [^ ]+ by nick %(",
	-- YHub Kick
	"^"..formatting.input_start.."\r\n[0-9:%[%] ]*<[^> ]+> [^ ]+, IP: [0-9.]+ was kicked",
	-- ADC test commands
	"^"..formatting.input_start.."\r\n[0-9:%[%] ]*<ADC>.*</ADC>$",
	-- p14nd4's join/part noise add
	"^"..formatting.input_start.."\r\n[0-9:%[%] ]*** Joins: [^ ]",
	"^"..formatting.input_start.."\r\n[0-9:%[%] ]*** Parts: [^ ]",
	-- Nick changes. For now, we assume that nicks can't contain spaces.. well, the hub's nick may can
	"^"..formatting.input_start.."\r\n[0-9:%[%] ]*<[^>]+> [^ ]+ is now known as [^ ]+$",
}

function dcpp.FormatChatText( hub, text )
	local txtcolor = formatting.color.normal
	local timecolor = formatting.color.time
	local nickcolor = formatting.color.nick

	if formatting.isNoise( text ) then
		-- escape chars
		text = string.gsub( text, "([{}\\])", "\\%1" )
		text = string.gsub( text, "\r\n", "\n" )
		text = string.gsub( text, "\n", "\\line\n" )
		txtcolor = formatting.color.noise
	elseif dcpp:hasHub( hub ) and dcpp:getHub( hub ):getOwnNick() then
		local hub = dcpp:getHub( hub )
		local mynick = hub:getOwnNick()
		if hub:getProtocol() == "nmdc" then
			mynick = DC():ToUtf8(mynick)
		end
		if formatting.testMyLine( text, mynick ) then
			txtcolor = formatting.color.self
			formatting.findChatPartner( hub, text )
		elseif formatting.testMyNick( text, mynick ) then
			txtcolor = formatting.color.myname
			timecolor = formatting.color.myname
			if formatting.flash_on_myname then
				hub:attention()
			end
		elseif formatting.testPartnerLine( text ) then
			txtcolor = formatting.color.partner
		end
		-- escape chars
		text = string.gsub( text, "([{}\\])", "\\%1" )
		text = string.gsub( text, "\r\n", "\n" )
		text = string.gsub( text, "\n", "\\line\n" )
		-- colorize OPs
		local ret,c,pre,nick,post = string.find( text,
				"^("..formatting.input_start.."\\line\n[0-9:,%a%s%[%]]*<)([^>]+)(>.*)$" )
		if ret then
			if formatting_opt.bold == 1 then
				text = string.sub( pre, 1, -2 ).."\\cf3 <\\b "..nick.."\\b0 >\\cf1 "..string.sub( post, 2 )
			else
				text = string.sub( pre, 1, -2 ).."\\cf3 <"..nick..">\\cf1 "..string.sub( post, 2 )
			end
			local haskey = false
			if hub:getProtocol() == "nmdc" then
				haskey = hub:isOp( DC():FromUtf8(nick) )
			else
				haskey = hub:isOp(hub:getSidbyNick(nick))
			end
			if haskey then
				nickcolor = formatting.color.op
			end
		end
	else
		-- escape chars
		text = string.gsub( text, "([{}\\])", "\\%1" )
		text = string.gsub( text, "\r\n", "\n" )
		text = string.gsub( text, "\n", "\\line\n" )
	end

	-- colorize timestamp
	text = string.gsub( text, "^("..formatting.input_start.."\\line\n)(%[[0-9:,%s%a]*%])", "%1\\cf4 %2\\cf1 " )
	-- em
	text = string.gsub( text, "(%s)(_%S+_)$", "%1\\ul %2\\ul0 " )
	text = string.gsub( text, "(%s)(_%S+_)(%s)", "%1\\ul %2\\ul0 %3" )

	text = string.gsub( text, "(%s)(*%S+*)$", "%1\\b %2\\b0 " )
	text = string.gsub( text, "(%s)(*%S+*)(%s)", "%1\\b %2\\b0 %3" )

	text = string.gsub( text, "(%s)(/[^/\r\n\t]+/)$", "%1\\i %2\\i0 " )
	text = string.gsub( text, "(%s)(/[^/\n\n\t]+/)(%s)", "%1\\i %2\\i0 %3" )

	-- colorize links
	text = string.gsub( text, "([ <\t\r\n])(dchub://[^ \t\\>]+)", "%1\\ul\\cf2 %2\\cf1\\ul0 " )
	text = string.gsub( text, "([ <\t\r\n])(adc://[^ \t\\>]+:[0-9]+)", "%1\\ul\\cf2 %2\\cf1\\ul0 " )
	text = string.gsub( text, "([ <\t\r\n])(http://[^ \t\\>]+)", "%1\\ul\\cf2 %2\\cf1\\ul0 " )
	text = string.gsub( text, "([ <\t\r\n])(https://[^ \t\\>]+)", "%1\\ul\\cf2 %2\\cf1\\ul0 " )
	text = string.gsub( text, "([ <\t\r\n])(www%.[^ \t\\>]+)", "%1\\ul\\cf2 %2\\cf1\\ul0 " )
	text = string.gsub( text, "([ <\t\r\n])(ftp://[^ \t\\>]+)", "%1\\ul\\cf2 %2\\cf1\\ul0 " )
	text = string.gsub( text, "([ <\t\r\n])(magnet:%?[^ \t\\>]+)", "%1\\ul\\cf2 %2\\cf1\\ul0 " )
	text = string.gsub( text, "([ <\t\r\n])(irc://[^ \t\\>]+)","%1\\ul\\cf2 %2\\cf1\\ul0 " )
	text = string.gsub( text, "([ <\t\r\n])(mailto:[^ \t\\>]+)", "%1\\ul\\cf2 %2\\cf1\\ul0 " )

	-- escape all > 127 characters
	text = string.gsub( text, ".", function( c )
		if string.byte( c ) >= 128 then
			return string.format( "\\'%02x", string.byte( c ) )
		else
			return c
		end
	end )

	return "{\\urtf1\\ul0\\b0\\i0\n"..
			"{\\colortbl ;"..txtcolor..formatting.color.link..nickcolor..timecolor.."}\n"..
			"\\cf1 "..text.."}\n"
end

-- time data
formatting.time = {}
formatting.time.partners = {}

-- color conversion types
formatting.color_conversion = {}
formatting.color_conversion.black	= {0, 0, 0}
formatting.color_conversion.white	= {255, 255, 255}
formatting.color_conversion.gray	= {127, 127, 127}
formatting.color_conversion.red		= {255, 0, 0}
formatting.color_conversion.green	= {0, 255, 0}
formatting.color_conversion.blue	= {0, 0, 255}
formatting.color_conversion.yellow	= {255, 255, 0}
formatting.color_conversion.magenta	= {255, 0, 255}
formatting.color_conversion.cyan	= {0, 255, 255}
-- dark variants
formatting.color_conversion.darkgray	= {63, 63, 63}
formatting.color_conversion.darkred		= {127, 0, 0}
formatting.color_conversion.darkgreen	= {0, 127, 0}
formatting.color_conversion.darkblue	= {0, 0, 127}
formatting.color_conversion.darkyellow	= {127, 127, 0}
formatting.color_conversion.darkmagenta	= {127, 0, 127}
formatting.color_conversion.darkcyan	= {0, 127, 127}
-- aliases
formatting.color_conversion.brown		= formatting.color_conversion.darkyellow
formatting.color_conversion.grey		= formatting.color_conversion.gray
formatting.color_conversion.darkgrey	= formatting.color_conversion.darkgray
formatting.color_conversion.lightgrey   = formatting.color_conversion.lightgray

-- convert colors..
function formatting.setColors()
	formatting.color = {}
	for k,v in pairs(formatting_opt.color) do
		if formatting.color_conversion[v] then
			local c = formatting.color_conversion[v]
			formatting.color[k] = "\\red"..c[1].."\\green"..c[2].."\\blue"..c[3]..";"
		else
			formatting.color[k] = v
		end
		-- optionally invert color
		if formatting.background_is_dark then
			local ret,c,r,g,b = string.find( formatting.color[k], "^\\red(%d+)\\green(%d+)\\blue(%d+);$" )
			if ret then
				formatting.color[k] = "\\red"..(255-tonumber(r))..
										"\\green"..(255-tonumber(g))..
										"\\blue"..(255-tonumber(b))..";"
			end
		end
	end
end

-- set "also match nicks"
function formatting.setAlsoMatchNicks()
	formatting.also_match_nicks = {}
	for k,v in pairs(formatting_opt.also_match_nicks) do
		formatting.also_match_nicks[k] = string.gsub( string.lower( v ), "([^%l%d])", "%%%1" )
	end
end

-- set other options
function formatting.setOtherOptions()
	local function test( b )
		b = string.lower( tostring( b ) )
		if b == "nil" or b == "0" or b == "off" or b == "no" then return nil
		else return 1
		end
	end
	formatting.flash_on_myname = test( formatting_opt.flash_on_myname )
	formatting.background_is_dark = test( formatting_opt.background_is_dark )
end

function formatting.setOptions()
	formatting.setDefaultOptions()
	formatting.setOtherOptions()
	formatting.setColors()
	formatting.setAlsoMatchNicks()
end

function formatting.setDefaultOptions()
	if not formatting_opt then formatting_opt = {} end
	if not formatting_opt.background_is_dark then
		local bgcolor = DC():GetSetting( "BackgroundColor" ) -- BACKGROUND_COLOR (IntSetting)
		local R = math.mod( bgcolor, 256 )
		local G = math.mod( bgcolor / 256, 256 )
		local B = math.mod( bgcolor / 256 / 256, 256 )
		if R + G + B < 384 then
			formatting_opt.background_is_dark = 1
		else
			formatting_opt.background_is_dark = 0
		end
	end
	if not formatting_opt.flash_on_myname then formatting_opt.flash_on_myname = 0 end
	if not formatting_opt.bold then formatting_opt.bold = 1 end
	if not formatting_opt.color then formatting_opt.color = {} end
	if not formatting_opt.color.normal then formatting_opt.color.normal = "black" end
	if not formatting_opt.color.self then formatting_opt.color.self = "red" end
	if not formatting_opt.color.myname then formatting_opt.color.myname = "darkgreen" end
	if not formatting_opt.color.time then formatting_opt.color.time = "darkgray" end
	if not formatting_opt.color.link then formatting_opt.color.link = "blue" end
	if not formatting_opt.color.op then formatting_opt.color.op = "blue" end
	if not formatting_opt.color.nick then formatting_opt.color.nick = "darkred" end
	if not formatting_opt.color.partner then formatting_opt.color.partner = "darkmagenta" end
	if not formatting_opt.color.noise then formatting_opt.color.noise = "darkgray" end
	if not formatting_opt.also_match_nicks then formatting_opt.also_match_nicks = {} end
end

function formatting.save()
	formatting.setOptions()
	pickle.store( formatting.settings_file, {formatting_opt = formatting_opt} )
end

function formatting.load()
	local o = io.open( formatting.settings_file, "r" )
	if o then
		dofile( formatting.settings_file )
		o:close()
	end
	formatting.setOptions()
end

--/////////////////////////////////--
--// chatline matching functions //--
--/////////////////////////////////--

function formatting.testMyLine( text, nick )
	if string.find( text, "<"..nick.."> ", 1, 1 ) then
		return 1
	end
end
function formatting.testPartnerLine( text )
	local now = os.time()
	local ret = nil
	for k,v in pairs(formatting.time.partners) do
		if string.find( text, "<"..k.."> ", 1, 1 ) then
			ret = 1
			formatting.time.partners[k] = now -- re-enforce user speaking
		else
			if now - v > 60*10 then -- expire after 10 mins
				formatting.time.partners[k] = nil
			end
		end
	end
	return ret
end
function formatting.testMyNick( text, nick )
	-- a possible bug when examining utf-8 chat?
	text = string.lower( text )

	-- test also_match_nicks first
	for k,v in pairs(formatting.also_match_nicks) do
		if string.find( text, "[^%l]"..v.."[^%l]" ) or string.find( text, "[^%l]"..v.."$" ) then
			return 1
		end
	end

	-- ok.. not found.. try our hub specific nick, if exists
	if not nick then
		return false
	end
	nick = string.lower( nick )
	local enick = nil
	local enick_notag = string.gsub( nick, "%[.*%]", "" )
	if enick_notag ~= "" then
		enick = string.gsub( enick_notag, "([^a-z0-9])", "%%%1" )
	else
		enick = string.gsub( nick, "([^a-z0-9])", "%%%1" )
	end

	if string.find( text, "[^a-z]"..enick.."[^a-z]" ) or string.find( text, "[^a-z]"..enick.."$" ) then
		return 1
	end
end
function formatting.findChatPartner( hub, text )
	local ret,c,nick = string.find( text, "> ([%[%]%a%d_-]+)" )
	if not ret then return end
	local notag = nil
	if not string.find( nick, "[%[%]]" ) then notag = 1 end
	-- iterate through hub nicks to find someone,
	-- if we didn't check user existence, we'd store a heck of a lot of words
	local list = hub:findUsers( nick, notag )
	local now = os.time()
	for k,v in pairs(list) do
		formatting.time.partners[v:getNick()] = now
	end
end
function formatting.isNoise( text )
	for k,v in pairs(formatting.noise) do
		if string.find( text, v ) then
			return 1
		end
	end
end


--//////////////////////////////--


function formatting.tokenize( str )
	local ret = {}
	string.gsub( str, "(%S+)", function( s ) table.insert( ret, s ) end )
	return ret
end

dcpp:setListener( "ownChatOut", "formatting",
	function( hub, text )
		if string.sub( text, 1, 1 ) ~= "/" then return end
		parms = formatting.tokenize( string.sub( text , 2 ) )
		if parms[1] == "help" then
			hub:addLine("(formatting.lua) /color [what] [color], /flash [on/off], /highlight [add/del] [nick], /invert [on/off], /bold [on/off]")
			return nil
		elseif parms[1] == "flash" then
			if parms[2] == "on" then
				formatting_opt.flash_on_myname = 1
				formatting.save()
				hub:addLine( "Flash on your nick enabled" )
			elseif parms[2] == "off" then
				formatting_opt.flash_on_myname = 0
				formatting.save()
				hub:addLine( "Flash on your nick disabled" )
			else
				hub:addLine( "/flash (parameters: 'on' or 'off'): "..
								"When enabled, the BCDC++ window flashes when someone calls you in main chat." )
			end
			return 1
		elseif parms[1] == "invert" then
			if parms[2] == "on" then
				formatting_opt.background_is_dark = 1
				formatting.save()
				hub:addLine( "Invert colors enabled" )
			elseif parms[2] == "off" then
				formatting_opt.background_is_dark = 0
				formatting.save()
				hub:addLine( "Invert colors disabled" )
			else
				hub:addLine( "/invert (parameters: 'on' or 'off'): "..
								"When enabled, the main chat colors are inverted, useful when there's "..
								"low contrast between the chat colors and background (e.g. if you have a "..
								"black background). "..
								"As an alternate option, you can set the colors manually as you like by "..
								"using /color, see /help." )
			end
			return 1
		elseif parms[1] == "bold" then
			if parms[2] == "on" then
				formatting_opt.bold = 1
				formatting.save()
				hub:addLine( "Bold nicks enabled" )
			elseif parms[2] == "off" then
				formatting_opt.bold = 0
				formatting.save()
				hub:addLine( "Bold nicks disabled" )
			else
				hub:addLine( "/bold (parameters: 'on' or 'off): " ..
								"When enabled, the nicks are written with bold font to the main chat." )
			end
			return 1
		elseif parms[1] == "color" then
			if parms[2] == "show" then
				-- show current settings
				local msg = "The current color set is the following:"
				for k,v in pairs(formatting_opt.color) do
					msg = msg.. "\r\n*"..k..":* "..v
				end
				hub:addLine( msg )
			elseif not parms[2] or not parms[3] then
				local msg = "/color (parameters: <what> and <color>): "..
							"Use this to change the colors used in the chat.\r\n"..
							"Available color-identifiers to set (<what>):\r\n"..
							"  normal = the normal text color\r\n"..
							"  self = lines you speak\r\n"..
							"  myname = lines where your name is used (someone calling you)\r\n"..
							"  time = the timestamp\r\n"..
							"  link = URLs (clickable links)\r\n"..
							"  op = the nickname of an operator (the one between angle brackets)\r\n"..
							"  nick = the nickname of normal people\r\n"..
							"  partner = lines someone with whom you spoke recently\r\n"..
							"  noise = kick messages\r\n"..
							"Available colors (<color>):\r\n  "
				for k,v in pairs(formatting.color_conversion) do
					msg = msg..k..", "
				end
				msg = string.sub( msg, 1, -3 )
				msg = msg.."\r\nUse \"/color show\" to get the current color set displayed."
				hub:addLine( msg )
			else
				if formatting_opt.color[parms[2]] then
					if formatting.color_conversion[parms[3]] then
						formatting_opt.color[parms[2]] = parms[3];
						formatting.save()
						hub:addLine( "/color -- set "..parms[2].." to "..parms[3] )
					else
						hub:addLine( "/color -- invalid <color>" )
					end
				else
					hub:addLine( "/color -- invalid <what>" )
				end
			end
			return 1
		elseif parms[1] == "highlight" then
			if not parms[2] or not parms[3] then
				local msg = "/highlight (parameters: 'add' or 'del' and <word>): "..
							"Adds or removes words to match for highlighting besides your own name. "..
							"E.g. use \"/highlight add operator\" to let BCDC++ highlight lines that "..
							"include the word \"operator\".\r\n"..
							"Currently matched words:\r\n  "
				for k,v in pairs(formatting_opt.also_match_nicks) do
					msg = msg..v..", "
				end
				if hub:getProtocol() == "nmdc" then
					msg = DC():FromUtf8( msg )
				end
				hub:addLine( string.sub( msg, 1, -3 ) )
			elseif parms[2] == "add" then
				for k,v in pairs(formatting_opt.also_match_nicks) do
					if string.lower( v ) == parms[3] then
						local par = parms[3]
						if hub:getProtocol() == "nmdc" then
							par = DC():FromUtf8( par )
						end
						hub:addLine( "/highlight -- "..par .." exists already" )
						return 1
					end
				end
				table.insert( formatting_opt.also_match_nicks, parms[3] )
				formatting.save()
				local par = parms[3]
				if hub:getProtocol() == "nmdc" then
					par = DC():FromUtf8( par )
				end
				hub:addLine( "Highlight, added: "..par )
			elseif parms[2] == "del" then
				local rm = nil
				for k,v in pairs(formatting_opt.also_match_nicks) do
					if v == parms[3] then
						table.remove( formatting_opt.also_match_nicks, k )
						rm = 1
					end
				end
				if rm then
					formatting.save()
					if hub:getProtocol() == "nmdc" then
						parms[3] = DC():FromUtf8( parms[3] )
					end
					hub:addLine( "Highlight, removed: "..parms[3] )
				else
					hub:addLine( "/highlight -- "..parms[3].." didn't exist" )
				end
			else
				hub:addLine( "/highlight -- "..parms[2].." is not a switch" )
			end
			return 1
		end
	end
)

-- load saved settings
formatting.load()
formatting.save() -- recreate settings file

DC():PrintDebug( "  ** Loaded formatting.lua **" )

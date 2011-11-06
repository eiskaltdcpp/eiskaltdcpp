kml_geoip = {}

function kml_geoip.find_record(ip_addr)
	local offset = 0
	local mask = 2147483648  -- 1<<31
	local depth = 31
	local record_length = 3
	local x
	repeat
		local branch_offset
		if ip_addr > mask then
			-- Take the right-hand branch
			ip_addr = ip_addr - mask
			branch_offset = record_length
		else
			-- Take the left-hand branch
			branch_offset = 0
		end

		local r_pos = record_length*2*offset + branch_offset
		local locs = kml_geoip._locations
		x = locs:byte(r_pos+3)*65536+locs:byte(r_pos+2)*256+locs:byte(r_pos+1)
		if x >= kml_geoip._db_segments then
			local netmask = 32 - depth
			return x
		end

		mask = mask / 2
		depth = depth - 1
		offset = x
	until depth == 0

	assert(false)
end

function kml_geoip.extract_record(seek_record)
	local record_length = 3

	if seek_record == kml_geoip._db_segments then
		return
	end

	-- one byte for country code; skip, because with a map it's superfluous
	-- also, requires rather large/tedious lookup table; this is the +1 at end
	-- other +1 is to adjust for Lua's 1-based position numbering
	local record_ptr = seek_record + (2*record_length-1)*kml_geoip._db_segments+1+1
	assert(record_ptr < string.len(kml_geoip._locations))

	local function strcpy()
		local null_s, null_e = string.find(kml_geoip._locations, '%z', record_ptr)
		assert(null_s)
		local str = string.sub(kml_geoip._locations, record_ptr, null_s - 1)
		record_ptr = null_e + 1
		return str
	end

	local function int32_repr(repr)
		assert(repr)
		return repr:byte(1)+repr:byte(2)*256+repr:byte(3)*65536
	end

	local region = strcpy()
	local city = strcpy()
	local postal_code = strcpy()
	local latitude = int32_repr(string.sub(kml_geoip._locations, record_ptr+0, record_ptr+2))/10000-180
	local longitude = int32_repr(string.sub(kml_geoip._locations, record_ptr+3, record_ptr+5))/10000-180
	assert(region and city and postal_code and latitude and longitude)

	return region, city, postal_code, latitude, longitude
end

function kml_geoip.ip_lookup(ip_addr)
	return kml_geoip.extract_record(kml_geoip.find_record(ip_addr))
end

function kml_geoip.read_locations(filename)
	local f = io.open(DC():GetConfigPath()..'/'..filename, 'rb')
	local blocks = f:read('*a')
	f:close()

	-- find delimiter, as measured from EOF
	local delim_pos = 2+string.len(blocks)
	for i = 1,20 do
		delim_pos = delim_pos - 4
		if string.sub(blocks, delim_pos, delim_pos+2) == '\255\255\255' then
	 break
		end
	end

	-- verify it's a city database
	local dbType = blocks:byte(delim_pos+3)
	local GEOIP_CITY_EDITION_REV0 = 6
	local GEOIP_CITY_EDITION_REV1 = 2
	assert(dbType == GEOIP_CITY_EDITION_REV0 or dbType == GEOIP_CITY_EDITION_REV1)

	local db_segments = blocks:byte(delim_pos+4)+blocks:byte(delim_pos+5)*256+blocks:byte(delim_pos+6)*65536

	return blocks, db_segments
end

-- KML writing
function kml_geoip.xml_escape(text)
	-- http://www.w3.org/TR/xml/#syntax
	local subst_table = {
		['"'] = ' &quot; ', ["'"] = ' &apos; ', ["<"] = ' &lt; ',
		[">"] = ' &gt; '  , ["&"] = ' &amp; ' }
	local s, count = string.gsub(tostring(text), "[\"'<>&]", subst_table)
	return s
end

function kml_geoip.write_placemark(hub, user, writer)
	local function int32_str(str)
		s, e, o1, o2, o3, o4 = string.find(str, '(%d+).(%d+).(%d+).(%d+)')
		return o1*16777216+o2*65536+o3*256+o4
	end

	local function write_placemark_subtag(tag, text)
		writer(string.format('<%s>%s</%s>', tag, text, tag), 2)
	end

	-- Don't pollute KMLs with users without geographical information
	if user._ip == '' then return end

	local region, city, postcode, lat, long = kml_geoip.ip_lookup(int32_str(user._ip))

	-- Know their IP, but GeoIP database knows nothing of them.
	if not region then return end

	writer('<Placemark>', 1)
	write_placemark_subtag('name', user._nick)

	-- Write as much of an address as feasible.
	local address = ''
	if postcode ~= '' then
		assert(region ~= '' and city ~= '')
		address = string.format('%s, %s %s', city, region, postcode)
	elseif city ~= '' then
		assert(region ~= '')
		address = string.format('%s %s', city, region)
	elseif region ~= '' then
		address = region
	end
	write_placemark_subtag('address', address)

	write_placemark_subtag('description', string.format('%s (%s) on %s', user._nick, user._class, hub:getHubName()))
	writer(string.format('<Point><coordinates>%f, %f</coordinates></Point>', long, lat), 2)
	writer('</Placemark>', 1)
end

function kml_geoip.write_kml(hub, filename)
	local f = io.open(filename, 'w')

	local function write(handle, text, indentation_level)
		if not indentation_level then indentation_level = 0 end
		handle:write(string.rep('\t', indentation_level)..text.."\n")
	end
	writer = function(t,l) write(f, t, l) end

	writer('<?xml version="1.0" encoding="UTF-8"?>')
	writer('<kml xmlns="http://www.opengis.net/kml/2.2">\n<Document>')
	for sid, user in pairs(hub._users) do
		kml_geoip.write_placemark(hub, user, writer)
	end
	writer('</Document>\n</kml>')

	f:close()
end

-- Initialization

-- Download http://geolite.maxmind.com/download/geoip/database/GeoLiteCity.dat.gz
-- and gunzip it into the DC++ configuration diretory.
kml_geoip._locations, kml_geoip._db_segments = kml_geoip.read_locations('GeoLiteCity.dat')

-- Listeners
dcpp:setListener( "ownChatOut", "kml_geoip_ownchat",
	function( hub, text )
		if string.sub(text, 1, 1) ~= '/' then
			return
		end
		_, _, cmd, param = string.find(text, '([^ ]*) +(.*)')

		if cmd == "/help" then
			-- logfunctions.injectSmallHelp(hub, param)
		elseif cmd == "/kml" then
			if not param then
				hub:addLine("Missing parameter. Usage: /kml filename")
			else
				kml_geoip.write_kml(hub, param)
			end
			return true
		end
	end
)

DC():PrintDebug( "  ** Loaded kml_geoip.lua **" )
-- vim:ts=4:sw=4:noet
if not pickle then -- #ifndef _INCLUDED_LIBSIMPLEPICKLE_
--/////////////////////////////////////////////////////////////////////////////
--
-- Simple Pickle module for LUA 5.1.1, version 1.0.4
--
-- Interface:
--		pickle.store( filename, data[, error_func] )
--			Loads data from disk and executes it
--		pickle.restore( filename, data[, error_func] )
--			Saves data (a table) to disk
--
-- Hidden functions:
-- 		pickle._pickle( indent, table )
-- 			Convert table to a string
-- 		pickle._execute( code )
-- 			Execute a piece of code
-- 		pickle._save( filename, data )
-- 			Save data (a string) to disk
-- 		pickle._load( filename )
-- 			Load data (full file) from disk
--
-- Copyright:
-- 		Walter Doekes <walter@djcvt.net>, 2004
--
--/////////////////////////////////////////////////////////////////////////////

pickle = {}

-- Store data
function pickle.store( filename, data, error_func )
	local d,e = pickle._pickle( 0, data )
	if d then
		d,e = pickle._save( filename, d )
	end
	if not d then
		error_func = error_func or function( s ) error( "pickle.store: failed: "..s ) end
		error_func( e )
	end
end

-- Restore data
function pickle.restore( filename, error_func )
	local d,e = pickle._load( filename )
	if d then
		d,e = pickle._execute( d )
	end
	if not d then
		error_func = error_func or function( s ) error( "pickle.restore: failed: "..s ) end
		error_func( e )
	end
end

-- Build string
function pickle._pickle( indent, theTable )
	local str = ""
	local regex = "^[a-zA-Z_][a-zA-Z0-9_]*$"
	for k,v in pairs(theTable) do
		local kq,vq
		str = str..string.rep( "\t", indent )
		kq = string.format( "%q", k ) ; kq = string.sub( kq, 2, -2 )
		if type( v ) == "table" then
			if type( k ) == "number" then
				str = str.."["..k.."] = {\r\n"..pickle._pickle( indent + 1, v )..string.rep( "\t", indent ).."}"
			elseif not string.find( k, regex ) then
				str = str.."[\""..kq.."\"] = {\r\n"..pickle._pickle( indent + 1, v )..string.rep( "\t", indent ).."}"
			else
				str = str..kq.." = {\r\n"..pickle._pickle( indent + 1, v )..string.rep( "\t", indent ).."}"
			end
		elseif type( v ) == "string" then
			vq = string.format( "%q", v )
			if type( k ) == "number" then
				str = str.."["..k.."] = "..vq
			elseif not string.find( kq, regex ) then
				str = str.."[\""..kq.."\"] = "..vq
			else
				str = str..kq.." = "..vq
			end
		elseif type( v ) == "number" then
			if type( k ) == "number" then
				str = str.."["..kq.."] = ".. pickle._tostring( v )
			elseif not string.find( kq, regex ) then
				str = str.."[\""..kq.."\"] = ".. pickle._tostring( v )
			else
				str = str..kq.." = ".. pickle._tostring( v )
			end
		elseif type( v ) == "boolean" then
			if type( k ) == "number" then
				str = str.."["..kq.."] = ".. pickle._tostring( v )
			elseif not string.find( kq, regex ) then
				str = str.."[\""..kq.."\"] = ".. pickle._tostring( v )
			else
				str = str..kq.." = ".. pickle._tostring( v )
			end
		else
			return nil,"pickle._pickle: error: valuetype "..type( v ).." is not supported!"
		end
		if indent > 0 then str = str..",\r\n" else str = str.."\r\n" end
	end
	return str
end
		
-- Execute a given string/code-snippet (a loaded settings file, perhaps)
function pickle._execute( str )
	if str then
		local f, e = loadstring( str ) 
		if f then x,e = pcall( f ) end 
		if e then return nil,"pickle._execute: "..e else return 1 end
	else
		return nil,"pickle._execute: no string supplied"
	end
end

-- Save file
function pickle._save( filename, data )
	local f,e = io.open( filename, "wb" )
	if f then
		f:write( data )
		f:close()
		return 1
	else
		return nil,"pickle._save: failed: "..e
	end
end


-- Load a file
function pickle._load( filename )
	local f,e = io.open( filename, "rb" )
	if f then
		local r = f:read( "*a" )
		f:close()
		return r
	else
		return nil,"pickle._load: failed: "..e
	end
end

-- Convert strings to numbers because of fucking locales
function pickle._tostring( num )
	return string.gsub(tostring(num), ",", ".")
end

end -- #endif //_INCLUDED_LIBSIMPLEPICKLE_

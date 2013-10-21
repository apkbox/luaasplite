-- ASP object model
asplite = asplite or {};

function asplite.ComposeQueryString(t)
	local s = '';
	local key = nil;

	while true do
		local values;
		local i;

		key, values = next(t, key);
		if not key then
			break;
		end
		
		if #s > 0 then
			s = s .. '&';
		end

		local pairs = '';

		for i = 1, #values do
			pairs = pairs .. key .. '=' .. values[i];
			if i < #values then
				pairs = pairs .. '&';
			end
		end

		s = s .. pairs;
	end

	return s;
end


function asplite.SetNewQueryStringValue(t, k, v)
	if not t[k] then
		rawset(t, k, {});
	end
	table.insert(t[k], v);
end


function asplite.InitServerVariables()
	-- request_method = 'GET'
	Request.ServerVariables['HTTP_METHOD'] = 'GET';
	Request.ServerVariables['REQUEST_METHOD'] = 'GET';

	-- uri = '/page.asp'
	Request.ServerVariables['URL'] = '/page.asp';   -- decoded
	Request.ServerVariables['SCRIPT_NAME'] = '/page.asp';

	-- http_version = '1.1'
	Request.ServerVariables['HTTP_VERSION'] = 'HTTP/1.1';    -- raw
	Request.ServerVariables['SERVER_PROTOCOL'] = 'HTTP/1.1';  -- canonicalized

	-- query_string = 'test=result'
	Request.ServerVariables['QUERY_STRING'] = 'test=result';
	
	-- is_ssl = 0;
	Request.ServerVariables['SERVER_PORT_SECURE'] = '0';

	-- remote_ip = '192.12.22.11'
	Request.ServerVariables['REMOTE_ADDR'] = '192.12.22.11';
	Request.ServerVariables['REMOTE_HOST'] = '192.12.22.11';

	-- remote_port = '21234';
	Request.ServerVariables['REMOTE_PORT'] = '21234';

	Request.ServerVariables['HTTP_HOST'] = 'www.donkey.net';
	Request.ServerVariables['HTTP_URL'] = '/page.asp?test=result';

	Request.ServerVariables['LOCAL_ADDR'] = '192.12.22.11';

	Request.ServerVariables['PATH_TRANSLATED'] = 'c:\\httpserver\\page.asp';
	Request.ServerVariables['SCRIPT_TRANSLATED'] = 'c:\\httpserver\\page.asp';

	Request.ServerVariables['SERVER_NAME'] = 'www.donkey.net';
	Request.ServerVariables['SERVER_PORT'] = '80';

	Request.ServerVariables['SERVER_SOFTWARE'] = 'mongoose/1.4';
	Request.ServerVariables['UNENCODED_URL'] = '/page.asp?test=result';
end


function asplite.Init(url)
	Server = {};
	Server.Execute = function(path) end;
	Server.HTMLEncode = function(text) end;
	Server.MapPath = function(path) end;
	Server.Transfer = function(path) end;
	Server.URLEncode = function(url) end;

	-- Create Request object structure
	Request = {};
	Request.ServerVariables = {};
	Request.QueryString = asplite.CreateQueryString(url);
	Request.Form = {};

	-- Set metatables
	setmetatable(Request.QueryString, { 
		__tostring = asplite.ComposeQueryString,
		__newindex = asplite.SetNewQueryStringValue
	});

	-- set data
	asplite.InitServerVariables();

	Response = {};
	Response.AddHeader = function(name, value) end
	Response.AppendToLog = function(text) end
	Response.Clear = function() end
	Response.End = function() end
	Response.Flush = function() end
	Response.Redirect = function(url) end
	Response.Write = function() end

	Response.Buffer = false;
	Response.ContentType = '';
	Response.Expires = '';
	Response.ExpiresAbsolute = '';
	Response.Status = 200;
end


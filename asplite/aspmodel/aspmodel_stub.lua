--[=[
	We expect Lua environment to be initialized with
		AspPage__()  ASP page script.
		asplite      table with native functions.
			context = {
				writeFunc,
				logFunc,
				request = {};
			};
			HtmlEscapeString(string) : string
			UrlDecode(string) : string
			ParseQueryString(string) : table
			(optional) HttpDate(time_value) : string
--]=]
asplite = asplite or {};

asplite.context = asplite.context or {
	writeFunc = io.write;
	logFunc = io.write;
	request = {
		['REQUEST_METHOD'] = 'GET';
		['REQUEST_URI'] = 'asptest.asp';
		['SERVER_PROTOCOL'] = 'HTTP/1.1';
		['QUERY_STRING'] = 'a=1&b=2';
		['HTTPS'] = 0;
		['REMOTE_IP'] = '127.0.0.1';
		['REMOTE_PORT'] = 80;
		['SERVER_SOFTWARE'] = 'Mongoose/3.9';
	};
};

asplite.CreateQueryString = asplite.ParseQueryString or function (qstring)
	local t = {};
	local i = 1;
	if qstring then
		while true do
			local j = string.find(qstring, '&', i, true);
			j = j or 0
			local p = string.sub(qstring, i, j - 1);
			if p then
				local name, x, value = string.match(p, '([^=]+)(=?(.*))');
				if name then
					if not t[name] then
						t[name] = {};
					end
					table.insert(t[name], asplite.UrlDecode(value));
				end
			end
			if j == 0 then
				break;
			end
			i = j + 1;
		end
	end

	return t;
end;

-- This is actually done by ASP compiler.
-- It is here for testing only
if not AspPage__ then
	local asp, err = loadfile('asptest.asp.lua');
	if not asp then
		error("ASP page load error: " .. err);
	end
	asp();
end



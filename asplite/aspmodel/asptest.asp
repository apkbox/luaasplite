<html>
<head>
	<title><%= "ASP Lite test page" %></title>
</head>
<body>
<b>This is a test</b>

<%-- This is a comment test
<table>
<% for n, v in pairs(Request.QueryString) do %>
	<tr>
		<td><%: n %></td>
		<td><% for i = 1, #v do %><%: v[i] %><br><% end %></td>
	</tr>
<% end %>
</table>
--%>


<table>
<% for n, v in pairs(Request.QueryString) do %>
	<tr>
		<td><%: n %></td>
		<td><% for i = 1, #v do %><%: v[i] %><br><% end %></td>
	</tr>
<% end %>
</table>

</body>
</html>


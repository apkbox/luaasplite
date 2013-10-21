static int luax_len(lua_State *L, int index)
{
    int len;
    lua_len(L, index);
    len = (int)lua_tonumber(L, -1);
    lua_pop(L, 1);
    return len;
}


static int asplite_ArrayToQueryStringPairs(lua_State *L, int top)
{
    const int name_index = top + 1;
    const int array_index = top + 2;
    const int pairs_index = top + 3;
    int i;
    int length = luax_len(L, array_index);

    lua_pushstring(L, "");   // pairs

    for (i = 0; i < length; i++) {
        lua_pushvalue(L, name_index);
        lua_rawgeti(L, array_index, i);

        if (!lua_isnil(L, -1)) {
            lua_pushstring(L, "=");
            lua_insert(L, -2);
            lua_concat(L, 3);  // name .. '=' .. value
        }
        else {
            lua_remove(L, -1);  // remove nil
        }

        // check if more to go
        if ((i + 1) < length) {
            lua_pushstring(L, "&");
            lua_concat(L, 3);   // pairs .. name=value .. '&'
        }
        else {
            lua_concat(L, 2);   // pairs .. name=value
        }
    }

    return 1;
}


static int ComposeQueryString(lua_State *L)
{
    int length;
    int i;

    const char *ss;

    // [query_string_table]
    lua_pushstring(L, "");

    // [query_string_table, 'query_string']
    lua_pushnil(L);

    // [query_string_table, 'query_string', nil]
    while (lua_next(L, 1) != 0) {
        // [query_string_table, 'query_string', name, value_array_or_nil]
        if (!lua_istable(L, -1)) {
            // [query_string_table, 'query_string', name, nil]
            lua_pop(L, 1);

            // [query_string_table, 'query_string', name]
            if (luax_len(L, 2) > 0) {
                lua_pushvalue(L, 2);
                // [query_string_table, 'query_string', name, 'query_string']
                lua_pushstring(L, "&");
                // [query_string_table, 'query_string', name, 'query_string', '&']
                lua_pushvalue(L, 3);
                // [query_string_table, 'query_string', name, 'query_string', '&', name]
                lua_concat(L, 3);
                // [query_string_table, 'query_string', name, 'query_string+name']
            }
            else {
                // [query_string_table, '', name]
                lua_pushvalue(L, -1);
                // [query_string_table, '', name, 'name']
            }
        }
        else {
            // Expect name and value_varray on stack.
            asplite_ArrayToQueryStringPairs(L, lua_gettop(L) - 2);

            // [query_string_table, 'query_string', name, value_array, 'name=value&name=value...']
            lua_copy(L, 2, -2);

            // [query_string_table, 'query_string', name, 'query_string', 'name=value&name=value...']
            if (luax_len(L, -2) > 0) {
                lua_pushstring(L, "&");
                // [query_string_table, 'query_string', name, 
                //  'query_string', 'name=value&name=value...', '&']
                lua_insert(L, -2);
                // [query_string_table, 'query_string', name, 
                //  'query_string', '&', 'name=value&name=value...']
                lua_concat(L, 3);
            }
            else {
                // [query_string_table, '', name, '', 'name=value&name=value...']
                lua_concat(L, 2);
            }
        }

        // [query_string_table, '', name, 'name=value&name=value...']
        lua_replace(L, 2);
    }

    // [query_string_table, 'query_string']
    return 1;
}

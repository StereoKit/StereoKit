using System.Text;
using System.Text.RegularExpressions;

namespace StereoKitAPIGen;

/// <summary>
/// Generates Lua bindings from parsed header data.
/// Outputs a Lua C module plus a small Lua convenience layer.
/// </summary>
public static class BindLua {
	static NameOverrides _overrides = new();
	static HashSet<string> _opaqueTypes = new();
	static Dictionary<string, SKModule> _modulesByName = new();
	static Dictionary<string, SKEnum> _enumsByName = new();
	static HashSet<string> _structHelpers = new();
	static readonly List<string> _unsupported = new();

	public static void Bind(SKHeaderData data, string outputFolder, NameOverrides overrides) {
		_overrides     = overrides;
		_opaqueTypes   = data.OpaqueTypes;
		_modulesByName = data.Modules.ToDictionary(m => m.Name, m => m);
		_enumsByName   = data.Enums.ToDictionary(e => e.Name, e => e);
		_structHelpers = BuildStructHelperSet(data);
		_unsupported.Clear();

		if (!Directory.Exists(outputFolder))
			Directory.CreateDirectory(outputFolder);

		File.WriteAllText(Path.Combine(outputFolder, "stereokit_lua.c"), BuildCModule(data));
		File.WriteAllText(Path.Combine(outputFolder, "stereokit.lua"), BuildLuaModule(data));
		File.WriteAllText(Path.Combine(outputFolder, "LuaUnsupported.md"), BuildUnsupportedReport());

		int supported = data.Functions.Count - _unsupported.Count;
		Console.WriteLine($"Generated Lua bindings with {supported}/{data.Functions.Count} functions, {_structHelpers.Count} structs, {data.Enums.Count} enums");
	}

	const string FileHeader = @"// This is a generated file based on stereokit.h! Please don't modify it
// directly :) Instead, modify the header file, and run the StereoKitAPIGen
// project.

";

	static string BuildCModule(SKHeaderData data) {
		var sb = new StringBuilder();
		sb.Append(FileHeader);
		sb.AppendLine("#include <lua.h>");
		sb.AppendLine("#include <lauxlib.h>");
		sb.AppendLine("#include <stdint.h>");
		sb.AppendLine("#include <stdio.h>");
		sb.AppendLine("#include <stdlib.h>");
		sb.AppendLine("#include <string.h>");
		sb.AppendLine("#include \"stereokit.h\"");
		sb.AppendLine("#include \"stereokit_ui.h\"");
		sb.AppendLine();
		sb.AppendLine("#if LUA_VERSION_NUM < 502");
		sb.AppendLine("#define luaL_newlib(L,l) (lua_newtable(L), luaL_register(L, NULL, l))");
		sb.AppendLine("#define lua_rawlen lua_objlen");
		sb.AppendLine("#endif");
		sb.AppendLine();
		sb.AppendLine("typedef struct sk_lua_asset_t { void *ptr; } sk_lua_asset_t;");
		sb.AppendLine("static lua_State *sk_lua_callback_state = NULL;");
		sb.AppendLine();
		BuildRuntimeHelpers(sb, data);

		foreach (var m in data.Modules.Where(m => _structHelpers.Contains(m.Name))) {
			BuildStructHelpers(sb, m);
		}

		var wrappers = new List<string>();
		foreach (var fn in data.Functions) {
			string? reason = UnsupportedReason(fn);
			if (reason != null) {
				if (BuildSpecialCallbackWrapper(sb, fn)) {
					wrappers.Add(fn.Name);
					continue;
				}
				_unsupported.Add($"- `{fn.Name}`: {reason}");
				continue;
			}
			string wrapperName = $"sk_lua_fn_{fn.Name}";
			wrappers.Add(fn.Name);
			BuildFunctionWrapper(sb, fn, wrapperName);
		}

		sb.AppendLine("static const luaL_Reg sk_lua_functions[] = {");
		sb.AppendLine("\t{\"__set_asset_index\", sk_lua_set_asset_index},");
		foreach (var name in wrappers)
			sb.AppendLine($"\t{{\"{name}\", sk_lua_fn_{name}}},");
		sb.AppendLine("\t{NULL, NULL}");
		sb.AppendLine("};");
		sb.AppendLine();
		sb.AppendLine("int luaopen_stereokit_lua(lua_State *L) {");
		sb.AppendLine("\tsk_lua_register_asset(L, \"StereoKit.Asset\");");
		foreach (var m in data.Modules.Where(m => m.IsAsset))
			sb.AppendLine($"\tsk_lua_register_asset(L, \"{AssetMetaName(m)}\");");
		sb.AppendLine("\tluaL_newlib(L, sk_lua_functions);");
		sb.AppendLine("\treturn 1;");
		sb.AppendLine("}");

		return sb.ToString();
	}

	static void BuildRuntimeHelpers(StringBuilder sb, SKHeaderData data) {
		sb.AppendLine("static int sk_lua_absindex(lua_State *L, int index) {");
		sb.AppendLine("\treturn index > 0 || index <= LUA_REGISTRYINDEX ? index : lua_gettop(L) + index + 1;");
		sb.AppendLine("}");
		sb.AppendLine();
		sb.AppendLine("static void sk_lua_getfield(lua_State *L, int index, const char *name) {");
		sb.AppendLine("\tlua_getfield(L, sk_lua_absindex(L, index), name);");
		sb.AppendLine("}");
		sb.AppendLine();
		sb.AppendLine("static double sk_lua_field_number(lua_State *L, int index, const char *name, double fallback) {");
		sb.AppendLine("\tsk_lua_getfield(L, index, name);");
		sb.AppendLine("\tdouble result = lua_isnil(L, -1) ? fallback : luaL_checknumber(L, -1);");
		sb.AppendLine("\tlua_pop(L, 1);");
		sb.AppendLine("\treturn result;");
		sb.AppendLine("}");
		sb.AppendLine();
		sb.AppendLine("static int64_t sk_lua_field_integer(lua_State *L, int index, const char *name, int64_t fallback) {");
		sb.AppendLine("\tsk_lua_getfield(L, index, name);");
		sb.AppendLine("\tint64_t result = lua_isnil(L, -1) ? fallback : (int64_t)luaL_checkinteger(L, -1);");
		sb.AppendLine("\tlua_pop(L, 1);");
		sb.AppendLine("\treturn result;");
		sb.AppendLine("}");
		sb.AppendLine();
		sb.AppendLine("static int sk_lua_field_bool(lua_State *L, int index, const char *name, int fallback) {");
		sb.AppendLine("\tsk_lua_getfield(L, index, name);");
		sb.AppendLine("\tint result = lua_isnil(L, -1) ? fallback : lua_toboolean(L, -1);");
		sb.AppendLine("\tlua_pop(L, 1);");
		sb.AppendLine("\treturn result;");
		sb.AppendLine("}");
		sb.AppendLine();
		sb.AppendLine("static void *sk_lua_check_asset(lua_State *L, int index, const char *meta) {");
		sb.AppendLine("\tif (lua_isnil(L, index)) return NULL;");
		sb.AppendLine("\tsk_lua_asset_t *asset = (sk_lua_asset_t *)luaL_checkudata(L, index, meta);");
		sb.AppendLine("\treturn asset->ptr;");
		sb.AppendLine("}");
		sb.AppendLine();
		sb.AppendLine("static int sk_lua_push_asset(lua_State *L, const char *meta, void *ptr) {");
		sb.AppendLine("\tif (ptr == NULL) { lua_pushnil(L); return 1; }");
		sb.AppendLine("\tsk_lua_asset_t *asset = (sk_lua_asset_t *)lua_newuserdata(L, sizeof(sk_lua_asset_t));");
		sb.AppendLine("\tasset->ptr = ptr;");
		sb.AppendLine("\tluaL_getmetatable(L, meta);");
		sb.AppendLine("\tlua_setmetatable(L, -2);");
		sb.AppendLine("\treturn 1;");
		sb.AppendLine("}");
		sb.AppendLine();
		sb.AppendLine("static void sk_lua_register_asset(lua_State *L, const char *meta) {");
		sb.AppendLine("\tif (luaL_newmetatable(L, meta)) {");
		sb.AppendLine("\t\tlua_pushvalue(L, -1);");
		sb.AppendLine("\t\tlua_setfield(L, -2, \"__index\");");
		sb.AppendLine("\t}");
		sb.AppendLine("\tlua_pop(L, 1);");
		sb.AppendLine("}");
		sb.AppendLine();
		sb.AppendLine("static int sk_lua_set_asset_index(lua_State *L) {");
		sb.AppendLine("\tconst char *meta = luaL_checkstring(L, 1);");
		sb.AppendLine("\tluaL_checktype(L, 2, LUA_TTABLE);");
		sb.AppendLine("\tluaL_getmetatable(L, meta);");
		sb.AppendLine("\tlua_pushvalue(L, 2);");
		sb.AppendLine("\tlua_setfield(L, -2, \"__index\");");
		sb.AppendLine("\tlua_pop(L, 1);");
		sb.AppendLine("\treturn 0;");
		sb.AppendLine("}");
		sb.AppendLine();
		sb.AppendLine("static void sk_lua_callback_error(lua_State *L, const char *name) {");
		sb.AppendLine("\tconst char *err = lua_tostring(L, -1);");
		sb.AppendLine("\tfprintf(stderr, \"StereoKit Lua callback %s failed: %s\\n\", name, err != NULL ? err : \"unknown error\");");
		sb.AppendLine("\tlua_pop(L, 1);");
		sb.AppendLine("}");
		sb.AppendLine();
		sb.AppendLine("static char16_t *sk_lua_check_utf16(lua_State *L, int index) {");
		sb.AppendLine("\tsize_t len;");
		sb.AppendLine("\tconst unsigned char *src = (const unsigned char *)luaL_checklstring(L, index, &len);");
		sb.AppendLine("\tchar16_t *dst = (char16_t *)calloc((len * 2) + 1, sizeof(char16_t));");
		sb.AppendLine("\tif (dst == NULL) luaL_error(L, \"out of memory\");");
		sb.AppendLine("\tsize_t out = 0;");
		sb.AppendLine("\tfor (size_t i = 0; i < len;) {");
		sb.AppendLine("\t\tuint32_t cp = src[i++];");
		sb.AppendLine("\t\tif ((cp & 0x80) == 0) {");
		sb.AppendLine("\t\t} else if ((cp & 0xE0) == 0xC0 && i < len) {");
		sb.AppendLine("\t\t\tcp = ((cp & 0x1F) << 6) | (src[i++] & 0x3F);");
		sb.AppendLine("\t\t} else if ((cp & 0xF0) == 0xE0 && i + 1 < len) {");
		sb.AppendLine("\t\t\tuint32_t b1 = src[i++] & 0x3F;");
		sb.AppendLine("\t\t\tuint32_t b2 = src[i++] & 0x3F;");
		sb.AppendLine("\t\t\tcp = ((cp & 0x0F) << 12) | (b1 << 6) | b2;");
		sb.AppendLine("\t\t} else if ((cp & 0xF8) == 0xF0 && i + 2 < len) {");
		sb.AppendLine("\t\t\tuint32_t b1 = src[i++] & 0x3F;");
		sb.AppendLine("\t\t\tuint32_t b2 = src[i++] & 0x3F;");
		sb.AppendLine("\t\t\tuint32_t b3 = src[i++] & 0x3F;");
		sb.AppendLine("\t\t\tcp = ((cp & 0x07) << 18) | (b1 << 12) | (b2 << 6) | b3;");
		sb.AppendLine("\t\t} else {");
		sb.AppendLine("\t\t\tcp = 0xFFFD;");
		sb.AppendLine("\t\t}");
		sb.AppendLine("\t\tif (cp <= 0xFFFF) {");
		sb.AppendLine("\t\t\tdst[out++] = (char16_t)cp;");
		sb.AppendLine("\t\t} else {");
		sb.AppendLine("\t\t\tcp -= 0x10000;");
		sb.AppendLine("\t\t\tdst[out++] = (char16_t)(0xD800 | (cp >> 10));");
		sb.AppendLine("\t\t\tdst[out++] = (char16_t)(0xDC00 | (cp & 0x3FF));");
		sb.AppendLine("\t\t}");
		sb.AppendLine("\t}");
		sb.AppendLine("\tdst[out] = 0;");
		sb.AppendLine("\treturn dst;");
		sb.AppendLine("}");
		sb.AppendLine();
		sb.AppendLine("static int sk_lua_push_utf16(lua_State *L, const char16_t *src) {");
		sb.AppendLine("\tif (src == NULL) { lua_pushnil(L); return 1; }");
		sb.AppendLine("\tsize_t units = 0;");
		sb.AppendLine("\twhile (src[units] != 0) units++;");
		sb.AppendLine("\tchar *dst = (char *)calloc((units * 4) + 1, sizeof(char));");
		sb.AppendLine("\tif (dst == NULL) return luaL_error(L, \"out of memory\");");
		sb.AppendLine("\tsize_t out = 0;");
		sb.AppendLine("\tfor (size_t i = 0; i < units; i++) {");
		sb.AppendLine("\t\tuint32_t cp = src[i];");
		sb.AppendLine("\t\tif (cp >= 0xD800 && cp <= 0xDBFF && i + 1 < units) {");
		sb.AppendLine("\t\t\tuint32_t low = src[i + 1];");
		sb.AppendLine("\t\t\tif (low >= 0xDC00 && low <= 0xDFFF) {");
		sb.AppendLine("\t\t\t\tcp = 0x10000 + (((cp - 0xD800) << 10) | (low - 0xDC00));");
		sb.AppendLine("\t\t\t\ti++;");
		sb.AppendLine("\t\t\t}");
		sb.AppendLine("\t\t}");
		sb.AppendLine("\t\tif (cp < 0x80) {");
		sb.AppendLine("\t\t\tdst[out++] = (char)cp;");
		sb.AppendLine("\t\t} else if (cp < 0x800) {");
		sb.AppendLine("\t\t\tdst[out++] = (char)(0xC0 | (cp >> 6));");
		sb.AppendLine("\t\t\tdst[out++] = (char)(0x80 | (cp & 0x3F));");
		sb.AppendLine("\t\t} else if (cp < 0x10000) {");
		sb.AppendLine("\t\t\tdst[out++] = (char)(0xE0 | (cp >> 12));");
		sb.AppendLine("\t\t\tdst[out++] = (char)(0x80 | ((cp >> 6) & 0x3F));");
		sb.AppendLine("\t\t\tdst[out++] = (char)(0x80 | (cp & 0x3F));");
		sb.AppendLine("\t\t} else {");
		sb.AppendLine("\t\t\tdst[out++] = (char)(0xF0 | (cp >> 18));");
		sb.AppendLine("\t\t\tdst[out++] = (char)(0x80 | ((cp >> 12) & 0x3F));");
		sb.AppendLine("\t\t\tdst[out++] = (char)(0x80 | ((cp >> 6) & 0x3F));");
		sb.AppendLine("\t\t\tdst[out++] = (char)(0x80 | (cp & 0x3F));");
		sb.AppendLine("\t\t}");
		sb.AppendLine("\t}");
		sb.AppendLine("\tlua_pushlstring(L, dst, out);");
		sb.AppendLine("\tfree(dst);");
		sb.AppendLine("\treturn 1;");
		sb.AppendLine("}");
		sb.AppendLine();
	}

	static void BuildStructHelpers(StringBuilder sb, SKModule m) {
		string cName = CName(m.Name);
		sb.AppendLine($"static {m.Name} sk_lua_check_{cName}(lua_State *L, int index) {{");
		sb.AppendLine($"\t{m.Name} value = {{0}};");
		sb.AppendLine("\tif (!lua_istable(L, index)) luaL_error(L, \"expected table for struct value\");");
		foreach (var f in m.Fields) {
			string field = LuaName(f.Name);
			if (f.Type.ArraySize1 > 0) {
				BuildReadArrayField(sb, f, field);
			} else if (_structHelpers.Contains(f.Type.Name)) {
				sb.AppendLine($"\tsk_lua_getfield(L, index, \"{field}\");");
				sb.AppendLine($"\tvalue.{f.Name} = lua_isnil(L, -1) ? ({CType(f.Type)}){{0}} : sk_lua_check_{CName(f.Type.Name)}(L, -1);");
				sb.AppendLine("\tlua_pop(L, 1);");
			} else {
				string expr = ReadFieldExpr(f.Type, "index", field);
				sb.AppendLine($"\tvalue.{f.Name} = {expr};");
			}
		}
		sb.AppendLine("\treturn value;");
		sb.AppendLine("}");
		sb.AppendLine();

		sb.AppendLine($"static int sk_lua_push_{cName}(lua_State *L, {m.Name} value) {{");
		sb.AppendLine("\tlua_newtable(L);");
		foreach (var f in m.Fields) {
			string field = LuaName(f.Name);
			if (f.Type.ArraySize1 > 0)
				PushArrayField(sb, f);
			else
				PushValue(sb, f.Type, $"value.{f.Name}");
			sb.AppendLine($"\tlua_setfield(L, -2, \"{field}\");");
		}
		sb.AppendLine("\treturn 1;");
		sb.AppendLine("}");
		sb.AppendLine();
	}

	static void BuildReadArrayField(StringBuilder sb, SKField field, string luaField) {
		var elementType = new SKType { Name = field.Type.Name, IsConst = field.Type.IsConst };
		string iName = $"i_{CName(field.Name)}";
		sb.AppendLine($"\tsk_lua_getfield(L, index, \"{luaField}\");");
		sb.AppendLine("\tif (!lua_isnil(L, -1)) {");
		sb.AppendLine($"\t\tluaL_checktype(L, -1, LUA_TTABLE);");
		sb.AppendLine($"\t\tfor (int {iName} = 0; {iName} < {field.Type.ArraySize1}; {iName}++) {{");
		sb.AppendLine($"\t\t\tlua_rawgeti(L, -1, {iName} + 1);");
		sb.AppendLine($"\t\t\tif (!lua_isnil(L, -1)) value.{field.Name}[{iName}] = {ReadStackExpr(elementType, "-1")};");
		sb.AppendLine("\t\t\tlua_pop(L, 1);");
		sb.AppendLine("\t\t}");
		sb.AppendLine("\t}");
		sb.AppendLine("\tlua_pop(L, 1);");
	}

	static void PushArrayField(StringBuilder sb, SKField field) {
		var elementType = new SKType { Name = field.Type.Name, IsConst = field.Type.IsConst };
		string iName = $"i_{CName(field.Name)}";
		sb.AppendLine("\tlua_newtable(L);");
		sb.AppendLine($"\tfor (int {iName} = 0; {iName} < {field.Type.ArraySize1}; {iName}++) {{");
		PushValue(sb, elementType, $"value.{field.Name}[{iName}]");
		sb.AppendLine($"\t\tlua_rawseti(L, -2, {iName} + 1);");
		sb.AppendLine("\t}");
	}

	static void BuildFunctionWrapper(StringBuilder sb, SKFunction fn, string wrapperName) {
		sb.AppendLine($"static int {wrapperName}(lua_State *L) {{");
		int luaArg = 1;
		var callArgs = new List<string>();
		var postReturns = new List<(SKType type, string name)>();
		var postArrayReturns = new List<(SKType type, string name, string count)>();
		var pendingRefArrayReturns = new List<(SKType type, string name, string countParam)>();
		var cleanup = new List<string>();
		string? pendingCountParam = null;
		string? pendingCountExpr = null;
		string? pendingSkippedParam = null;

		foreach (var p in fn.Parameters) {
			string local = CName(p.NameFlagless);
			if (pendingSkippedParam != null && p.NameFlagless == pendingSkippedParam) {
				pendingSkippedParam = null;
				continue;
			}
			if (pendingCountParam != null && p.NameFlagless == pendingCountParam) {
				callArgs.Add(pendingCountExpr!);
				pendingCountParam = null;
				pendingCountExpr = null;
				continue;
			}

			if (p.IsRefArr && CanMarshalRefArr(p) && TryGetFollowingCountParam(fn, p, out string refArrCountParam)) {
				var elementType = RefArrElementType(p.Type);
				sb.AppendLine($"\t{CType(elementType)} *{local} = NULL;");
				callArgs.Add($"&{local}");
				pendingRefArrayReturns.Add((elementType, local, refArrCountParam));
			} else if (CanMarshalOutputArray(fn, p)) {
				var elementType = PointeeType(p.Type);
				string countName = $"{local}_count";
				sb.AppendLine($"\tint32_t {countName} = (int32_t)luaL_checkinteger(L, {luaArg});");
				sb.AppendLine($"\t{CType(elementType)} *{local} = ({CType(elementType)} *)calloc((size_t){countName}, sizeof({CType(elementType)}));");
				sb.AppendLine($"\tif ({countName} > 0 && {local} == NULL) return luaL_error(L, \"out of memory\");");
				luaArg++;
				callArgs.Add(local);
				postArrayReturns.Add((elementType, local, countName));
				cleanup.Add($"free({local});");
				if (TryGetFollowingCountParam(fn, p, out string countParam)) {
					pendingSkippedParam = countParam;
					callArgs.Add(countName);
				}
			} else if (CanMarshalRefArray(fn, p)) {
				var elementType = PointeeType(p.Type);
				string countName = $"{local}_count";
				sb.AppendLine($"\tint32_t {countName} = (int32_t)lua_rawlen(L, {luaArg});");
				sb.AppendLine($"\t{CType(elementType)} *{local} = ({CType(elementType)} *)calloc((size_t){countName}, sizeof({CType(elementType)}));");
				sb.AppendLine($"\tif ({countName} > 0 && {local} == NULL) return luaL_error(L, \"out of memory\");");
				sb.AppendLine($"\tfor (int32_t i_{local} = 0; i_{local} < {countName}; i_{local}++) {{");
				sb.AppendLine($"\t\tlua_rawgeti(L, {luaArg}, i_{local} + 1);");
				sb.AppendLine($"\t\t{local}[i_{local}] = {ReadStackExpr(elementType, "-1")};");
				sb.AppendLine("\t\tlua_pop(L, 1);");
				sb.AppendLine("\t}");
				luaArg++;
				callArgs.Add(local);
				postArrayReturns.Add((elementType, local, countName));
				cleanup.Add($"free({local});");
				if (TryGetFollowingCountParam(fn, p, out string countParam)) {
					pendingSkippedParam = countParam;
					callArgs.Add(countName);
				}
			} else if (CanMarshalFixedArray(p)) {
				var elementType = new SKType { Name = p.Type.Name, IsConst = p.Type.IsConst };
				sb.AppendLine($"\t{CType(elementType)} {local}[{p.Type.ArraySize1}] = {{0}};");
				sb.AppendLine($"\tluaL_checktype(L, {luaArg}, LUA_TTABLE);");
				sb.AppendLine($"\tfor (int32_t i_{local} = 0; i_{local} < {p.Type.ArraySize1}; i_{local}++) {{");
				sb.AppendLine($"\t\tlua_rawgeti(L, {luaArg}, i_{local} + 1);");
				sb.AppendLine($"\t\tif (!lua_isnil(L, -1)) {local}[i_{local}] = {ReadStackExpr(elementType, "-1")};");
				sb.AppendLine("\t\tlua_pop(L, 1);");
				sb.AppendLine("\t}");
				luaArg++;
				callArgs.Add(local);
			} else if (CanMarshalInputArray(fn, p)) {
				var elementType = PointeeType(p.Type);
				string countName = $"{local}_count";
				sb.AppendLine($"\tint32_t {countName} = (int32_t)lua_rawlen(L, {luaArg});");
				sb.AppendLine($"\t{CType(elementType)} *{local} = ({CType(elementType)} *)calloc((size_t){countName}, sizeof({CType(elementType)}));");
				sb.AppendLine($"\tif ({countName} > 0 && {local} == NULL) return luaL_error(L, \"out of memory\");");
				sb.AppendLine($"\tfor (int32_t i_{local} = 0; i_{local} < {countName}; i_{local}++) {{");
				sb.AppendLine($"\t\tlua_rawgeti(L, {luaArg}, i_{local} + 1);");
				sb.AppendLine($"\t\t{local}[i_{local}] = {ReadStackExpr(elementType, "-1")};");
				sb.AppendLine("\t\tlua_pop(L, 1);");
				sb.AppendLine("\t}");
				luaArg++;
				callArgs.Add(local);
				cleanup.Add($"free({local});");
				if (TryGetFollowingCountParam(fn, p, out string countParam))
				{
					pendingCountParam = countParam;
					pendingCountExpr = countName;
				}
			} else if (IsUtf16InputString(p.Type)) {
				sb.AppendLine($"\tchar16_t *{local} = sk_lua_check_utf16(L, {luaArg});");
				luaArg++;
				callArgs.Add(local);
				cleanup.Add($"free({local});");
			} else if (IsWritableUtf16StringBuffer(p.Type)) {
				string sourceName = $"{local}_source";
				string unitsName = $"{local}_source_units";
				string capacityName = $"{local}_capacity";
				sb.AppendLine($"\tchar16_t *{sourceName} = sk_lua_check_utf16(L, {luaArg});");
				sb.AppendLine($"\tint32_t {unitsName} = 0;");
				sb.AppendLine($"\twhile ({sourceName}[{unitsName}] != 0) {unitsName}++;");
				sb.AppendLine($"\tint32_t {capacityName} = {unitsName} + 256;");
				sb.AppendLine($"\tif ({capacityName} < 1024) {capacityName} = 1024;");
				sb.AppendLine($"\tchar16_t *{local} = (char16_t *)calloc((size_t){capacityName}, sizeof(char16_t));");
				sb.AppendLine($"\tif ({local} == NULL) return luaL_error(L, \"out of memory\");");
				sb.AppendLine($"\tmemcpy({local}, {sourceName}, ((size_t){unitsName} + 1) * sizeof(char16_t));");
				luaArg++;
				callArgs.Add(local);
				postReturns.Add((new SKType { Name = "char16_t", PointerLevel = 1, IsConst = true }, local));
				cleanup.Add($"free({sourceName});");
				cleanup.Add($"free({local});");
				if (TryGetFollowingBufferSizeParam(fn, p, out string sizeParam)) {
					pendingSkippedParam = sizeParam;
					callArgs.Add(capacityName);
				}
			} else if (IsWritableStringBuffer(p.Type)) {
				string lenName = $"{local}_source_len";
				string capacityName = $"{local}_capacity";
				sb.AppendLine($"\tsize_t {lenName};");
				sb.AppendLine($"\tconst char *{local}_source = luaL_checklstring(L, {luaArg}, &{lenName});");
				sb.AppendLine($"\tint32_t {capacityName} = (int32_t){lenName} + 256;");
				sb.AppendLine($"\tif ({capacityName} < 1024) {capacityName} = 1024;");
				sb.AppendLine($"\tchar *{local} = (char *)calloc((size_t){capacityName}, sizeof(char));");
				sb.AppendLine($"\tif ({local} == NULL) return luaL_error(L, \"out of memory\");");
				sb.AppendLine($"\tmemcpy({local}, {local}_source, {lenName});");
				luaArg++;
				callArgs.Add(local);
				postReturns.Add((new SKType { Name = "char", PointerLevel = 1, IsConst = true }, local));
				cleanup.Add($"free({local});");
				if (TryGetFollowingBufferSizeParam(fn, p, out string sizeParam)) {
					pendingSkippedParam = sizeParam;
					callArgs.Add(capacityName);
				}
			} else if (p.Type.IsOptional && p.Type.PointerLevel == 1 && !IsString(p.Type) && !IsVoidPointer(p.Type)) {
				var valueType = PointeeType(p.Type);
				string ptrName = $"{local}_ptr";
				sb.AppendLine($"\t{CType(valueType)} {local} = {{0}};");
				sb.AppendLine($"\t{CType(valueType)} *{ptrName} = NULL;");
				sb.AppendLine($"\tif (!lua_isnoneornil(L, {luaArg})) {{ {local} = {ReadArgExpr(valueType, luaArg)}; {ptrName} = &{local}; }}");
				luaArg++;
				callArgs.Add(ptrName);
				if (p.PassType == SKPassType.Out || p.PassType == SKPassType.Ref)
					postReturns.Add((valueType, local));
			} else if (p.PassType == SKPassType.Out || p.PassType == SKPassType.Ref) {
				var valueType = PointeeType(p.Type);
				sb.AppendLine($"\t{CType(valueType)} {local};");
				if (p.PassType == SKPassType.Ref) {
					sb.AppendLine($"\t{local} = {ReadArgExpr(p, valueType, luaArg)};");
					luaArg++;
				}
				callArgs.Add($"&{local}");
				postReturns.Add((valueType, local));
			} else if (p.PassType == SKPassType.In && p.Type.PointerLevel == 1 && !IsString(p.Type)) {
				var valueType = PointeeType(p.Type);
				sb.AppendLine($"\t{CType(valueType)} {local} = {ReadArgExpr(p, valueType, luaArg)};");
				luaArg++;
				callArgs.Add($"&{local}");
			} else {
				sb.AppendLine($"\t{CType(p.Type)} {local} = {ReadArgExpr(p, p.Type, luaArg)};");
				luaArg++;
				callArgs.Add(local);
			}
		}

		bool hasReturn = !fn.ReturnType.IsVoid;
		if (hasReturn) {
			sb.AppendLine($"\t{CType(fn.ReturnType)} result = {fn.Name}({string.Join(", ", callArgs)});");
			PushValue(sb, fn.ReturnType, "result");
		} else {
			sb.AppendLine($"\t{fn.Name}({string.Join(", ", callArgs)});");
		}

		foreach (var (type, name) in postReturns)
			PushValue(sb, type, name);
		foreach (var (type, name, countParam) in pendingRefArrayReturns) {
			string countLocal = CName(countParam);
			PushArrayValue(sb, type, name, countLocal);
		}
		foreach (var (type, name, count) in postArrayReturns)
			PushArrayValue(sb, type, name, count);

		foreach (var line in cleanup)
			sb.AppendLine($"\t{line}");

		sb.AppendLine($"\treturn {(hasReturn ? 1 : 0) + postReturns.Count + pendingRefArrayReturns.Count + postArrayReturns.Count};");
		sb.AppendLine("}");
		sb.AppendLine();
	}

	static bool BuildSpecialCallbackWrapper(StringBuilder sb, SKFunction fn) {
		return fn.Name switch {
			"sk_step"                         => BuildSkStep(sb),
			"sk_run"                          => BuildSkRun(sb),
			"sk_run_data"                     => BuildSkRunData(sb),
			"sound_generate"                  => BuildSoundGenerate(sb),
			"log_subscribe"                   => BuildLogSubscribe(sb),
			"log_unsubscribe"                 => BuildLogUnsubscribe(sb),
			"input_subscribe"                 => BuildInputSubscribe(sb),
			"input_unsubscribe"               => BuildInputUnsubscribe(sb),
			"platform_file_picker"            => BuildPlatformFilePicker(sb, false),
			"platform_file_picker_sz"         => BuildPlatformFilePicker(sb, true),
			"render_screenshot_capture"       => BuildRenderScreenshot(sb, false),
			"render_screenshot_viewpoint"     => BuildRenderScreenshot(sb, true),
			"mesh_on_load"                    => BuildAssetOnLoad(sb, "mesh", "mesh_t", "StereoKit.Mesh"),
			"mesh_on_load_remove"             => BuildAssetOnLoadRemove(sb, "mesh", "mesh_t"),
			"tex_on_load"                     => BuildAssetOnLoad(sb, "tex", "tex_t", "StereoKit.Tex"),
			"tex_on_load_remove"              => BuildAssetOnLoadRemove(sb, "tex", "tex_t"),
			"model_on_load"                   => BuildAssetOnLoad(sb, "model", "model_t", "StereoKit.Model"),
			"model_on_load_remove"            => BuildAssetOnLoadRemove(sb, "model", "model_t"),
			"mesh_get_verts"                  => BuildMeshGetVerts(sb),
			"mesh_get_inds"                   => BuildMeshGetInds(sb),
			"tex_get_data"                    => BuildTexGetData(sb),
			"compute_buffer_get_data"         => BuildComputeBufferGetData(sb),
			"sensor_depth_try_get_latest_data" => BuildSensorDepthTryGetLatestData(sb),
			"material_get_param"              => BuildMaterialGetParam(sb, false),
			"material_get_param_id"           => BuildMaterialGetParam(sb, true),
			"backend_openxr_add_callback_pre_session_create" => BuildOpenXRPreSessionCallback(sb),
			"backend_openxr_add_callback_poll_event"         => BuildOpenXRPollEventCallback(sb),
			"backend_openxr_remove_callback_poll_event"      => BuildOpenXRRemovePollEventCallback(sb),
			_ => false,
		};
	}

	static bool IsSpecialCallbackFunction(string name) {
		return name is
			"sk_step" or
			"sk_run" or
			"sk_run_data" or
			"sound_generate" or
			"log_subscribe" or
			"log_unsubscribe" or
			"input_subscribe" or
			"input_unsubscribe" or
			"platform_file_picker" or
			"platform_file_picker_sz" or
			"render_screenshot_capture" or
			"render_screenshot_viewpoint" or
			"mesh_on_load" or
			"mesh_on_load_remove" or
			"tex_on_load" or
			"tex_on_load_remove" or
			"model_on_load" or
			"model_on_load_remove" or
			"mesh_get_verts" or
			"mesh_get_inds" or
			"tex_get_data" or
			"compute_buffer_get_data" or
			"sensor_depth_try_get_latest_data" or
			"material_get_param" or
			"material_get_param_id" or
			"backend_openxr_add_callback_pre_session_create" or
			"backend_openxr_add_callback_poll_event" or
			"backend_openxr_remove_callback_poll_event";
	}

	static bool BuildSkStep(StringBuilder sb) {
		sb.AppendLine("static int sk_lua_ref_sk_step = LUA_NOREF;");
		sb.AppendLine("static void sk_lua_cb_sk_step(void) {");
		sb.AppendLine("\tlua_State *L = sk_lua_callback_state;");
		sb.AppendLine("\tif (L == NULL || sk_lua_ref_sk_step == LUA_NOREF) return;");
		sb.AppendLine("\tlua_rawgeti(L, LUA_REGISTRYINDEX, sk_lua_ref_sk_step);");
		sb.AppendLine("\tif (lua_pcall(L, 0, 0, 0) != LUA_OK) sk_lua_callback_error(L, \"sk_step\");");
		sb.AppendLine("}");
		sb.AppendLine("static int sk_lua_fn_sk_step(lua_State *L) {");
		sb.AppendLine("\tsk_lua_callback_state = L;");
		sb.AppendLine("\tluaL_checktype(L, 1, LUA_TFUNCTION);");
		sb.AppendLine("\tlua_pushvalue(L, 1);");
		sb.AppendLine("\tluaL_unref(L, LUA_REGISTRYINDEX, sk_lua_ref_sk_step);");
		sb.AppendLine("\tsk_lua_ref_sk_step = luaL_ref(L, LUA_REGISTRYINDEX);");
		sb.AppendLine("\tlua_pushboolean(L, sk_step(sk_lua_cb_sk_step) != 0);");
		sb.AppendLine("\treturn 1;");
		sb.AppendLine("}");
		sb.AppendLine();
		return true;
	}

	static bool BuildSkRun(StringBuilder sb) {
		sb.AppendLine("static int sk_lua_ref_sk_run_step = LUA_NOREF;");
		sb.AppendLine("static int sk_lua_ref_sk_run_shutdown = LUA_NOREF;");
		sb.AppendLine("static void sk_lua_cb_sk_run_step(void) {");
		sb.AppendLine("\tlua_State *L = sk_lua_callback_state;");
		sb.AppendLine("\tif (L == NULL || sk_lua_ref_sk_run_step == LUA_NOREF) return;");
		sb.AppendLine("\tlua_rawgeti(L, LUA_REGISTRYINDEX, sk_lua_ref_sk_run_step);");
		sb.AppendLine("\tif (lua_pcall(L, 0, 0, 0) != LUA_OK) sk_lua_callback_error(L, \"sk_run step\");");
		sb.AppendLine("}");
		sb.AppendLine("static void sk_lua_cb_sk_run_shutdown(void) {");
		sb.AppendLine("\tlua_State *L = sk_lua_callback_state;");
		sb.AppendLine("\tif (L == NULL || sk_lua_ref_sk_run_shutdown == LUA_NOREF) return;");
		sb.AppendLine("\tlua_rawgeti(L, LUA_REGISTRYINDEX, sk_lua_ref_sk_run_shutdown);");
		sb.AppendLine("\tif (lua_pcall(L, 0, 0, 0) != LUA_OK) sk_lua_callback_error(L, \"sk_run shutdown\");");
		sb.AppendLine("}");
		sb.AppendLine("static int sk_lua_fn_sk_run(lua_State *L) {");
		sb.AppendLine("\tsk_lua_callback_state = L;");
		sb.AppendLine("\tluaL_checktype(L, 1, LUA_TFUNCTION);");
		sb.AppendLine("\tlua_pushvalue(L, 1); luaL_unref(L, LUA_REGISTRYINDEX, sk_lua_ref_sk_run_step); sk_lua_ref_sk_run_step = luaL_ref(L, LUA_REGISTRYINDEX);");
		sb.AppendLine("\tvoid (*shutdown)(void) = NULL;");
		sb.AppendLine("\tif (!lua_isnoneornil(L, 2)) { luaL_checktype(L, 2, LUA_TFUNCTION); lua_pushvalue(L, 2); luaL_unref(L, LUA_REGISTRYINDEX, sk_lua_ref_sk_run_shutdown); sk_lua_ref_sk_run_shutdown = luaL_ref(L, LUA_REGISTRYINDEX); shutdown = sk_lua_cb_sk_run_shutdown; }");
		sb.AppendLine("\tsk_run(sk_lua_cb_sk_run_step, shutdown);");
		sb.AppendLine("\treturn 0;");
		sb.AppendLine("}");
		sb.AppendLine();
		return true;
	}

	static bool BuildSkRunData(StringBuilder sb) {
		sb.AppendLine("static int sk_lua_ref_sk_run_data_step = LUA_NOREF;");
		sb.AppendLine("static int sk_lua_ref_sk_run_data_step_data = LUA_NOREF;");
		sb.AppendLine("static int sk_lua_ref_sk_run_data_shutdown = LUA_NOREF;");
		sb.AppendLine("static int sk_lua_ref_sk_run_data_shutdown_data = LUA_NOREF;");
		sb.AppendLine("static void sk_lua_cb_sk_run_data_step(void *step_data) {");
		sb.AppendLine("\tlua_State *L = sk_lua_callback_state;");
		sb.AppendLine("\tif (L == NULL || sk_lua_ref_sk_run_data_step == LUA_NOREF) return;");
		sb.AppendLine("\tlua_rawgeti(L, LUA_REGISTRYINDEX, sk_lua_ref_sk_run_data_step);");
		sb.AppendLine("\tint argc = 0;");
		sb.AppendLine("\tif (sk_lua_ref_sk_run_data_step_data != LUA_NOREF) { lua_rawgeti(L, LUA_REGISTRYINDEX, sk_lua_ref_sk_run_data_step_data); argc = 1; }");
		sb.AppendLine("\tif (lua_pcall(L, argc, 0, 0) != LUA_OK) sk_lua_callback_error(L, \"sk_run_data step\");");
		sb.AppendLine("}");
		sb.AppendLine("static void sk_lua_cb_sk_run_data_shutdown(void *shutdown_data) {");
		sb.AppendLine("\tlua_State *L = sk_lua_callback_state;");
		sb.AppendLine("\tif (L == NULL || sk_lua_ref_sk_run_data_shutdown == LUA_NOREF) return;");
		sb.AppendLine("\tlua_rawgeti(L, LUA_REGISTRYINDEX, sk_lua_ref_sk_run_data_shutdown);");
		sb.AppendLine("\tint argc = 0;");
		sb.AppendLine("\tif (sk_lua_ref_sk_run_data_shutdown_data != LUA_NOREF) { lua_rawgeti(L, LUA_REGISTRYINDEX, sk_lua_ref_sk_run_data_shutdown_data); argc = 1; }");
		sb.AppendLine("\tif (lua_pcall(L, argc, 0, 0) != LUA_OK) sk_lua_callback_error(L, \"sk_run_data shutdown\");");
		sb.AppendLine("}");
		sb.AppendLine("static int sk_lua_fn_sk_run_data(lua_State *L) {");
		sb.AppendLine("\tsk_lua_callback_state = L;");
		sb.AppendLine("\tluaL_checktype(L, 1, LUA_TFUNCTION);");
		sb.AppendLine("\tlua_pushvalue(L, 1); luaL_unref(L, LUA_REGISTRYINDEX, sk_lua_ref_sk_run_data_step); sk_lua_ref_sk_run_data_step = luaL_ref(L, LUA_REGISTRYINDEX);");
		sb.AppendLine("\tluaL_unref(L, LUA_REGISTRYINDEX, sk_lua_ref_sk_run_data_step_data); sk_lua_ref_sk_run_data_step_data = LUA_NOREF;");
		sb.AppendLine("\tif (!lua_isnoneornil(L, 2)) { lua_pushvalue(L, 2); sk_lua_ref_sk_run_data_step_data = luaL_ref(L, LUA_REGISTRYINDEX); }");
		sb.AppendLine("\tvoid (*shutdown)(void *) = NULL;");
		sb.AppendLine("\tluaL_unref(L, LUA_REGISTRYINDEX, sk_lua_ref_sk_run_data_shutdown); sk_lua_ref_sk_run_data_shutdown = LUA_NOREF;");
		sb.AppendLine("\tluaL_unref(L, LUA_REGISTRYINDEX, sk_lua_ref_sk_run_data_shutdown_data); sk_lua_ref_sk_run_data_shutdown_data = LUA_NOREF;");
		sb.AppendLine("\tif (!lua_isnoneornil(L, 3)) { luaL_checktype(L, 3, LUA_TFUNCTION); lua_pushvalue(L, 3); sk_lua_ref_sk_run_data_shutdown = luaL_ref(L, LUA_REGISTRYINDEX); shutdown = sk_lua_cb_sk_run_data_shutdown; }");
		sb.AppendLine("\tif (!lua_isnoneornil(L, 4)) { lua_pushvalue(L, 4); sk_lua_ref_sk_run_data_shutdown_data = luaL_ref(L, LUA_REGISTRYINDEX); }");
		sb.AppendLine("\tsk_run_data(sk_lua_cb_sk_run_data_step, NULL, shutdown, NULL);");
		sb.AppendLine("\treturn 0;");
		sb.AppendLine("}");
		sb.AppendLine();
		return true;
	}

	static bool BuildSoundGenerate(StringBuilder sb) {
		sb.AppendLine("static int sk_lua_ref_sound_generate = LUA_NOREF;");
		sb.AppendLine("static float sk_lua_cb_sound_generate(float sample_time) {");
		sb.AppendLine("\tlua_State *L = sk_lua_callback_state;");
		sb.AppendLine("\tif (L == NULL || sk_lua_ref_sound_generate == LUA_NOREF) return 0;");
		sb.AppendLine("\tlua_rawgeti(L, LUA_REGISTRYINDEX, sk_lua_ref_sound_generate);");
		sb.AppendLine("\tlua_pushnumber(L, sample_time);");
		sb.AppendLine("\tif (lua_pcall(L, 1, 1, 0) != LUA_OK) { sk_lua_callback_error(L, \"sound_generate\"); return 0; }");
		sb.AppendLine("\tfloat result = (float)luaL_optnumber(L, -1, 0);");
		sb.AppendLine("\tlua_pop(L, 1);");
		sb.AppendLine("\treturn result;");
		sb.AppendLine("}");
		sb.AppendLine("static int sk_lua_fn_sound_generate(lua_State *L) {");
		sb.AppendLine("\tsk_lua_callback_state = L;");
		sb.AppendLine("\tluaL_checktype(L, 1, LUA_TFUNCTION);");
		sb.AppendLine("\tlua_pushvalue(L, 1); luaL_unref(L, LUA_REGISTRYINDEX, sk_lua_ref_sound_generate); sk_lua_ref_sound_generate = luaL_ref(L, LUA_REGISTRYINDEX);");
		sb.AppendLine("\tfloat duration = (float)luaL_checknumber(L, 2);");
		sb.AppendLine("\tsound_t result = sound_generate(sk_lua_cb_sound_generate, duration);");
		sb.AppendLine("\tsk_lua_push_asset(L, \"StereoKit.Sound\", (void *)result);");
		sb.AppendLine("\treturn 1;");
		sb.AppendLine("}");
		sb.AppendLine();
		return true;
	}

	static bool BuildLogSubscribe(StringBuilder sb) {
		sb.AppendLine("static int sk_lua_ref_log = LUA_NOREF;");
		sb.AppendLine("static void sk_lua_cb_log(void *context, log_ level, const char *text) {");
		sb.AppendLine("\tlua_State *L = sk_lua_callback_state;");
		sb.AppendLine("\tif (L == NULL || sk_lua_ref_log == LUA_NOREF) return;");
		sb.AppendLine("\tlua_rawgeti(L, LUA_REGISTRYINDEX, sk_lua_ref_log);");
		sb.AppendLine("\tlua_pushinteger(L, level); lua_pushstring(L, text);");
		sb.AppendLine("\tif (lua_pcall(L, 2, 0, 0) != LUA_OK) sk_lua_callback_error(L, \"log_subscribe\");");
		sb.AppendLine("}");
		sb.AppendLine("static int sk_lua_fn_log_subscribe(lua_State *L) {");
		sb.AppendLine("\tsk_lua_callback_state = L; luaL_checktype(L, 1, LUA_TFUNCTION); lua_pushvalue(L, 1); luaL_unref(L, LUA_REGISTRYINDEX, sk_lua_ref_log); sk_lua_ref_log = luaL_ref(L, LUA_REGISTRYINDEX); log_subscribe(sk_lua_cb_log, NULL); return 0;");
		sb.AppendLine("}");
		sb.AppendLine();
		return true;
	}

	static bool BuildLogUnsubscribe(StringBuilder sb) {
		sb.AppendLine("static int sk_lua_fn_log_unsubscribe(lua_State *L) {");
		sb.AppendLine("\tlog_unsubscribe(sk_lua_cb_log, NULL); luaL_unref(L, LUA_REGISTRYINDEX, sk_lua_ref_log); sk_lua_ref_log = LUA_NOREF; return 0;");
		sb.AppendLine("}");
		sb.AppendLine();
		return true;
	}

	static bool BuildInputSubscribe(StringBuilder sb) {
		sb.AppendLine("static int sk_lua_ref_input = LUA_NOREF;");
		sb.AppendLine("static void sk_lua_cb_input(input_source_ source, button_state_ input_event, const pointer_t *in_pointer) {");
		sb.AppendLine("\tlua_State *L = sk_lua_callback_state;");
		sb.AppendLine("\tif (L == NULL || sk_lua_ref_input == LUA_NOREF) return;");
		sb.AppendLine("\tlua_rawgeti(L, LUA_REGISTRYINDEX, sk_lua_ref_input);");
		sb.AppendLine("\tlua_pushinteger(L, source); lua_pushinteger(L, input_event);");
		sb.AppendLine("\tif (in_pointer == NULL) lua_pushnil(L); else sk_lua_push_pointer_t(L, *in_pointer);");
		sb.AppendLine("\tif (lua_pcall(L, 3, 0, 0) != LUA_OK) sk_lua_callback_error(L, \"input_subscribe\");");
		sb.AppendLine("}");
		sb.AppendLine("static int sk_lua_fn_input_subscribe(lua_State *L) {");
		sb.AppendLine("\tsk_lua_callback_state = L;");
		sb.AppendLine("\tinput_source_ source = (input_source_)luaL_checkinteger(L, 1);");
		sb.AppendLine("\tbutton_state_ input_event = (button_state_)luaL_checkinteger(L, 2);");
		sb.AppendLine("\tluaL_checktype(L, 3, LUA_TFUNCTION);");
		sb.AppendLine("\tlua_pushvalue(L, 3); luaL_unref(L, LUA_REGISTRYINDEX, sk_lua_ref_input); sk_lua_ref_input = luaL_ref(L, LUA_REGISTRYINDEX);");
		sb.AppendLine("\tinput_subscribe(source, input_event, sk_lua_cb_input);");
		sb.AppendLine("\treturn 0;");
		sb.AppendLine("}");
		sb.AppendLine();
		return true;
	}

	static bool BuildInputUnsubscribe(StringBuilder sb) {
		sb.AppendLine("static int sk_lua_fn_input_unsubscribe(lua_State *L) {");
		sb.AppendLine("\tinput_source_ source = (input_source_)luaL_checkinteger(L, 1);");
		sb.AppendLine("\tbutton_state_ input_event = (button_state_)luaL_checkinteger(L, 2);");
		sb.AppendLine("\tinput_unsubscribe(source, input_event, sk_lua_cb_input);");
		sb.AppendLine("\tluaL_unref(L, LUA_REGISTRYINDEX, sk_lua_ref_input); sk_lua_ref_input = LUA_NOREF;");
		sb.AppendLine("\treturn 0;");
		sb.AppendLine("}");
		sb.AppendLine();
		return true;
	}

	static bool BuildPlatformFilePicker(StringBuilder sb, bool sized) {
		string suffix = sized ? "sz" : "utf8";
		string fnName = sized ? "platform_file_picker_sz" : "platform_file_picker";
		string cbArgs = sized ? "void *callback_data, bool32_t confirmed, const char *filename_ptr, int32_t filename_length" : "void *callback_data, bool32_t confirmed, const char *filename";
		sb.AppendLine($"typedef struct sk_lua_file_picker_{suffix}_context_t {{ int callback_ref; }} sk_lua_file_picker_{suffix}_context_t;");
		sb.AppendLine($"static void sk_lua_cb_file_picker_{suffix}({cbArgs}) {{");
		sb.AppendLine($"\tsk_lua_file_picker_{suffix}_context_t *ctx = (sk_lua_file_picker_{suffix}_context_t *)callback_data;");
		sb.AppendLine("\tlua_State *L = sk_lua_callback_state;");
		sb.AppendLine("\tif (L == NULL || ctx == NULL) { free(ctx); return; }");
		sb.AppendLine("\tlua_rawgeti(L, LUA_REGISTRYINDEX, ctx->callback_ref);");
		sb.AppendLine("\tlua_pushboolean(L, confirmed != 0);");
		if (sized)
			sb.AppendLine("\tif (filename_ptr == NULL) lua_pushnil(L); else lua_pushlstring(L, filename_ptr, (size_t)filename_length);");
		else
			sb.AppendLine("\tlua_pushstring(L, filename);");
		sb.AppendLine($"\tif (lua_pcall(L, 2, 0, 0) != LUA_OK) sk_lua_callback_error(L, \"{fnName}\");");
		sb.AppendLine("\tluaL_unref(L, LUA_REGISTRYINDEX, ctx->callback_ref);");
		sb.AppendLine("\tfree(ctx);");
		sb.AppendLine("}");
		sb.AppendLine($"static int sk_lua_fn_{fnName}(lua_State *L) {{");
		sb.AppendLine("\tsk_lua_callback_state = L;");
		sb.AppendLine("\tpicker_mode_ mode = (picker_mode_)luaL_checkinteger(L, 1);");
		sb.AppendLine("\tluaL_checktype(L, 2, LUA_TFUNCTION);");
		sb.AppendLine($"\tsk_lua_file_picker_{suffix}_context_t *ctx = (sk_lua_file_picker_{suffix}_context_t *)calloc(1, sizeof(sk_lua_file_picker_{suffix}_context_t));");
		sb.AppendLine("\tif (ctx == NULL) return luaL_error(L, \"out of memory\");");
		sb.AppendLine("\tlua_pushvalue(L, 2); ctx->callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);");
		sb.AppendLine("\tfile_filter_t *filters = NULL;");
		sb.AppendLine("\tint32_t filter_count = 0;");
		sb.AppendLine("\tif (!lua_isnoneornil(L, 3)) {");
		sb.AppendLine("\t\tluaL_checktype(L, 3, LUA_TTABLE);");
		sb.AppendLine("\t\tfilter_count = (int32_t)lua_rawlen(L, 3);");
		sb.AppendLine("\t\tfilters = (file_filter_t *)calloc((size_t)filter_count, sizeof(file_filter_t));");
		sb.AppendLine("\t\tif (filter_count > 0 && filters == NULL) { luaL_unref(L, LUA_REGISTRYINDEX, ctx->callback_ref); free(ctx); return luaL_error(L, \"out of memory\"); }");
		sb.AppendLine("\t\tfor (int32_t i = 0; i < filter_count; i++) {");
		sb.AppendLine("\t\t\tlua_rawgeti(L, 3, i + 1);");
		sb.AppendLine("\t\t\tconst char *ext = NULL;");
		sb.AppendLine("\t\t\tif (lua_istable(L, -1)) { lua_getfield(L, -1, \"ext\"); ext = luaL_checkstring(L, -1); strncpy(filters[i].ext, ext, sizeof(filters[i].ext) - 1); lua_pop(L, 1); }");
		sb.AppendLine("\t\t\telse { ext = luaL_checkstring(L, -1); strncpy(filters[i].ext, ext, sizeof(filters[i].ext) - 1); }");
		sb.AppendLine("\t\t\tlua_pop(L, 1);");
		sb.AppendLine("\t\t}");
		sb.AppendLine("\t}");
		sb.AppendLine($"\t{fnName}(mode, ctx, sk_lua_cb_file_picker_{suffix}, filters, filter_count);");
		sb.AppendLine("\tfree(filters);");
		sb.AppendLine("\treturn 0;");
		sb.AppendLine("}");
		sb.AppendLine();
		return true;
	}

	static bool BuildRenderScreenshot(StringBuilder sb, bool viewpoint) {
		string suffix = viewpoint ? "viewpoint" : "capture";
		string fnName = viewpoint ? "render_screenshot_viewpoint" : "render_screenshot_capture";
		sb.AppendLine($"typedef struct sk_lua_screenshot_{suffix}_context_t {{ int callback_ref; }} sk_lua_screenshot_{suffix}_context_t;");
		sb.AppendLine($"static void sk_lua_cb_screenshot_{suffix}(color32 *color_buffer, int32_t width, int32_t height, void *context) {{");
		sb.AppendLine($"\tsk_lua_screenshot_{suffix}_context_t *ctx = (sk_lua_screenshot_{suffix}_context_t *)context;");
		sb.AppendLine("\tlua_State *L = sk_lua_callback_state;");
		sb.AppendLine("\tif (L == NULL || ctx == NULL) { free(ctx); return; }");
		sb.AppendLine("\tlua_rawgeti(L, LUA_REGISTRYINDEX, ctx->callback_ref);");
		sb.AppendLine("\tlua_pushlightuserdata(L, color_buffer); lua_pushinteger(L, width); lua_pushinteger(L, height);");
		sb.AppendLine($"\tif (lua_pcall(L, 3, 0, 0) != LUA_OK) sk_lua_callback_error(L, \"{fnName}\");");
		sb.AppendLine("\tluaL_unref(L, LUA_REGISTRYINDEX, ctx->callback_ref);");
		sb.AppendLine("\tfree(ctx);");
		sb.AppendLine("}");
		sb.AppendLine($"static int sk_lua_fn_{fnName}(lua_State *L) {{");
		sb.AppendLine("\tsk_lua_callback_state = L;");
		sb.AppendLine($"\tsk_lua_screenshot_{suffix}_context_t *ctx = (sk_lua_screenshot_{suffix}_context_t *)calloc(1, sizeof(sk_lua_screenshot_{suffix}_context_t));");
		sb.AppendLine("\tif (ctx == NULL) return luaL_error(L, \"out of memory\");");
		sb.AppendLine("\tluaL_checktype(L, 1, LUA_TFUNCTION); lua_pushvalue(L, 1); ctx->callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);");
		if (viewpoint) {
			sb.AppendLine("\tmatrix camera = sk_lua_check_matrix(L, 2);");
			sb.AppendLine("\tmatrix projection = sk_lua_check_matrix(L, 3);");
			sb.AppendLine("\tint32_t width = (int32_t)luaL_checkinteger(L, 4);");
			sb.AppendLine("\tint32_t height = (int32_t)luaL_checkinteger(L, 5);");
			sb.AppendLine("\trender_layer_ layer_filter = (render_layer_)luaL_optinteger(L, 6, render_layer_all);");
			sb.AppendLine("\trender_clear_ clear = (render_clear_)luaL_optinteger(L, 7, render_clear_all);");
			sb.AppendLine("\trect_t viewport = lua_isnoneornil(L, 8) ? (rect_t){0} : sk_lua_check_rect_t(L, 8);");
			sb.AppendLine("\ttex_format_ tex_format = (tex_format_)luaL_optinteger(L, 9, tex_format_rgba32);");
			sb.AppendLine("\trender_screenshot_viewpoint(sk_lua_cb_screenshot_viewpoint, camera, projection, width, height, layer_filter, clear, viewport, tex_format, ctx);");
		} else {
			sb.AppendLine("\tpose_t viewpoint = sk_lua_check_pose_t(L, 2);");
			sb.AppendLine("\tint32_t width = (int32_t)luaL_checkinteger(L, 3);");
			sb.AppendLine("\tint32_t height = (int32_t)luaL_checkinteger(L, 4);");
			sb.AppendLine("\tfloat field_of_view_degrees = (float)luaL_checknumber(L, 5);");
			sb.AppendLine("\ttex_format_ tex_format = (tex_format_)luaL_optinteger(L, 6, tex_format_rgba32);");
			sb.AppendLine("\trender_screenshot_capture(sk_lua_cb_screenshot_capture, viewpoint, width, height, field_of_view_degrees, tex_format, ctx);");
		}
		sb.AppendLine("\treturn 0;");
		sb.AppendLine("}");
		sb.AppendLine();
		return true;
	}

	static bool BuildAssetOnLoad(StringBuilder sb, string prefix, string cType, string meta) {
		sb.AppendLine($"typedef struct sk_lua_{prefix}_on_load_context_t {{ int callback_ref; }} sk_lua_{prefix}_on_load_context_t;");
		sb.AppendLine($"static void sk_lua_cb_{prefix}_on_load({cType} asset, void *context) {{");
		sb.AppendLine($"\tsk_lua_{prefix}_on_load_context_t *ctx = (sk_lua_{prefix}_on_load_context_t *)context;");
		sb.AppendLine("\tlua_State *L = sk_lua_callback_state;");
		sb.AppendLine("\tif (L == NULL || ctx == NULL) { free(ctx); return; }");
		sb.AppendLine("\tlua_rawgeti(L, LUA_REGISTRYINDEX, ctx->callback_ref);");
		sb.AppendLine($"\tsk_lua_push_asset(L, \"{meta}\", (void *)asset);");
		sb.AppendLine($"\tif (lua_pcall(L, 1, 0, 0) != LUA_OK) sk_lua_callback_error(L, \"{prefix}_on_load\");");
		sb.AppendLine("\tluaL_unref(L, LUA_REGISTRYINDEX, ctx->callback_ref);");
		sb.AppendLine("\tfree(ctx);");
		sb.AppendLine("}");
		sb.AppendLine($"static int sk_lua_fn_{prefix}_on_load(lua_State *L) {{");
		sb.AppendLine("\tsk_lua_callback_state = L;");
		sb.AppendLine($"\t{cType} asset = ({cType})sk_lua_check_asset(L, 1, \"{meta}\");");
		sb.AppendLine("\tluaL_checktype(L, 2, LUA_TFUNCTION);");
		sb.AppendLine($"\tsk_lua_{prefix}_on_load_context_t *ctx = (sk_lua_{prefix}_on_load_context_t *)calloc(1, sizeof(sk_lua_{prefix}_on_load_context_t));");
		sb.AppendLine("\tif (ctx == NULL) return luaL_error(L, \"out of memory\");");
		sb.AppendLine("\tlua_pushvalue(L, 2); ctx->callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);");
		sb.AppendLine($"\t{prefix}_on_load(asset, sk_lua_cb_{prefix}_on_load, ctx);");
		sb.AppendLine("\treturn 0;");
		sb.AppendLine("}");
		sb.AppendLine();
		return true;
	}

	static bool BuildAssetOnLoadRemove(StringBuilder sb, string prefix, string cType) {
		string meta = prefix switch {
			"mesh" => "StereoKit.Mesh",
			"tex" => "StereoKit.Tex",
			"model" => "StereoKit.Model",
			_ => "StereoKit.Asset",
		};
		sb.AppendLine($"static int sk_lua_fn_{prefix}_on_load_remove(lua_State *L) {{");
		sb.AppendLine($"\t{cType} asset = ({cType})sk_lua_check_asset(L, 1, \"{meta}\");");
		sb.AppendLine($"\t{prefix}_on_load_remove(asset, sk_lua_cb_{prefix}_on_load);");
		sb.AppendLine("\treturn 0;");
		sb.AppendLine("}");
		sb.AppendLine();
		return true;
	}

	static bool BuildOpenXRPreSessionCallback(StringBuilder sb) {
		sb.AppendLine("static int sk_lua_ref_openxr_pre_session = LUA_NOREF;");
		sb.AppendLine("static void sk_lua_cb_openxr_pre_session(void *context) {");
		sb.AppendLine("\tlua_State *L = sk_lua_callback_state;");
		sb.AppendLine("\tif (L == NULL || sk_lua_ref_openxr_pre_session == LUA_NOREF) return;");
		sb.AppendLine("\tlua_rawgeti(L, LUA_REGISTRYINDEX, sk_lua_ref_openxr_pre_session);");
		sb.AppendLine("\tif (lua_pcall(L, 0, 0, 0) != LUA_OK) sk_lua_callback_error(L, \"backend_openxr_add_callback_pre_session_create\");");
		sb.AppendLine("}");
		sb.AppendLine("static int sk_lua_fn_backend_openxr_add_callback_pre_session_create(lua_State *L) {");
		sb.AppendLine("\tsk_lua_callback_state = L; luaL_checktype(L, 1, LUA_TFUNCTION);");
		sb.AppendLine("\tlua_pushvalue(L, 1); luaL_unref(L, LUA_REGISTRYINDEX, sk_lua_ref_openxr_pre_session); sk_lua_ref_openxr_pre_session = luaL_ref(L, LUA_REGISTRYINDEX);");
		sb.AppendLine("\tbackend_openxr_add_callback_pre_session_create(sk_lua_cb_openxr_pre_session, NULL);");
		sb.AppendLine("\treturn 0;");
		sb.AppendLine("}");
		sb.AppendLine();
		return true;
	}

	static bool BuildOpenXRPollEventCallback(StringBuilder sb) {
		sb.AppendLine("static int sk_lua_ref_openxr_poll_event = LUA_NOREF;");
		sb.AppendLine("static void sk_lua_cb_openxr_poll_event(void *context, void *XrEventDataBuffer) {");
		sb.AppendLine("\tlua_State *L = sk_lua_callback_state;");
		sb.AppendLine("\tif (L == NULL || sk_lua_ref_openxr_poll_event == LUA_NOREF) return;");
		sb.AppendLine("\tlua_rawgeti(L, LUA_REGISTRYINDEX, sk_lua_ref_openxr_poll_event);");
		sb.AppendLine("\tlua_pushlightuserdata(L, XrEventDataBuffer);");
		sb.AppendLine("\tif (lua_pcall(L, 1, 0, 0) != LUA_OK) sk_lua_callback_error(L, \"backend_openxr_add_callback_poll_event\");");
		sb.AppendLine("}");
		sb.AppendLine("static int sk_lua_fn_backend_openxr_add_callback_poll_event(lua_State *L) {");
		sb.AppendLine("\tsk_lua_callback_state = L; luaL_checktype(L, 1, LUA_TFUNCTION);");
		sb.AppendLine("\tlua_pushvalue(L, 1); luaL_unref(L, LUA_REGISTRYINDEX, sk_lua_ref_openxr_poll_event); sk_lua_ref_openxr_poll_event = luaL_ref(L, LUA_REGISTRYINDEX);");
		sb.AppendLine("\tbackend_openxr_add_callback_poll_event(sk_lua_cb_openxr_poll_event, NULL);");
		sb.AppendLine("\treturn 0;");
		sb.AppendLine("}");
		sb.AppendLine();
		return true;
	}

	static bool BuildOpenXRRemovePollEventCallback(StringBuilder sb) {
		sb.AppendLine("static int sk_lua_fn_backend_openxr_remove_callback_poll_event(lua_State *L) {");
		sb.AppendLine("\tbackend_openxr_remove_callback_poll_event(sk_lua_cb_openxr_poll_event);");
		sb.AppendLine("\tluaL_unref(L, LUA_REGISTRYINDEX, sk_lua_ref_openxr_poll_event); sk_lua_ref_openxr_poll_event = LUA_NOREF;");
		sb.AppendLine("\treturn 0;");
		sb.AppendLine("}");
		sb.AppendLine();
		return true;
	}

	static bool BuildMeshGetVerts(StringBuilder sb) {
		sb.AppendLine("static int sk_lua_fn_mesh_get_verts(lua_State *L) {");
		sb.AppendLine("\tmesh_t mesh = (mesh_t)sk_lua_check_asset(L, 1, \"StereoKit.Mesh\");");
		sb.AppendLine("\tmemory_ reference_mode = (memory_)luaL_optinteger(L, 2, memory_copy);");
		sb.AppendLine("\tvert_t *vertices = NULL;");
		sb.AppendLine("\tint32_t vertex_count = 0;");
		sb.AppendLine("\tmesh_get_verts(mesh, &vertices, &vertex_count, reference_mode);");
		sb.AppendLine("\tlua_newtable(L);");
		sb.AppendLine("\tfor (int32_t i_vertices = 0; i_vertices < vertex_count; i_vertices++) {");
		sb.AppendLine("\t\tsk_lua_push_vert_t(L, vertices[i_vertices]);");
		sb.AppendLine("\t\tlua_rawseti(L, -2, i_vertices + 1);");
		sb.AppendLine("\t}");
		sb.AppendLine("\tif (reference_mode == memory_copy) free(vertices);");
		sb.AppendLine("\tlua_pushinteger(L, vertex_count);");
		sb.AppendLine("\treturn 2;");
		sb.AppendLine("}");
		sb.AppendLine();
		return true;
	}

	static bool BuildMeshGetInds(StringBuilder sb) {
		sb.AppendLine("static int sk_lua_fn_mesh_get_inds(lua_State *L) {");
		sb.AppendLine("\tmesh_t mesh = (mesh_t)sk_lua_check_asset(L, 1, \"StereoKit.Mesh\");");
		sb.AppendLine("\tmemory_ reference_mode = (memory_)luaL_optinteger(L, 2, memory_copy);");
		sb.AppendLine("\tvind_t *indices = NULL;");
		sb.AppendLine("\tint32_t index_count = 0;");
		sb.AppendLine("\tmesh_get_inds(mesh, &indices, &index_count, reference_mode);");
		sb.AppendLine("\tlua_newtable(L);");
		sb.AppendLine("\tfor (int32_t i_indices = 0; i_indices < index_count; i_indices++) {");
		sb.AppendLine("\t\tlua_pushinteger(L, (lua_Integer)indices[i_indices]);");
		sb.AppendLine("\t\tlua_rawseti(L, -2, i_indices + 1);");
		sb.AppendLine("\t}");
		sb.AppendLine("\tif (reference_mode == memory_copy) free(indices);");
		sb.AppendLine("\tlua_pushinteger(L, index_count);");
		sb.AppendLine("\treturn 2;");
		sb.AppendLine("}");
		sb.AppendLine();
		return true;
	}

	static bool BuildTexGetData(StringBuilder sb) {
		sb.AppendLine("static int sk_lua_fn_tex_get_data(lua_State *L) {");
		sb.AppendLine("\ttex_t texture = (tex_t)sk_lua_check_asset(L, 1, \"StereoKit.Tex\");");
		sb.AppendLine("\tsize_t data_size = (size_t)luaL_checkinteger(L, 2);");
		sb.AppendLine("\tint32_t mip_level = (int32_t)luaL_optinteger(L, 3, 0);");
		sb.AppendLine("\tchar *data = (char *)malloc(data_size);");
		sb.AppendLine("\tif (data_size > 0 && data == NULL) return luaL_error(L, \"out of memory\");");
		sb.AppendLine("\ttex_get_data(texture, data, data_size, mip_level);");
		sb.AppendLine("\tlua_pushlstring(L, data, data_size);");
		sb.AppendLine("\tfree(data);");
		sb.AppendLine("\treturn 1;");
		sb.AppendLine("}");
		sb.AppendLine();
		return true;
	}

	static bool BuildComputeBufferGetData(StringBuilder sb) {
		sb.AppendLine("static int sk_lua_fn_compute_buffer_get_data(lua_State *L) {");
		sb.AppendLine("\tcompute_buffer_t buffer = (compute_buffer_t)sk_lua_check_asset(L, 1, \"StereoKit.ComputeBuffer\");");
		sb.AppendLine("\tint32_t element_count = (int32_t)luaL_optinteger(L, 2, compute_buffer_get_count(buffer));");
		sb.AppendLine("\tint32_t stride = compute_buffer_get_stride(buffer);");
		sb.AppendLine("\tsize_t data_size = (size_t)element_count * (size_t)stride;");
		sb.AppendLine("\tchar *data = (char *)malloc(data_size);");
		sb.AppendLine("\tif (data_size > 0 && data == NULL) return luaL_error(L, \"out of memory\");");
		sb.AppendLine("\tcompute_buffer_get_data(buffer, data, element_count);");
		sb.AppendLine("\tlua_pushlstring(L, data, data_size);");
		sb.AppendLine("\tfree(data);");
		sb.AppendLine("\treturn 1;");
		sb.AppendLine("}");
		sb.AppendLine();
		return true;
	}

	static bool BuildSensorDepthTryGetLatestData(StringBuilder sb) {
		sb.AppendLine("static int sk_lua_fn_sensor_depth_try_get_latest_data(lua_State *L) {");
		sb.AppendLine("\tint32_t view_index = (int32_t)luaL_optinteger(L, 1, -1);");
		sb.AppendLine("\tsensor_depth_frame_t frame = {0};");
		sb.AppendLine("\tsize_t data_size = 0;");
		sb.AppendLine("\tbool32_t available = sensor_depth_try_get_latest_data(&frame, NULL, &data_size, view_index);");
		sb.AppendLine("\tif (!available) { lua_pushboolean(L, 0); sk_lua_push_sensor_depth_frame_t(L, frame); lua_pushnil(L); return 3; }");
		sb.AppendLine("\tchar *data = (char *)malloc(data_size);");
		sb.AppendLine("\tif (data_size > 0 && data == NULL) return luaL_error(L, \"out of memory\");");
		sb.AppendLine("\tavailable = sensor_depth_try_get_latest_data(&frame, data, &data_size, view_index);");
		sb.AppendLine("\tlua_pushboolean(L, available != 0);");
		sb.AppendLine("\tsk_lua_push_sensor_depth_frame_t(L, frame);");
		sb.AppendLine("\tif (available) lua_pushlstring(L, data, data_size); else lua_pushnil(L);");
		sb.AppendLine("\tfree(data);");
		sb.AppendLine("\treturn 3;");
		sb.AppendLine("}");
		sb.AppendLine();
		return true;
	}

	static bool BuildMaterialGetParam(StringBuilder sb, bool byId) {
		string fnName = byId ? "material_get_param_id" : "material_get_param";
		string keyType = byId ? "id_hash_t" : "const char *";
		string keyRead = byId ? "(id_hash_t)luaL_checkinteger(L, 2)" : "luaL_checkstring(L, 2)";
		sb.AppendLine($"static int sk_lua_fn_{fnName}(lua_State *L) {{");
		sb.AppendLine("\tmaterial_t material = (material_t)sk_lua_check_asset(L, 1, \"StereoKit.Material\");");
		sb.AppendLine($"\t{keyType} key = {keyRead};");
		sb.AppendLine("\tmaterial_param_ type = (material_param_)luaL_checkinteger(L, 3);");
		sb.AppendLine("\tbool32_t ok = 0;");
		sb.AppendLine("\tswitch (type) {");
		AppendMaterialCase(sb, byId, "material_param_float", "float value = 0", "value", "lua_pushnumber(L, (lua_Number)value);");
		AppendMaterialCase(sb, byId, "material_param_color128", "color128 value = {0}", "value", "sk_lua_push_color128(L, value);");
		AppendMaterialCase(sb, byId, "material_param_vector2", "vec2 value = {0}", "value", "sk_lua_push_vec2(L, value);");
		AppendMaterialCase(sb, byId, "material_param_vector3", "vec3 value = {0}", "value", "sk_lua_push_vec3(L, value);");
		AppendMaterialCase(sb, byId, "material_param_vector4", "vec4 value = {0}", "value", "sk_lua_push_vec4(L, value);");
		AppendMaterialCase(sb, byId, "material_param_matrix", "matrix value = {0}", "value", "sk_lua_push_matrix(L, value);");
		AppendMaterialCase(sb, byId, "material_param_texture", "tex_t value = NULL", "value", "sk_lua_push_asset(L, \"StereoKit.Tex\", (void *)value);");
		AppendMaterialCase(sb, byId, "material_param_int", "int32_t value = 0", "value", "lua_pushinteger(L, (lua_Integer)value);");
		AppendMaterialCase(sb, byId, "material_param_uint", "uint32_t value = 0", "value", "lua_pushinteger(L, (lua_Integer)value);");
		AppendMaterialVectorCase(sb, byId, "material_param_int2", "int32_t", 2);
		AppendMaterialVectorCase(sb, byId, "material_param_int3", "int32_t", 3);
		AppendMaterialVectorCase(sb, byId, "material_param_int4", "int32_t", 4);
		AppendMaterialVectorCase(sb, byId, "material_param_uint2", "uint32_t", 2);
		AppendMaterialVectorCase(sb, byId, "material_param_uint3", "uint32_t", 3);
		AppendMaterialVectorCase(sb, byId, "material_param_uint4", "uint32_t", 4);
		AppendMaterialCase(sb, byId, "material_param_buffer", "material_buffer_t value = NULL", "value", "sk_lua_push_asset(L, \"StereoKit.MaterialBuffer\", (void *)value);");
		sb.AppendLine("\tdefault: lua_pushboolean(L, 0); lua_pushnil(L); return 2;");
		sb.AppendLine("\t}");
		sb.AppendLine("}");
		sb.AppendLine();
		return true;
	}

	static void AppendMaterialCase(StringBuilder sb, bool byId, string enumName, string declaration, string valueName, string pushLine) {
		string fnName = byId ? "material_get_param_id" : "material_get_param";
		sb.AppendLine($"\tcase {enumName}: {{");
		sb.AppendLine($"\t\t{declaration};");
		sb.AppendLine($"\t\tok = {fnName}(material, key, type, &{valueName});");
		sb.AppendLine("\t\tlua_pushboolean(L, ok != 0);");
		sb.AppendLine($"\t\tif (ok) {pushLine} else lua_pushnil(L);");
		sb.AppendLine("\t\treturn 2;");
		sb.AppendLine("\t}");
	}

	static void AppendMaterialVectorCase(StringBuilder sb, bool byId, string enumName, string cType, int count) {
		string fnName = byId ? "material_get_param_id" : "material_get_param";
		sb.AppendLine($"\tcase {enumName}: {{");
		sb.AppendLine($"\t\t{cType} value[{count}] = {{0}};");
		sb.AppendLine($"\t\tok = {fnName}(material, key, type, value);");
		sb.AppendLine("\t\tlua_pushboolean(L, ok != 0);");
		sb.AppendLine("\t\tif (ok) {");
		sb.AppendLine("\t\t\tlua_newtable(L);");
		sb.AppendLine($"\t\t\tfor (int32_t i_value = 0; i_value < {count}; i_value++) {{ lua_pushinteger(L, (lua_Integer)value[i_value]); lua_rawseti(L, -2, i_value + 1); }}");
		sb.AppendLine("\t\t} else lua_pushnil(L);");
		sb.AppendLine("\t\treturn 2;");
		sb.AppendLine("\t}");
	}


	static string BuildLuaModule(SKHeaderData data) {
		var sb = new StringBuilder();
		sb.AppendLine("-- This is a generated file based on stereokit.h! Please don't modify it");
		sb.AppendLine("-- directly. Modify the header file, and run StereoKitAPIGen.");
		sb.AppendLine();
		sb.AppendLine("local raw = require('stereokit_lua')");
		sb.AppendLine("local sk = { raw = raw }");
		sb.AppendLine();

		foreach (var e in data.Enums) {
			string enumName = LuaTypeName(e.Name);
			sb.AppendLine($"sk.{enumName} = {{");
			foreach (var item in e.Items) {
				string name = LuaEnumItemName(item.Name, e.Name);
				long? numeric = item.ValueNumeric ?? InferEnumValue(e, item);
				string value = numeric?.ToString() ?? "nil";
				if (value == "nil" && item.ValueExpr != null)
					value = BuildLuaEnumExpr(item.ValueExpr, e.Name);
				sb.AppendLine($"\t{name} = {value},");
			}
			sb.AppendLine("}");
			sb.AppendLine();
		}

		foreach (var m in data.Modules.Where(m => _structHelpers.Contains(m.Name))) {
			string typeName = LuaTypeName(m.Name);
			sb.AppendLine($"sk.{typeName} = setmetatable({{}}, {{ __call = function(_, value) return value or {{}} end }})");
			sb.AppendLine();
		}

		foreach (var group in data.Functions.Where(f => UnsupportedReason(f) == null || IsSpecialCallbackFunction(f.Name)).GroupBy(f => f.Module?.Name ?? GetModuleFromName(f.Name))) {
			string moduleName = LuaTypeName(group.Key);
			sb.AppendLine($"sk.{moduleName} = sk.{moduleName} or {{}}");
			foreach (var fn in group) {
				string friendly = LuaFunctionName(fn, group.Key);
				if (fn.Relation == SKFunctionRelation.Instance && fn.Module != null) {
					sb.AppendLine($"function sk.{moduleName}:{friendly}(...) return raw.{fn.Name}(self, ...) end");
				} else if (fn.Relation == SKFunctionRelation.Creation && fn.Module != null) {
					sb.AppendLine($"function sk.{moduleName}.{friendly}(...) return raw.{fn.Name}(...) end");
				} else {
					sb.AppendLine($"sk.{moduleName}.{friendly} = raw.{fn.Name}");
				}
			}
			sb.AppendLine();
		}

		sb.AppendLine("if raw.__set_asset_index then");
		sb.AppendLine("\tif sk.Asset then raw.__set_asset_index('StereoKit.Asset', sk.Asset) end");
		foreach (var m in data.Modules.Where(m => m.IsAsset)) {
			string moduleName = LuaTypeName(m.Name);
			sb.AppendLine($"\tif sk.{moduleName} then raw.__set_asset_index('{AssetMetaName(m)}', sk.{moduleName}) end");
		}
		sb.AppendLine("end");
		sb.AppendLine();

		sb.AppendLine("return sk");
		return sb.ToString();
	}

	static string BuildUnsupportedReport() {
		var sb = new StringBuilder();
		sb.AppendLine("# Lua Unsupported Signatures");
		sb.AppendLine();
		if (_unsupported.Count == 0) {
			sb.AppendLine("None.");
			return sb.ToString();
		}
		sb.AppendLine("These StereoKit C APIs need hand-written Lua policy or additional generator support.");
		sb.AppendLine();
		foreach (var line in _unsupported.OrderBy(l => l))
			sb.AppendLine(line);
		return sb.ToString();
	}

	static string? UnsupportedReason(SKFunction fn) {
		if (fn.Parameters.Any(p => p.Type.IsCallback))
			return "callback/function pointer parameters need Lua registry lifetime management";
		if (fn.Parameters.Any(p => (p.IsArray || p.Type.ArraySize1 > 0) && !CanMarshalInputArray(fn, p) && !CanMarshalOutputArray(fn, p) && !CanMarshalRefArray(fn, p) && !CanMarshalFixedArray(p)) || fn.Parameters.Any(p => p.IsRefArr && !CanMarshalRefArr(p)))
			return "array parameters need table-to-buffer marshaling policy";
		if (fn.Parameters.Any(p => p.Type.Name == "char16_t" && !IsUtf16InputString(p.Type) && !IsWritableUtf16StringBuffer(p.Type)))
			return "UTF-16 strings need conversion support";
		if (fn.ReturnType.IsCallback)
			return "callback return type is unsupported";
		if (fn.ReturnType.PointerLevel > 0 && !IsString(fn.ReturnType) && !IsVoidPointer(fn.ReturnType) && !IsStructPointer(fn.ReturnType))
			return "pointer return type needs API-specific ownership policy";
		if (!CanMarshalType(fn.ReturnType, isReturn: true))
			return $"unsupported return type `{fn.ReturnType}`";
		string? skippedParam = null;
		foreach (var p in fn.Parameters) {
			if (skippedParam != null && p.NameFlagless == skippedParam) {
				skippedParam = null;
				continue;
			}
			var type = p.PassType == SKPassType.Out || p.PassType == SKPassType.Ref || (p.PassType == SKPassType.In && p.Type.PointerLevel == 1 && !IsString(p.Type))
				? PointeeType(p.Type)
				: p.Type;
			if (p.Type.IsOptional && p.Type.PointerLevel == 1 && !IsString(p.Type) && !IsVoidPointer(p.Type))
				type = PointeeType(p.Type);
			if (CanMarshalInputArray(fn, p)) {
				type = PointeeType(p.Type);
				if (TryGetFollowingCountParam(fn, p, out string countParam))
					skippedParam = countParam;
			}
			if (CanMarshalOutputArray(fn, p) || CanMarshalRefArray(fn, p)) {
				type = PointeeType(p.Type);
				if (TryGetFollowingCountParam(fn, p, out string countParam))
					skippedParam = countParam;
			}
			if (p.IsRefArr && CanMarshalRefArr(p))
				type = RefArrElementType(p.Type);
			if (CanMarshalFixedArray(p))
				type = new SKType { Name = p.Type.Name, IsConst = p.Type.IsConst };
			if (IsWritableUtf16StringBuffer(p.Type))
				type = new SKType { Name = "char16_t", PointerLevel = 1, IsConst = true };
			if (!CanMarshalType(type, isReturn: false))
				return $"unsupported parameter `{p.Name}` type `{p.Type}`";
		}
		return null;
	}

	static bool CanMarshalInputArray(SKFunction fn, SKParameter p) {
		bool hasArrayShape = p.IsArray || TryGetFollowingCountParam(fn, p, out _) || (p.Type.Name == "char" && p.Type.PointerLevel == 2);
		if (!hasArrayShape || p.PassType == SKPassType.Out || p.PassType == SKPassType.Ref) return false;
		if (p.Type.Name == "void" || p.Type.Name == "char16_t") return false;
		if (p.Type.Name == "char" && p.Type.PointerLevel == 2) return true;
		if (p.Type.PointerLevel != 1) return false;
		var elementType = PointeeType(p.Type);
		return CanMarshalType(elementType, isReturn: false);
	}

	static bool CanMarshalOutputArray(SKFunction fn, SKParameter p) {
		if (!p.IsArray || p.PassType != SKPassType.Out || p.Type.PointerLevel != 1) return false;
		if (!TryGetFollowingCountParam(fn, p, out _)) return false;
		var elementType = PointeeType(p.Type);
		return CanMarshalType(elementType, isReturn: false);
	}

	static bool CanMarshalRefArray(SKFunction fn, SKParameter p) {
		if (!p.IsArray || p.PassType != SKPassType.Ref || p.Type.PointerLevel != 1) return false;
		if (!TryGetFollowingCountParam(fn, p, out _)) return false;
		var elementType = PointeeType(p.Type);
		return CanMarshalType(elementType, isReturn: false);
	}

	static bool CanMarshalRefArr(SKParameter p) {
		if (!p.IsRefArr || p.Type.PointerLevel < 2) return false;
		var elementType = RefArrElementType(p.Type);
		return CanMarshalType(elementType, isReturn: false);
	}

	static bool CanMarshalFixedArray(SKParameter p) {
		if (p.Type.ArraySize1 <= 0 || p.PassType == SKPassType.Out || p.PassType == SKPassType.Ref) return false;
		var elementType = new SKType { Name = p.Type.Name, IsConst = p.Type.IsConst };
		return CanMarshalType(elementType, isReturn: false);
	}

	static bool TryGetFollowingCountParam(SKFunction fn, SKParameter arrayParam, out string countParam) {
		countParam = "";
		int index = fn.Parameters.IndexOf(arrayParam);
		if (index < 0 || index + 1 >= fn.Parameters.Count) return false;

		var next = fn.Parameters[index + 1];
		if (!IsInteger(next.Type.Name)) return false;
		string name = next.NameFlagless.ToLowerInvariant();
		if (name == "count" || name == "num" || name.EndsWith("count") || name.EndsWith("_count") || name.Contains("count") || name.EndsWith("num")) {
			countParam = next.NameFlagless;
			return true;
		}
		return false;
	}

	static bool TryGetFollowingBufferSizeParam(SKFunction fn, SKParameter bufferParam, out string sizeParam) {
		sizeParam = "";
		int index = fn.Parameters.IndexOf(bufferParam);
		if (index < 0 || index + 1 >= fn.Parameters.Count) return false;

		var next = fn.Parameters[index + 1];
		if (!IsInteger(next.Type.Name)) return false;
		string name = next.NameFlagless.ToLowerInvariant();
		if (name == "size" || name.EndsWith("size") || name.Contains("buffer")) {
			sizeParam = next.NameFlagless;
			return true;
		}
		return false;
	}

	static HashSet<string> BuildStructHelperSet(SKHeaderData data) {
		var result = new HashSet<string>();
		bool changed;
		do {
			changed = false;
			foreach (var m in data.Modules.Where(m => m.IsStruct || m.IsUnion)) {
				if (result.Contains(m.Name)) continue;
				if (CanGenerateStruct(m, result)) {
					result.Add(m.Name);
					changed = true;
				}
			}
		} while (changed);
		return result;
	}

	static bool CanGenerateStruct(SKModule m, HashSet<string>? knownStructs = null) {
		foreach (var f in m.Fields) {
			if (f.Type.PointerLevel > 0 || f.Type.FunctionPtr != null)
				return false;
			var fieldType = f.Type.ArraySize1 > 0
				? new SKType { Name = f.Type.Name, PointerLevel = f.Type.PointerLevel, IsConst = f.Type.IsConst }
				: f.Type;
			if (!CanMarshalType(fieldType, isReturn: false, knownStructs))
				return false;
		}
		return true;
	}

	static bool CanMarshalType(SKType type, bool isReturn, HashSet<string>? knownStructs = null) {
		if (type.IsVoid) return isReturn;
		if (IsVoidPointer(type)) return true;
		if (IsString(type)) return true;
		if (IsUtf16InputString(type)) return true;
		if (IsStructPointer(type)) return true;
		if (type.PointerLevel > 0) return false;
		if (IsPrimitive(type.Name) || type.Name == "bool32_t") return true;
		if (_enumsByName.ContainsKey(type.Name)) return true;
		if (_opaqueTypes.Contains(type.Name)) return true;
		if ((knownStructs ?? _structHelpers).Contains(type.Name)) return true;
		if (type.Name == "asset_t") return true;
		return false;
	}

	static bool IsPrimitive(string name) => name switch {
		"int8_t" or "int16_t" or "int32_t" or "int64_t" or
		"uint8_t" or "uint16_t" or "uint32_t" or "uint64_t" or
		"int" or "float" or "double" or "size_t" or "id_hash_t" or "vind_t" or
		"text_style_t" or "model_node_id" or "interactor_t" or "hand_sim_id_t" or
		"char32_t" or "openxr_handle_t" => true,
		_ => false,
	};

	static bool IsInteger(string name) => name switch {
		"int" or "int8_t" or "int16_t" or "int32_t" or "int64_t" or
		"uint8_t" or "uint16_t" or "uint32_t" or "uint64_t" or
		"size_t" or "id_hash_t" or "vind_t" or "text_style_t" or
		"model_node_id" or "interactor_t" or "hand_sim_id_t" or "char32_t" or
		"openxr_handle_t" => true,
		_ => false,
	};

	static bool IsNumber(string name) => name == "float" || name == "double";
	static bool IsString(SKType type) => type.Name == "char" && type.PointerLevel == 1;
	static bool IsWritableStringBuffer(SKType type) => type.Name == "char" && type.PointerLevel == 1 && !type.IsConst;
	static bool IsUtf16InputString(SKType type) => type.Name == "char16_t" && type.PointerLevel == 1 && type.IsConst;
	static bool IsWritableUtf16StringBuffer(SKType type) => type.Name == "char16_t" && type.PointerLevel == 1 && !type.IsConst;
	static bool IsVoidPointer(SKType type) => type.Name == "void" && type.PointerLevel > 0;
	static bool IsStructPointer(SKType type) => type.PointerLevel == 1 && _structHelpers.Contains(type.Name);

	static SKType PointeeType(SKType type) => new() {
		Name = type.Name,
		PointerLevel = Math.Max(0, type.PointerLevel - 1),
		IsConst = type.IsConst,
	};

	static SKType RefArrElementType(SKType type) => new() {
		Name = type.Name,
		PointerLevel = Math.Max(0, type.PointerLevel - 2),
		IsConst = type.IsConst,
	};

	static string CType(SKType type) {
		if (IsString(type)) return type.IsConst ? "const char *" : "char *";
		string result = type.IsConst && type.PointerLevel > 0 ? $"const {type.Name}" : type.Name;
		for (int i = 0; i < type.PointerLevel; i++)
			result += "*";
		return result;
	}

	static string ReadArgExpr(SKType type, int luaArg) {
		if (type.Name == "bool32_t") return $"(bool32_t)lua_toboolean(L, {luaArg})";
		if (IsInteger(type.Name) || _enumsByName.ContainsKey(type.Name)) return $"({CType(type)})luaL_checkinteger(L, {luaArg})";
		if (IsNumber(type.Name)) return $"({CType(type)})luaL_checknumber(L, {luaArg})";
		if (IsString(type)) return $"luaL_checkstring(L, {luaArg})";
		if (IsVoidPointer(type)) return $"({CType(type)})lua_touserdata(L, {luaArg})";
		if (_opaqueTypes.Contains(type.Name)) return $"({CType(type)})sk_lua_check_asset(L, {luaArg}, \"{AssetMetaName(type.Name)}\")";
		if (type.Name == "asset_t") return $"({CType(type)})sk_lua_check_asset(L, {luaArg}, \"StereoKit.Asset\")";
		if (_structHelpers.Contains(type.Name)) return $"sk_lua_check_{CName(type.Name)}(L, {luaArg})";
		return $"({CType(type)})0";
	}

	static string ReadArgExpr(SKParameter p, SKType type, int luaArg) {
		string read = ReadArgExpr(type, luaArg);
		if (p.DefaultValue == null || !TryBuildDefaultExpr(type, p.DefaultValue, out string? defaultExpr))
			return read;
		return $"lua_isnoneornil(L, {luaArg}) ? {defaultExpr} : {read}";
	}

	static string ReadStackExpr(SKType type, string luaIndex) {
		if (type.Name == "bool32_t") return $"(bool32_t)lua_toboolean(L, {luaIndex})";
		if (IsInteger(type.Name) || _enumsByName.ContainsKey(type.Name)) return $"({CType(type)})luaL_checkinteger(L, {luaIndex})";
		if (IsNumber(type.Name)) return $"({CType(type)})luaL_checknumber(L, {luaIndex})";
		if (IsString(type)) return $"luaL_checkstring(L, {luaIndex})";
		if (IsVoidPointer(type)) return $"({CType(type)})lua_touserdata(L, {luaIndex})";
		if (_opaqueTypes.Contains(type.Name)) return $"({CType(type)})sk_lua_check_asset(L, {luaIndex}, \"{AssetMetaName(type.Name)}\")";
		if (type.Name == "asset_t") return $"({CType(type)})sk_lua_check_asset(L, {luaIndex}, \"StereoKit.Asset\")";
		if (_structHelpers.Contains(type.Name)) return $"sk_lua_check_{CName(type.Name)}(L, {luaIndex})";
		return $"({CType(type)})0";
	}

	static string ReadFieldExpr(SKType type, string index, string field) {
		if (type.Name == "bool32_t") return $"(bool32_t)sk_lua_field_bool(L, {index}, \"{field}\", 0)";
		if (IsInteger(type.Name) || _enumsByName.ContainsKey(type.Name)) return $"({CType(type)})sk_lua_field_integer(L, {index}, \"{field}\", 0)";
		if (IsNumber(type.Name)) return $"({CType(type)})sk_lua_field_number(L, {index}, \"{field}\", 0)";
		if (_structHelpers.Contains(type.Name)) {
			string tmp = $"sk_lua_check_{CName(type.Name)}";
			return $"(sk_lua_getfield(L, {index}, \"{field}\"), lua_isnil(L, -1) ? ({CType(type)}){{0}} : {tmp}(L, -1))";
		}
		return $"({CType(type)}){{0}}";
	}

	static bool TryBuildDefaultExpr(SKType type, string defaultValue, out string? expr) {
		expr = null;
		string value = defaultValue.Trim();
		if (value.Length == 0) return false;

		if (value == "nullptr" || value == "NULL") {
			if (type.PointerLevel > 0 || _opaqueTypes.Contains(type.Name) || type.Name == "asset_t") {
				expr = $"({CType(type)})NULL";
				return true;
			}
			return false;
		}

		if (type.Name == "bool32_t") {
			if (value == "true" || value == "false") {
				expr = value == "true" ? "(bool32_t)1" : "(bool32_t)0";
				return true;
			}
			if (IsNumericLiteral(value)) {
				expr = $"(bool32_t){value}";
				return true;
			}
			return false;
		}

		if (IsString(type)) {
			if (IsCStringLiteral(value)) {
				expr = value;
				return true;
			}
			return false;
		}

		if (_structHelpers.Contains(type.Name)) {
			expr = BuildStructDefaultExpr(type, value);
			return expr != null;
		}

		if (IsInteger(type.Name) || IsNumber(type.Name) || _enumsByName.ContainsKey(type.Name)) {
			expr = $"({CType(type)}){value}";
			return true;
		}

		if (_opaqueTypes.Contains(type.Name) || type.Name == "asset_t" || IsVoidPointer(type)) {
			expr = $"({CType(type)}){value}";
			return true;
		}

		return false;
	}

	static string? BuildStructDefaultExpr(SKType type, string value) {
		if (value == "{}") return $"({CType(type)}){{0}}";
		if (value.StartsWith("{") && value.EndsWith("}"))
			return $"({CType(type)}){value}";

		string typeName = type.Name;
		if (!value.StartsWith(typeName, StringComparison.Ordinal))
			return null;

		string initializer = value[typeName.Length..].Trim();
		if (initializer == "{}") return $"({CType(type)}){{0}}";
		if (initializer.StartsWith("{") && initializer.EndsWith("}"))
			return $"({CType(type)}){initializer}";
		return null;
	}

	static bool IsCStringLiteral(string value) {
		return value.Length >= 2 && value[0] == '"' && value[^1] == '"';
	}

	static bool IsNumericLiteral(string value) {
		return Regex.IsMatch(value, @"^-?(?:0x[0-9a-fA-F]+|\d+(?:\.\d+)?)(?:[fFuUlL]*)$");
	}

	static void PushValue(StringBuilder sb, SKType type, string expr) {
		if (type.Name == "bool32_t") {
			sb.AppendLine($"\tlua_pushboolean(L, {expr} != 0);");
		} else if (IsInteger(type.Name) || _enumsByName.ContainsKey(type.Name)) {
			sb.AppendLine($"\tlua_pushinteger(L, (lua_Integer){expr});");
		} else if (IsNumber(type.Name)) {
			sb.AppendLine($"\tlua_pushnumber(L, (lua_Number){expr});");
		} else if (IsString(type)) {
			sb.AppendLine($"\tlua_pushstring(L, {expr});");
		} else if (IsUtf16InputString(type)) {
			sb.AppendLine($"\tsk_lua_push_utf16(L, {expr});");
		} else if (IsVoidPointer(type)) {
			sb.AppendLine($"\tlua_pushlightuserdata(L, {expr});");
		} else if (IsStructPointer(type)) {
			sb.AppendLine($"\tif ({expr} == NULL) lua_pushnil(L); else sk_lua_push_{CName(type.Name)}(L, *{expr});");
		} else if (_opaqueTypes.Contains(type.Name)) {
			sb.AppendLine($"\tsk_lua_push_asset(L, \"{AssetMetaName(type.Name)}\", (void *){expr});");
		} else if (type.Name == "asset_t") {
			sb.AppendLine($"\tsk_lua_push_asset(L, \"StereoKit.Asset\", (void *){expr});");
		} else if (_structHelpers.Contains(type.Name)) {
			sb.AppendLine($"\tsk_lua_push_{CName(type.Name)}(L, {expr});");
		} else {
			sb.AppendLine("\tlua_pushnil(L);");
		}
	}

	static void PushArrayValue(StringBuilder sb, SKType type, string expr, string countExpr) {
		string iName = $"i_{CName(expr)}";
		sb.AppendLine("\tlua_newtable(L);");
		sb.AppendLine($"\tfor (int32_t {iName} = 0; {iName} < {countExpr}; {iName}++) {{");
		PushValue(sb, type, $"{expr}[{iName}]");
		sb.AppendLine($"\t\tlua_rawseti(L, -2, {iName} + 1);");
		sb.AppendLine("\t}");
	}

	static string AssetMetaName(SKModule m) => AssetMetaName(m.Name.EndsWith("_t") ? m.Name : m.Name + "_t");
	static string AssetMetaName(string cType) {
		string moduleName = cType.EndsWith("_t") ? cType[..^2] : cType;
		return $"StereoKit.{LuaTypeName(moduleName)}";
	}

	static string LuaFunctionName(SKFunction fn, string groupName) {
		if (_overrides.TryGet(fn.Name, out string? custom)) return custom;
		string name = fn.Name;
		string prefix = fn.Module?.Prefix ?? $"{groupName}_";
		if (name.StartsWith(prefix))
			name = name[prefix.Length..];
		if (fn.Relation == SKFunctionRelation.Creation && name.StartsWith("create"))
			name = "new" + name["create".Length..];
		return LuaName(name);
	}

	static string LuaTypeName(string name) {
		if (_overrides.TryGet(name, out string? custom)) return custom;
		if (name.EndsWith("_t")) name = name[..^2];
		if (name.EndsWith("_")) name = name[..^1];
		return SnakeToPascal(name);
	}

	static string LuaEnumItemName(string name, string enumPrefix) {
		if (_overrides.TryGet(name, out string? custom)) return custom;
		if (name.StartsWith(enumPrefix)) name = name[enumPrefix.Length..];
		return LuaName(name);
	}

	static string BuildLuaEnumExpr(string expr, string enumPrefix) {
		if (long.TryParse(expr, out _)) return expr;
		var shift = Regex.Match(expr, @"(\d+)\s*<<\s*(\d+)");
		if (shift.Success)
			return $"({shift.Groups[1].Value} << {shift.Groups[2].Value})";
		return "0";
	}

	static long? InferEnumValue(SKEnum e, SKEnumItem item) {
		long next = 0;
		foreach (var curr in e.Items) {
			long? value = curr.ValueNumeric;
			if (value == null && curr.ValueExpr != null)
				value = TryEvaluateLuaEnumExpr(curr.ValueExpr, e);
			if (curr == item)
				return value ?? next;
			next = (value ?? next) + 1;
		}
		return null;
	}

	static long? TryEvaluateLuaEnumExpr(string expr, SKEnum e) {
		if (long.TryParse(expr, out long value)) return value;
		if (expr.StartsWith("0x", StringComparison.OrdinalIgnoreCase)
			&& long.TryParse(expr[2..], System.Globalization.NumberStyles.HexNumber, null, out value))
			return value;

		var shift = Regex.Match(expr, @"(\d+)\s*<<\s*(\d+)");
		if (shift.Success) {
			long baseValue = long.Parse(shift.Groups[1].Value);
			int amount = int.Parse(shift.Groups[2].Value);
			return baseValue << amount;
		}

		long result = 0;
		bool matched = false;
		foreach (var part in expr.Split('|', StringSplitOptions.RemoveEmptyEntries)) {
			string name = part.Trim();
			var item = e.Items.FirstOrDefault(i => i.Name == name);
			if (item == null) return null;
			long? itemValue = item.ValueNumeric ?? InferEnumValue(e, item);
			if (itemValue == null) return null;
			result |= itemValue.Value;
			matched = true;
		}
		return matched ? result : null;
	}

	static string LuaName(string name) {
		if (_overrides.TryGet(name, out string? custom)) return custom;
		string result = SnakeToCamel(name);
		if (result.Length > 0 && char.IsDigit(result[0])) result = "n" + result;
		return result switch {
			"and" or "break" or "do" or "else" or "elseif" or "end" or "false" or "for" or
			"function" or "goto" or "if" or "in" or "local" or "nil" or "not" or "or" or
			"repeat" or "return" or "then" or "true" or "until" or "while" => result + "_",
			_ => result,
		};
	}

	static string CName(string name) {
		var sb = new StringBuilder();
		foreach (char c in name) {
			if (char.IsLetterOrDigit(c) || c == '_') sb.Append(c);
			else sb.Append('_');
		}
		return sb.ToString();
	}

	static string SnakeToPascal(string name) {
		var parts = name.Split('_', StringSplitOptions.RemoveEmptyEntries);
		var sb = new StringBuilder();
		foreach (var part in parts) {
			sb.Append(char.ToUpperInvariant(part[0]));
			if (part.Length > 1) sb.Append(part[1..]);
		}
		return sb.ToString();
	}

	static string SnakeToCamel(string name) {
		var pascal = SnakeToPascal(name);
		return pascal.Length == 0 ? pascal : char.ToLowerInvariant(pascal[0]) + pascal[1..];
	}

	static string GetModuleFromName(string funcName) {
		int idx = funcName.IndexOf('_');
		return idx > 0 ? funcName[..idx] : funcName;
	}
}

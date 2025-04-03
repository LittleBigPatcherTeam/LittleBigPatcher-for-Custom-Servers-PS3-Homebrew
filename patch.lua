local MAX_URL_LEN_INCL_NULL <const> = 72
local BIGGEST_POSSIBLE_URL_IN_EBOOT_INCL_NULL <const> = 80
local MAX_DIGEST_LEN_INCL_NULL <const> = 20
local BIGGEST_POSSIBLE_DIGEST_IN_EBOOT_INCL_NULL <const> = 24

local BIGGEST_POSSIBLE_URL_IN_LBPK_EBOOT_INCL_NULL <const> = 30

-- any other digests shall be added here for refresh or normalise_digest
local REPLACE_DIGESTS <const> = {
	"!?/*hjk7duOZ1f@daX\x00", -- lbp1, lbp2, lbp3 and many more debug builds digest
	"$ghj3rLl2e5E28@~[!\x00", -- lbpv digest
	"9yF*A&L#5i3q@9|&*F\x00", -- lbp hub digest
	"CustomServerDigest\x00" -- refresher patcher digest
	-- "C5S^FWw*8yIp3O%Me!\x00" -- ps4 digest, ps4 uses a differnt digest calcluation so its not included here
}
local NORMALISE_DIGEST <const> = "!?/*hjk7duOZ1f@daX" -- we will put the trailing null later

local LBP_VITA_URL_FOR_CROSS_CONTROLLER <const> = "lbpvita.online.scee.com\x00"

-- you shouldnt need to use working_dir but its passed nevertheless


---@param str string
---@return string
local function repr(str)
	---@type string
	local result, _ = string.format("%q", str):gsub("\\\n", "\\n")
	return result
end

---@alias file file*

---@class BasePatchInstruction
---@field url string
---@field further_patching_func fun(offset: integer, file: file, url: string, respect_https: boolean): boolean
---@field biggest_possible_size integer
---@field find_string_start string
---@field respect_https boolean
---@field base_patch_group integer
---@field found_a_match boolean
local BasePatchInstruction = {}

-- base_patch_group means if any of them found a match in the group, it will stop doing that group
---@param url string
---@param further_patching_func function
---@param biggest_possible_size integer
---@param find_string_start string
---@param respect_https boolean
---@param base_patch_group integer
function BasePatchInstruction:new(o, url, further_patching_func, biggest_possible_size, find_string_start, respect_https,
								  base_patch_group)
	if not url then error("BasePatchInstruction:new missing attribute: url") end
	if not further_patching_func then error("BasePatchInstruction:new missing attribute: further_patching_func") end
	if not biggest_possible_size then error("BasePatchInstruction:new missing attribute: biggest_possible_size") end
	if not find_string_start then error("BasePatchInstruction:new missing attribute: find_string_start") end
	if respect_https == nil then error("BasePatchInstruction:new missing attribute: respect_https") end
	if not base_patch_group then error("BasePatchInstruction:new missing attribute: base_patch_group") end

	local self = setmetatable({}, BasePatchInstruction)

	self.url = url
	self.further_patching_func = further_patching_func
	self.biggest_possible_size = biggest_possible_size
	self.find_string_start = find_string_start
	self.respect_https = respect_https
	self.base_patch_group = base_patch_group
	self.found_a_match = false
	return self
end

function BasePatchInstruction:print()
	local print_result = string.format("BasePatchInstruction:new(%s, %s, %d, %s, %s, %d); found_a_match = %s",
		repr(self.url), tostring(self.further_patching_func), self.biggest_possible_size, repr(self.find_string_start),
		tostring(self.respect_https), self.base_patch_group, tostring(self.found_a_match))
	print(print_result)
end

function BasePatchInstruction.__index(tab, key)
	return BasePatchInstruction[key]
end

-- https://stackoverflow.com/questions/67255213/how-to-create-a-python-generator-in-lua
local function range_co_for_gen_maker_two_args(co, arg1, arg2)
	return function()
		local err, v = coroutine.resume(co, arg1, arg2)
		if coroutine.status(co) ~= "dead" then
			return v
		end
	end
end

local function gen_maker_two_args(func, arg1, arg2)
	return range_co_for_gen_maker_two_args(coroutine.create(func), arg1, arg2)
end

---@param main_str string
---@param to_find_str string
local function find_all_in_str(main_str, to_find_str)
	local b = 1
	while true do
		local x, y = string.find(main_str, to_find_str, b, true)
		if x == nil then break end
		coroutine.yield(x)
		b = y + 1
	end
end

--- this assumes that the string ends with a null char
---@param input_str string
---@return string|nil
local function remove_trailing_null_chars(input_str)
	for new_str in string.gmatch(input_str, '([^\x00]+)') do
		return new_str
	end
	return nil
end

---@param url string
---@return string|nil
local function extract_domain_from_url(url)
	_, url = url:match("^(https?://)(.+)$")
	if not url then
		return url
	end
	for url in string.gmatch(url, '([^/]+)') do
		return url
	end
	return nil
end

-- less general functions
---@param offset integer
---@param file file
---@param input_data string
---@param biggest_possible_size integer
---@return string|nil
---@return integer|nil
local function check_if_is_valid_null_termed(offset, file, input_data, biggest_possible_size)
	file:seek("set", offset)
	local checking_value = file:read(biggest_possible_size)

	-- assuming that checking_value will not start with a null byte

	local last_occurance_of_null = 0
	local i = 1 -- lua 1th indexed
	for checking_value_char in checking_value:gmatch "." do
		if checking_value_char == "\x00" then
			last_occurance_of_null = i - 1
		else
			if last_occurance_of_null ~= 0 then
				if checking_value_char ~= "\x00" then
					break
				end
			end
		end
		i = i + 1
	end
	if last_occurance_of_null == 0 then
		return nil, nil
	end
	last_occurance_of_null = last_occurance_of_null + 1
	return checking_value, last_occurance_of_null
end

---@param file file
---@param offset integer
---@param new_thing string
---@param in_size_url integer
local function write_new_thing(file, offset, new_thing, in_size_url)
	-- print(new_thing)
	-- print(string.format("%x",string.len(new_thing)))
	-- print(string.format("%x",in_size_url))
	-- print(string.format("%x",offset))
	file:seek("set", offset)
	file:write(string.rep("\x00", in_size_url))
	file:seek("set", offset)
	file:write(new_thing)
end

-- functions specific to a game

---further checking and patching for LITTLEBIGPLANETPS3_XML urls
---@param offset integer
---@param file file
---@param url string
---@param respect_https boolean
---@return boolean
local function lbp_main_is_offset_valid(offset, file, url, respect_https)
	-- can copy and paste this
	local existing_url_orig, in_size_url = check_if_is_valid_null_termed(offset, file, url,
		BIGGEST_POSSIBLE_URL_IN_EBOOT_INCL_NULL)
	if not in_size_url or not existing_url_orig then
		return false
	end
	local existing_url_str = remove_trailing_null_chars(existing_url_orig)
	if not existing_url_str then
		return false
	end
	-- can copy and paste this ^

	if existing_url_str:match "LITTLEBIGPLANETPS3_XML$" == nil and existing_url_str:match "LITTLEBIGPLANETPSP_XML$" == nil then
		return false
	end

	if respect_https then
		local protocol, rest = url:match("^(https?://)(.+)$")
		if not protocol then
			error("your url '" .. url .. "' does not have a valid protocol")
		end

		local existing_protocol = existing_url_str:match("^(https?://)")
		if not existing_protocol then
			-- we could just return false here, but i highly doubt that such a url will exist in normal eboots
			error("the existing_url_str '" .. existing_url_str .. "' does not have a valid protocol")
		end

		url = existing_protocol .. rest
	end
	-- its assumes that you input the url is already concanated with a null at the end
	-- this should be done at the main patch function level, for each call of base patch
	if string.len(url) > in_size_url then
		error("Found a valid url " .. existing_url_str .. " but input url is too big")
	end
	write_new_thing(file, offset, url, in_size_url)
	return true
end

---@param offset integer
---@param file file
---@param thing string
---@param biggest_possible_size integer
---@param thing_type_str_for_error string
---@return boolean
local function basic_replace(offset, file, thing, biggest_possible_size, thing_type_str_for_error)
	-- can copy and paste this
	local existing_url_orig, in_size_url = check_if_is_valid_null_termed(offset, file, thing, biggest_possible_size)
	if not in_size_url or not existing_url_orig then
		return false
	end
	local existing_url_str = remove_trailing_null_chars(existing_url_orig)
	if not existing_url_str then
		return false
	end
	-- can copy and paste this ^

	-- its assumes that you input the url is already concanated with a null at the end
	-- this should be done at the main patch function level, for each call of base patch
	if string.len(thing) > in_size_url then
		error("Found a valid " ..
			thing_type_str_for_error ..
			" " .. existing_url_str .. " but input " .. thing_type_str_for_error .. " is too big")
	end
	write_new_thing(file, offset, thing, in_size_url)
	return true
end

---@param offset integer
---@param file file
---@param digest string
---@param respect_https boolean
local function lbp_main_digest_patch(offset, file, digest, respect_https)
	return basic_replace(offset, file, digest, BIGGEST_POSSIBLE_DIGEST_IN_EBOOT_INCL_NULL, "digest")
end

---@param offset integer
---@param file file
---@param url string
---@param respect_https boolean
---@return boolean
local function lbp_vita_loose_domain_for_cross_controller_app_patch(offset, file, url, respect_https)
	return basic_replace(offset, file, url, BIGGEST_POSSIBLE_URL_IN_EBOOT_INCL_NULL, "url")
end

---@param offset integer
---@param file file
---@param url string
---@param respect_https boolean
---@return boolean
local function lbpk_patch(offset, file, url, respect_https)
	return basic_replace(offset, file, url, BIGGEST_POSSIBLE_URL_IN_LBPK_EBOOT_INCL_NULL, "url")
end

---@param eboot_elf_path string
---@param working_dir string
---@param list_of_patches (BasePatchInstruction)[]
---@return nil
local function base_patch(eboot_elf_path, working_dir,
						  list_of_patches)
	local file = io.open(eboot_elf_path, "r+b")
	if not file then
		error(eboot_elf_path .. " was not found")
	end
	local is_function_succeed, result = pcall(function()
		local done_groups_dict = {}
		local trailing_buffer_size = 0
		for _, value in ipairs(list_of_patches) do
			if value.biggest_possible_size > trailing_buffer_size then
				trailing_buffer_size = value
					.biggest_possible_size
			end
		end
		while true do
			local start_offset = file:seek()
			local chunk = file:read(4096)

			if not chunk then break end
			if string.len(chunk) >= 4096 then
				chunk = chunk .. (file:read(trailing_buffer_size) or "")
				file:seek("cur", -trailing_buffer_size)
			end
			local return_offset = file:seek()
			for index_patch_to_do, patch_to_do in ipairs(list_of_patches) do
				if done_groups_dict[patch_to_do.base_patch_group] and done_groups_dict[patch_to_do.base_patch_group] ~= index_patch_to_do then
					-- continue	
				else
					for offset in gen_maker_two_args(find_all_in_str, chunk, patch_to_do.find_string_start) do
						offset = (offset + start_offset) - 1 -- lua 1th index finna drive me crazy
						if patch_to_do.further_patching_func(offset, file, patch_to_do.url, patch_to_do.respect_https) then
							for inner_patch_to_do_index, inner_patch_to_do in ipairs(list_of_patches) do
								if inner_patch_to_do.base_patch_group == patch_to_do.base_patch_group then
									inner_patch_to_do.found_a_match = true
								end
							end
							done_groups_dict[patch_to_do.base_patch_group] = index_patch_to_do
						end
					end
				end
			end
			file:seek("set", return_offset)
		end
		return nil
	end)
	file:close()
	if not is_function_succeed then
		error(result)
	end

	return result
end

---@param eboot_elf_path string
---@param url string
---@param digest string
---@param normalise_digest boolean
---@param working_dir string
---@param respect_https boolean
---@param lbp_vita_loose_domain_for_cross_controller_app boolean
---@return boolean
local function internal_patch_lbp_main(eboot_elf_path, url, digest, normalise_digest, working_dir, respect_https,
									   lbp_vita_loose_domain_for_cross_controller_app)
	---@type (BasePatchInstruction)[]
	local patches_list = {}
	local base_patch_instruction = BasePatchInstruction:new(nil, url .. "\x00", lbp_main_is_offset_valid,
		BIGGEST_POSSIBLE_URL_IN_EBOOT_INCL_NULL, "http", respect_https, 1)
	table.insert(patches_list, base_patch_instruction)
	-- digest will always be a string, empty if not provided
	if digest == '' and normalise_digest then
		digest = NORMALISE_DIGEST
	end

	if digest ~= '' then
		for _, digest_to_find in ipairs(REPLACE_DIGESTS) do
			base_patch_instruction = BasePatchInstruction:new(nil, digest .. "\x00", lbp_main_digest_patch,
				BIGGEST_POSSIBLE_DIGEST_IN_EBOOT_INCL_NULL, digest_to_find, false, 2)
			table.insert(patches_list, base_patch_instruction)
		end
	end
	if lbp_vita_loose_domain_for_cross_controller_app then
		local domain = extract_domain_from_url(url)
		if not domain then
			error("url does not start with http:// or https://")
		end

		base_patch_instruction = BasePatchInstruction:new(nil, domain .. "\x00",
			lbp_vita_loose_domain_for_cross_controller_app_patch, BIGGEST_POSSIBLE_URL_IN_EBOOT_INCL_NULL,
			LBP_VITA_URL_FOR_CROSS_CONTROLLER, false, 3)
		table.insert(patches_list, base_patch_instruction)
	end
	base_patch(eboot_elf_path, working_dir, patches_list)

	for _, value in ipairs(patches_list) do
		if value.base_patch_group == 1 and not value.found_a_match then
			error("Could not find any urls to patch")
		end
		if value.base_patch_group == 2 and not value.found_a_match then
			error("Could not find any digests to patch")
		end
		if value.base_patch_group == 3 and not value.found_a_match then
			error(
				"Could not lbp vita cross controller domain, likely you gave an eboot that is not the cross controller app")
		end
	end
	return true
end

patch_method_lbp_main = "LittleBigPlanet Main Series"
---@param eboot_elf_path string
---@param url string
---@param digest string
---@param normalise_digest boolean
---@param working_dir string
---@return boolean
function patch_lbp_main(eboot_elf_path, url, digest, normalise_digest, working_dir)
	return internal_patch_lbp_main(eboot_elf_path, url, digest, normalise_digest, working_dir, false, false)
end

patch_method_ps3_lbpk = "LittleBigPlanet Karting"
---@param eboot_elf_path string
---@param url string
---@param digest string
---@param normalise_digest boolean
---@param working_dir string
---@return boolean
function patch_ps3_lbpk(eboot_elf_path, url, digest, normalise_digest, working_dir)
	---@type (BasePatchInstruction)[]
	local patches_list = { BasePatchInstruction:new(nil, url .. "\x00", lbpk_patch,
		BIGGEST_POSSIBLE_URL_IN_LBPK_EBOOT_INCL_NULL, "lbpk.ps3.online.scea.com\x00", false, 1) }
	base_patch(eboot_elf_path, working_dir, patches_list)
	if not patches_list[1].found_a_match then
		error("Could not find any urls to patch")
	end
	return true
end

patch_method_vita_lbp_vita_cross_controller = "LittleBigPlanet Cross Controller Vita app"
---@param eboot_elf_path string
---@param url string
---@param digest string
---@param normalise_digest boolean
---@param working_dir string
---@return boolean
function patch_vita_lbp_vita_cross_controller(eboot_elf_path, url, digest, normalise_digest, working_dir)
	return internal_patch_lbp_main(eboot_elf_path, url, digest, normalise_digest, working_dir, true, true)
end

patch_method_vita_test_patch = "Test patch. If you chose the correct game, it should error on open"
---@param eboot_elf_path string
---@param url string
---@param digest string
---@param normalise_digest boolean
---@param working_dir string
---@return boolean
function patch_vita_test_patch(eboot_elf_path, url, digest, normalise_digest, working_dir)
	local file = io.open("app0:/vita_test_patch_file.velf", "rb")
	if not file then
		error("could not open app0:/vita_test_patch_file.velf")
	end
	local is_function_succeed, result = pcall(function()
		return file:read()
	end)
	file:close()
	if not is_function_succeed then
		error(result)
	end

	local file_input_elf = io.open(eboot_elf_path, "wb")
	if not file_input_elf then
		error(eboot_elf_path .. " was not found")
	end
	local is_function_succeed_2, result_2 = pcall(function()
		return file_input_elf:write(result)
	end)
	file_input_elf:close()
	if not is_function_succeed_2 then
		error(result_2)
	end
	
	return true
end

patch_method_ps3_test_patch = "Test patch. If you chose the correct game, it should quit on open automatically"
---@param eboot_elf_path string
---@param url string
---@param digest string
---@param normalise_digest boolean
---@param working_dir string
---@return boolean
function patch_ps3_test_patch(eboot_elf_path, url, digest, normalise_digest, working_dir)
	local file = io.open("/dev_hdd0/game/LBPCSPPHB/USRDIR/ps3_test_patch_file.elf", "rb")
	if not file then
		error("could not open /dev_hdd0/game/LBPCSPPHB/USRDIR/ps3_test_patch_file.elf")
	end
	local is_function_succeed, result = pcall(function()
		return file:read()
	end)
	file:close()
	if not is_function_succeed then
		error(result)
	end

	local file_input_elf = io.open(eboot_elf_path, "wb")
	if not file_input_elf then
		error(eboot_elf_path .. " was not found")
	end
	local is_function_succeed_2, result_2 = pcall(function()
		return file_input_elf:write(result)
	end)
	file_input_elf:close()
	if not is_function_succeed_2 then
		error(result_2)
	end
	
	return true
end

function example_usage()
	patch_lbp_main("EBOOT.ELF", "http://lnfinite.site/LITTLEBIGPLANETPS3_XML", "", true, "./")
end
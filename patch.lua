MAX_URL_LEN_INCL_NULL = 72
BIGGEST_POSSIBLE_URL_IN_EBOOT_INCL_NULL = 80
MAX_DIGEST_LEN_INCL_NULL = 20
BIGGEST_POSSIBLE_DIGEST_IN_EBOOT_INCL_NULL = 24

BIGGEST_POSSIBLE_URL_IN_LBPK_EBOOT_INCL_NULL = 30

PATCH_ERR_EBOOT_ELF_NO_EXISTS = 1
PATCH_ERR_NO_URLS_FOUND = 2
PATCH_ERR_NO_DIGESTS_FOUND = 3
PATCH_ERR_THIS_SHOULD_NEVER_HAPPEN = 4

-- you shouldnt need to use working_dir but its passed nevertheless

-- https://stackoverflow.com/questions/67255213/how-to-create-a-python-generator-in-lua
function range_co_for_gen_maker(co,arg1,arg2)
  return function()
    local err, v = coroutine.resume(co,arg1,arg2)
    if coroutine.status(co) ~= "dead" then
      return v
    end
  end
end

function gen_maker(func,arg1,arg2)
    return range_co_for_gen_maker(coroutine.create(func),arg1,arg2)
end

function find_all_in_str(main_str, to_find_str)
	b=1
	while true do
		local x,y=string.find(main_str,to_find_str,b,true)
		if x==nil then break end
		coroutine.yield(x)
		b=y+1
	end
end

-- less general functions
function check_if_is_valid_null_termed(offset, file, input_data, biggest_possible_size)
	file:seek("set",offset)
	checking_value = file:read(biggest_possible_size)
	
	-- assuming that checking_value will not start with a null byte

	last_occurance_of_null = 1 -- lua 1th indexed
	local i = 1
	for checking_value_char in checking_value:gmatch"." do
		if last_occurance_of_null <> 1 then
			if checking_value_char <> "\x00" then break end
		end
		if checking_value_char == "\x00" then
			last_occurance_of_null = i
			break
		end
		index = index + 1
	end
	if last_occurance_of_null == 0 then
		return false
	end
	
	return last_occurance_of_null
end

-- functions spefic to a game
function lbp_main_is_offset_valid(offset, file, url)
	in_size_url = check_if_is_valid_null_termed(offset, file, url, BIGGEST_POSSIBLE_URL_IN_EBOOT_INCL_NULL)
end


function base_patch(eboot_elf_path, url, digest, normalise_digest, working_dir, 
trailing_buffer_size, find_string_start)
	local file = io.open(eboot_elf_path,"r+b")
	if not file then
		return PATCH_ERR_EBOOT_ELF_NO_EXISTS
	end
	local is_function_succeed,result = pcall(function()
		while true do
			start_offset = file:seek()
			chunk = file:read(4096)
			file:seek("cur",-trailing_buffer_size)
			if not chunk then break end
			chunk = chunk .. (file:read(trailing_buffer_size) or "")
			
			return_offset = file:seek()
			for offset in gen_maker(find_all_in_str,chunk,find_string_start) do
				offset = (offset + start_offset) - 1 -- lua 1th index finna drive me crazy
				
			end
			file:seek("set",return_offset)
		end
		return 0
	end)
	file:close()
	if not is_function_succeed then
		error(result)
	end
	
	return result
end

-- patch_method_lbp_main = "LittleBigPlanet Main Series"
-- function patch_lbp_main(eboot_elf_path, url, digest, normalise_digest, working_dir)
	-- base_patch(eboot_elf_path, url, digest, normalise_digest, working_dir,BIGGEST_POSSIBLE_URL_IN_EBOOT_INCL_NULL,"http")
-- end

-- patch_lbp_main("EBOOT.ELF","http://lnfinite.site/LITTLEBIGPLANETPS3_XML","",true,"./")
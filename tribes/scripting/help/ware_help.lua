--  =======================================================
--  *************** Basic helper functions ****************
--  =======================================================

-- RST
-- .. function:: image_line(image, count[, text = nil])
--
--    Aligns the image to a row on the right side with text on the left.
--
--    :arg image: the picture to be aligned to a row.
--    :arg count: length of the picture row.
--    :arg text: if given the text aligned on the left side, formatted via
--       formatting.lua functions.
--    :returns: the text on the left and a picture row on the right.
--
function image_line(image, count, text)
	local imgs={}
	for i=1,count do
		imgs[#imgs + 1] = image
	end
	local imgstr = table.concat(imgs, ";")

	if text then
		return rt("image=" .. imgstr .. " image-align=right", "  " .. text)
	else
		return rt("image=" .. imgstr .. " image-align=right", "")
	end
end


--  =======================================================
--  ************* Main worker help functions *************
--  =======================================================

-- RST
-- .. function ware_help_string(tribe, ware_description)
--
--    Displays the ware with a helptext and an image
--
--    :arg tribe: The :class:`LuaTribeDescription` for a tribe that uses this ware.
--    :arg ware_description: the worker_description from C++.
--    :returns: Help string for the ware
--
function ware_help_string(tribe, ware_description)
	include(ware_description.directory .. "helptexts.lua")

	local result = ware_helptext()
	if (result ~= "") then
		result = result .. " "
	else
		result = ""
	end
	result = result .. ware_helptext(tribe)

	-- TODO(GunChleoc): Split into purpose and note
	-- We also want the ware quantity info collected while loading the tribes.

	result = rt("image=" .. ware_description.icon_name, p(result))
	return result
end


return {
   func = function(tribe, ware_description)
      set_textdomain("tribes_encyclopedia")
	   return ware_help_string(tribe, ware_description)
   end
}

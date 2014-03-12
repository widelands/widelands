-- RST
-- format_help.lua
-- ---------------
--
-- Functions used in the ingame help windows for formatting the text and pictures.

-- RST
-- .. function:: dependencies(images[, text = nil])
--
--    Creates a dependencies line of any length.
--
--    :arg images: images in the correct order from left to right as table (set in {}).
--    :arg text: comment of the image.
--    :returns: a row of pictures connected by arrows.
--
function dependencies(images, text)
	if not text then
		text = ""
	end

	string = "image=" .. images[1]
	for k,v in ipairs({unpack(images,2)}) do
		string = string .. ";pics/arrow-right.png;" .. v
	end

	return rt(string, text)
end

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

-- RST
-- .. function text_line(t1, t2[, imgstr = nil])
--
--    Creates a line of h3 formatted text followed by normal text and an image.
--
--    :arg t1: text in h3 format.
--    :arg t2: text in p format.
--    :arg imgstr: image aligned right.
--    :returns: header followed by normal text and image.
--
function text_line(t1, t2, imgstr)
	if imgstr then
		return "<rt text-align=left image=" .. imgstr .. " image-align=right><p font-size=13 font-color=D1D1D1>" ..  t1 .. "</p><p line-spacing=3 font-size=12>" .. t2 .. "<br></p><p font-size=8> <br></p></rt>"
	else
		return "<rt text-align=left><p font-size=13 font-color=D1D1D1>" ..  t1 .. "</p><p line-spacing=3 font-size=12>" .. t2 .. "<br></p><p font-size=8> <br></p></rt>"
	end
end

-- Tabs für die Hilfe, weiß aber nicht, ob das wirklich so funktionieren kann,
-- oder wie ich das eigentlich anwenden muss… Oder was ich falsch mache…
function make_tabs_array(t1, t2)
  return { {
        text = t1,
        tab_picture = "pics/small.png", -- Graphic for the tab button
     },
     {
        text = t2,
        tab_picture = "pics/medium.png",
     }
  }
end


-- RST
-- .. function building_size_string(tribename, buildingname)
--
--    Creates a text_line that describes the building's size in a help text.
--
--    :arg t1: tribename, e.g. "barbarians".
--    :arg t2: buildingname, e.g. "lumberjakcs_hut".
--    :returns: "Space required" header followed by size description text and image.
--
function building_size_string(tribename, buildingname)

  local building_descr = wl.Game():get_building_description(tribename,buildingname)

  if(building_descr.ismine) then
	return text_line(_"Space required:",_"Mine plot","pics/mine.png")
  elseif(building_descr.isport) then
	return text_line(_"Space required:",_"Port plot","pics/port.png")
  else
	if (building_descr.size == 1) then
 		return text_line(_"Space required:",_"Small plot","pics/small.png")
	elseif (building_descr.size == 2) then
  		return text_line(_"Space required:",_"Medium plot","pics/medium.png")
	elseif (building_descr.size == 3) then
		return text_line(_"Space required:",_"Big plot","pics/big.png")
	else
		return p(_"Space required:" .. _"Unknown")
	end
  end
end

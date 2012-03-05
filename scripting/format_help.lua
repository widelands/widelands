-- RST
-- format_help.lua
-- ---------------
--
-- Functions used in the ingame help windows for formatting the text and pictures.

-- RST
-- .. function:: image_line_dep(image1, image2, image3[, text = nil])
--
--    Creates a dependencies line.
--
--    :arg image1-3: images in the correct order from left to right from ressource
--       to the following worker.
--    :arg text: normally the name of the worker.
--    :returns: a row of pictures connected by arrows. If there is no text the
--       row will be aligned right else left with following text.
--
function image_line_dep(image1, image2, image3, text)
	local imgs={image1, "pics/arrow-right.png", image2, "pics/arrow-right.png", image3}
	local imgstr = table.concat(imgs, ";")

	if text then
		return rt("image=" .. imgstr, text)
	else
		return rt("image=" .. imgstr, "")
	end
end

-- RST
-- .. function:: image_line_head(image, count[, text = nil])
--
--    Aligns the image to a row on the right side with text on the left.
--
--    :arg image: the picture to be aligned to a row.
--    :arg count: length of the picture row.
--    :arg text: if given the text aligned on the left side, formatted via
--       formatting.lua functions.
--    :returns: the text on the left and a picture row on the right.
--
function image_line_text(image, count, text)
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
-- .. function text_line(t1, t2)
--
--    Creates a line of h3 formatted text followed by normal text.
--
--    :arg t1: text in h3 format.
--    :arg t2: text in p format.
--    :returns: header followed by normal text.
--
function text_line(t1, t2)
	return "<rt text-align=left><p font-size=13 font-color=D1D1D1>" ..  t1 ..
		"</p><p line-spacing=3 font-size=12>" .. t2 .. "<br></p><p font-size=8> <br></p></rt>"
end

-- h3-Header, Text, Image
function text_image_line(t1, t2, imgstr)
	return "<rt text-align=left image=" .. imgstr .. " image-align=right><p font-size=13 font-color=D1D1D1>" ..  t1 ..
		"</p><p line-spacing=3 font-size=12>" .. t2 .. "<br></p><p font-size=8> <br></p></rt>"
end

-- Two images connected by arrows followed by text.
function image_line_out(image1, image2, text)
	local imgs={image1, "pics/arrow-right.png", image2}
	local imgstr = table.concat(imgs, ";")

	if text then
		return rt("image=" .. imgstr, text)
	else
		return rt("image=" .. imgstr .. " image-align=right", "")
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
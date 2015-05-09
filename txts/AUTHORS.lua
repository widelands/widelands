include "scripting/formatting.lua"
include "txts/developers.lua"
include "txts/translators/translators.lua"

set_textdomain("texts")

-- Formatting functions
function h1_authors(text)
	return "<rt text-align=left><p font-size=24 font-decoration=bold font-face=serif font-color=f4a131>" .. text .. "</p></rt>"
end

function h2_authors(text)
	return "<rt><p font-size=6> <br></p>" .. h2(text) .. "</rt>"
end

function p_authors(person, image)
	return "<rt image=" .. image .. " text-align=left image-align=left><p font-size=12>" .. person .. "</p></rt>"
end


-- Uses structured data to format developers and inserts translators before packagers
function list_authors()
	local developers = developers()
	local result = ""
	for i, category in ipairs(developers) do
		if (category["heading"] == _"Packagers") then -- Translators come from a separate list
			result = result .. list_translators(translators())
		end
		result = result .. "<rt>" .. h1_authors(category["heading"]) .. "</rt>"
		for j, entry in ipairs(category["entries"])  do
			if (entry["subheading"] ~= nil) then
				result = result .. h2_authors(entry["subheading"])
			end
			for k, member in ipairs(entry["members"])  do
				result = result .. p_authors(member, category["image"] )
			end
		end
		result = result .. rt("<p font-size=10> <br></p>")
	end
	return result
end

-- Uses structured data to format translators
function list_translators(translators)
		local result = ""
		result = result .. "<rt>" .. h1_authors(_"Translators") .. "</rt>"
		for i, entry in ipairs(translators) do
			result = result .. h2_authors(entry[1])
			for i, person in ipairs(entry[2])  do
			   result = result .. p_authors(person, "pics/fsel_editor_set_height.png")
		   end
		end
		return result
end

-- Main script
return {
   title = _"Authors",
   text = rt(
		"<rt text-align=center><p font-size=28 font-decoration=bold font-face=serif font-color=2F9131>" .. _"Widelands Development Team" .. "</p></rt>" ..
		list_authors()
	)
}

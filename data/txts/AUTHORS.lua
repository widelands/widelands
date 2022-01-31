include "txts/format_authors.lua"
include "txts/developers.lua"

push_textdomain("texts")

-- Uses structured data to format authors
function list_authors()
   local authors = developers()
   local result = ""
   for i, category in ipairs(authors) do
      result = result .. h1_authors(category["heading"])
      for j, entry in ipairs(category["entries"])  do
         if (entry["subheading"] ~= nil) then
            result = result .. h2_authors(entry["subheading"])
         end
         for k, member in ipairs(entry["members"])  do
            result = result .. p(img(category["image"]) .. " " .. member)
         end
      end
      result = result .. p(vspace(6))
   end
   return result
end

-- Main script
local r = {
   rt(fs_color(
      p_font("align=center", "size=28 color=2F9131", _("Widelands Development Team")) ..
      list_authors()
   ))
}
pop_textdomain()
return r

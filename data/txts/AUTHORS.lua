include "txts/format_authors.lua"
include "txts/developers.lua"

push_textdomain("texts")
set_fs_style(true)

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
         result = result .. columns(entry["members"], img(category["image"]))
      end
   end
   return div("width=100%", pagetitle(_("Widelands Development Team")) .. result)
end

-- Main script
local r = {
   rt(list_authors())
}

set_fs_style(false)
pop_textdomain()
return r

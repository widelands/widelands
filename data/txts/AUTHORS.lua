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
         for k, member in ipairs(entry["members"])  do
            result = result .. p("valign=center", img(category["image"]) .. " " .. member)
         end
      end
   end
   return result
end

-- Main script
local r = {
   rt(div("width=100%",
      pagetitle(_("Widelands Development Team")) ..
      list_authors()
   ))
}

set_fs_style(false)
pop_textdomain()
return r

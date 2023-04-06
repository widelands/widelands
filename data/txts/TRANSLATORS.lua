include "txts/format_authors.lua"
include "txts/translators_data.lua"

push_textdomain("texts")
set_fs_style(true)

-- Uses structured data to format authors
function list_authors()
   local authors = translators()
   local result = ""
   for i, category in ipairs(authors) do
      result = result .. h2_authors(category["heading"])
      for j, entry in ipairs(category["entries"])  do
         for k, member in ipairs(entry["members"])  do
            result = result .. p("• " .. member)
         end
      end
      result = result .. p(" ")
   end
   return result
end

-- Main script
local r = {
   rt(div("width=100%",
      pagetitle(_("Widelands Translators")) ..
      list_authors()
   ))
}

set_fs_style(false)
pop_textdomain()
return r

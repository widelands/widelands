include "txts/format_authors.lua"
include "txts/translators_data.lua"

push_textdomain("texts")

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
      result = result .. p(vspace(6))
   end
   return result
end

-- Main script
local r = {
   rt(fs_color(
      p_font("align=center", "size=28 color=2F9131", _("Widelands Translators")) ..
      list_authors()
   ))
}
pop_textdomain()
return r

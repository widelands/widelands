include "txts/format_authors.lua"
include "txts/developers.lua"

set_textdomain("texts")

-- Uses structured data to format authors
function list_authors()
   local authors = developers()
   local result = ""
   for i, category in ipairs(authors) do
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

-- Main script
return {
   title = _"Developers",
   text = rt(
      "<rt text-align=center><p font-size=28 font-decoration=bold font-face=serif font-color=2F9131>" .. _"Widelands Development Team" .. "</p></rt>" ..
      list_authors()
   )
}

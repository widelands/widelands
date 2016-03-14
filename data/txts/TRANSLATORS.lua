include "txts/format_authors.lua"
include "txts/translators_data.lua"

set_textdomain("texts")

-- Uses structured data to format authors
function list_authors()
   local authors = translators()
   local result = ""
   local translators_image = "images/wui/editor/fsel_editor_set_height.png"
   for i, category in ipairs(authors) do
      result = result .. "<rt>" .. h2_authors(category["heading"]) .. "</rt>"
      for j, entry in ipairs(category["entries"])  do
         for k, member in ipairs(entry["members"])  do
            result = result .. p_authors(member, translators_image)
         end
      end
      result = result .. rt("<p font-size=10> <br></p>")
   end
   return result
end

-- Main script
return {
   title = _"Translators",
   text = rt(
      title(_"Widelands Translators") ..
      list_authors()
   )
}

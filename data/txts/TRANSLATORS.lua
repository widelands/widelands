include "txts/format_authors.lua"
include "txts/translators_data.lua"

push_textdomain("texts")
set_fs_style(true)

local translators_ = translators()
local languages = {}
for lang, data in pairs(translators_) do
   table.insert(languages, lang)
end
table.sort(languages)

local function list_one_language(lang)
   local data = translators_[lang]
   if data == nil then
      print("txts/TRANSLATORS.lua: Invalid language code: " .. lang)
      return ""
   end
   local result = h2_authors(data["heading"])
   if #data.entries ~= 1 then
      print("Language " .. lang .. " has multiple translator lists")
   end
   for j, entry in ipairs(data["entries"]) do
      result = result .. columns(entry["members"], "•")
   end
   result = result .. p(" ")
   return result
end

function list_language_translators(lang)
   local result = list_one_language(lang)
   return div("width=100%", h1_authors(_("Widelands Translators")) .. result )
end

-- Uses structured data to format authors
function list_translators()
   local result = ""
   for i, lang in ipairs(languages) do
      result = result .. list_one_language(lang)
   end
   return div("width=100%", pagetitle(_("Widelands Translators")) .. result )
end

-- Main script
local r = {
   rt(list_translators())
}

set_fs_style(false)
pop_textdomain()
return r

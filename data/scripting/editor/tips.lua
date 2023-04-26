include "scripting/richtext.lua"

function get_editor_tips()
   include "txts/tips/editor.lua"
   return tips
end

local text = ""
for index, contents in pairs(get_editor_tips()) do
   text = text .. li(contents["text"])
end

push_textdomain("widelands_editor")
local r = {
  title = _("Tips"),
  text = text
}
pop_textdomain()
return r

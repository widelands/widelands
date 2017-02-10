include "scripting/formatting.lua"

function get_editor_tips()
   include "txts/tips/editor.lua"
   return tips
end

local text = paragraphdivider()
for index, contents in pairs(get_editor_tips()) do
   text = text .. listitem_bullet(contents["text"])
end
text = text .. "</p>"

set_textdomain("widelands_editor")
return {
  title = _"Tips",
  text = rt(text)
}

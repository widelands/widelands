-- TODO(GunChleoc): This is for the legacy font renderer. Remove when we remove the old renderer and break savegame compatibility.


function speech(img, clr, g_title, g_text)
   local title, text = g_title, g_text
   if not text then
      title = nil
      text = g_title
   end

   -- Surround the text with translatable ","
   text = (_'“%s”'):format(text)

   local s = ""
   if title then
      s = rt("<p font-size=20 font-weight=bold font-face=serif " ..
         ("font-color=%s>"):format(clr) .. title ..
         "</p><p font-size=8> <br></p>"
      )
   end

   return s .. rt(("image=%s"):format(img), p(text))
end


function objective_text(heading, body)
   return rt(h2(heading) .. p(body))
end


function new_objectives(...)
   local sum = 0
   local s = ""
   for idx,obj in ipairs{...} do
      s = s .. obj.body
      sum = sum + obj.number
   end
   return rt("<p font-size=10> <br></p>" ..
      "<p font=serif font-size=18 font-weight=bold font-color=D1D1D1>"
      .. ngettext("New Objective", "New Objectives", sum) .. "</p>") .. s
end

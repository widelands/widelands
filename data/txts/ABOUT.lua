-- Index for the "About" screen tabs

push_textdomain("texts")

local r = {
   {
      name =  _("Readme"),
      script = "txts/README.lua"
   },
   {
      name =  _("License"),
      script = "txts/LICENSE.lua"
   },
   {
      name =  _("Developers"),
      script = "txts/AUTHORS.lua"
   },
   {
      name =  _("Translators"),
      script = "txts/TRANSLATORS.lua"
   }
}
pop_textdomain()
return r

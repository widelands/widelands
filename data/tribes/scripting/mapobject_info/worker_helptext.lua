-- This is used by the mapinfo standalone executable to get workers' helptexts.
-- Note that this can't handle localization properly.

return {
   func = function(helptext_script)
      include(helptext_script)
      return worker_helptext()
   end
}

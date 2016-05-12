-- This is used by the mapinfo standalone executable to get workers' helptexts.
-- Note that this can't handle localization properly.

return {
   func = function(workername)
      local worker_description = wl.Game():get_worker_description(workername)
      include(worker_description.helptext_script)
      return worker_helptext()
   end
}

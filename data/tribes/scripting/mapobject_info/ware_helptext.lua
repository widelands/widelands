-- This is used by the mapinfo standalone executable to get wares' helptexts.
-- Note that this can't handle localization properly.

return {
   func = function(tribename, warename)
      local ware_description = wl.Game():get_ware_description(warename)
      include(ware_description.helptext_script)
      return ware_helptext(tribename)
   end
}

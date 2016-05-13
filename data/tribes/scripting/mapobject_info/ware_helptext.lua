-- This is used by the mapinfo standalone executable to get wares' helptexts.
-- Note that this can't handle localization properly.

return {
   func = function(tribename, helptext_script)
      include(helptext_script)
      return ware_helptext(tribename)
   end
}

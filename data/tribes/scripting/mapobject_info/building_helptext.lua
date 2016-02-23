-- This is used by the mapinfo standalone executable to get buildings' helptexts.
-- Note that this can't handle localization properly.

return {
   func = function(building_description)
      include(building_description.helptext_script)
      return building_helptext_purpose()
   end
}

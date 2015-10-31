function ware_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a ware: Light Trident
      atlanteans = pgettext("atlanteans_ware", "This is the basic weapon of the Atlantean soldiers. Together with a tabard, it makes up the equipment of young soldiers. Light tridents are produced in the weapon smithy as are all other tridents.")
   }
   local result = ""
   if tribe then
      result = helptext[tribe]
   else
      result = helptext["default"]
   end
   if (result == nil) then result = "" end
   return result
end
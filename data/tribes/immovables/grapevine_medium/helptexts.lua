function immovable_helptext(tribe)
   local helptext = {
      -- TRANSLATORS: Helptext for a grapevine
      default = _("This grapevine is growing.")
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

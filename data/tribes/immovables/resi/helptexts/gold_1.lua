function immovable_helptext(tribe)
   local helptext = {
      default = pgettext("sentence_separator", "%s %s"):bformat(
         -- TRANSLATORS: Helptext for a resource: Gold
         _("Gold veins contain gold ore that can be dug up by gold mines."),
         -- TRANSLATORS: Helptext for a resource: Gold
         _("There is only a little bit of gold here."))
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

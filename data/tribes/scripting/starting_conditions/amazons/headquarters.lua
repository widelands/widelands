-- =======================================================================
--                 Start conditions for Headquarters
-- =======================================================================

include "scripting/infrastructure.lua"

set_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _ "Headquarters",
   -- TRANSLATORS: This is the tooltip for the "Headquarters" starting condition
   tooltip = _"Start the game with your headquarters only",
   func = function(player, shared_in_start)

   local sf = wl.Game().map.player_slots[player.number].starting_field
   if shared_in_start then
      sf = shared_in_start
   else
      player:allow_workers("all")
   end

   hq = prefilled_buildings(player, { "amazons_headquarters", sf.x, sf.y,
      wares = {
         log = 40,
         granite = 50,
         water = 10,
         fish = 10,
         meat = 10,
         cassava_root = 5,
         ration = 20,
         coal = 20,
         gold = 4,
         gold_dust = 5,
         bread_amazons = 15,
         hammer = 5,
         kitchen_tools = 2,
         felling_ax = 3,
         needles = 1,
         pick = 5,
         shovel = 5,
         hunting_spear = 2,
         fishing_net = 3,
         bread_amazons= 20,
         cocoa_beans = 5,
         chocolate= 15,
         tunic= 15,
         vest_padded= 2,
         protector_padded= 1,
         helmet_wooden= 1,
         boots_sturdy= 2,
         boots_swift= 1,
         boots_hero= 1,
         spear_wooden= 15,
         spear_stone_tipped= 1,
         spear_hardened= 1,
         armor_wooden= 2,
         warriors_coat= 1,
         stonebowl= 1,
         chisel= 2,
         rope= 20,

      },
      workers = {
         amazons_cook = 3,
         amazons_builder = 10,
         amazons_charcoal_burner = 1,
         amazons_gold_digger = 1,
         amazons_gold_smelter = 1,
         amazons_carrier = 40,
         amazons_cassava_farmer = 1,
         amazons_cocoa_farmer = 1,
         amazons_wilderness_keeper = 1,
         amazons_stonecarver = 2,
         amazons_geologist = 4,
         amazons_woodcutter = 3,
         amazons_liana_cutter = 1,
         amazons_jungle_preserver = 2,
         amazons_stonecutter = 2,
         amazons_trainer = 3,
         amazons_dressmaker = 1,
         amazons_tapir= 5,
      },
      soldiers = {
         [{0,0,0,0}] = 45,
      }
   })
end
}

return init

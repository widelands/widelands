-- This file is only used by the random map generator. It is not part of the
-- world definition (i.e. it is not sourced by world/init.lua) and is only
-- loaded when the random map generator is initialized in the Editor.


------------------------
--  Former greenland  --
------------------------

##### This info is needed by the random map generator #####



##########################################################
########## Height, area and terrain definitions ##########
##########################################################

[heights]
ocean=10
shelf=10
shallow=10
coast=11
upperland=14
mountainfoot=15
mountain=16
snow=39
summit=45

[areas]
water=water
land=land,wiese1
wasteland=steppe,sumpf
mountains=mountains

[water]
ocean_terrains=wasser
shelf_terrains=wasser
shallow_terrains=wasser


[land]

weight=2

coast_terrains=strand
land_terrains=wiese2,wiese3
upper_terrains=wiese2,wiese3


[wiese1]

weight=1

coast_terrains=strand
land_terrains=wiese1
upper_terrains=wiese2,wiese3


[sumpf]
inner_terrains=sumpf
outer_terrains=sumpf


[steppe]
inner_terrains=steppe_kahl
outer_terrains=steppe


[mountains]
mountainfoot_terrains=bergwiese
mountain_terrains=berg1,berg2,berg3,berg4
snow_terrains=schnee


#####################################
########## BOB definitions ##########
#####################################


# The postfix "_lr" means "land resource"

[bobs]
areas=forest_lr,bushland_lr,meadow_lr,stone_lr,sstone_lr
bob_kinds=tree_bobs,bush_bobs,grass_bobs,stone_bobs,sstone_bobs,wasteland_bobs

[forest_lr]
weight=6
immovable_density=100
moveable_density=20
land_coast_bobs=bush_bobs
land_inner_bobs=tree_bobs
land_upper_bobs=bush_bobs
wasteland_inner_bobs=wasteland_bobs
wasteland_outer_bobs=bush_bobs

[bushland_lr]
weight=2
immovable_density=50
moveable_density=20
land_coast_bobs=grass_bobs
land_inner_bobs=bush_bobs
land_upper_bobs=grass_bobs
wasteland_inner_bobs=wasteland_bobs
wasteland_outer_bobs=grass_bobs

[meadow_lr]
weight=2
immovable_density=10
moveable_density=20
land_coast_bobs=grass_bobs
land_inner_bobs=grass_bobs
land_upper_bobs=grass_bobs
wasteland_inner_bobs=wasteland_bobs
wasteland_outer_bobs=grass_bobs

[stone_lr]
weight=4
immovable_density=100
moveable_density=0
land_coast_bobs=
land_inner_bobs=stone_bobs
land_upper_bobs=
wasteland_inner_bobs=stone_bobs
wasteland_outer_bobs=stone_bobs

[sstone_lr]
weight=2
immovable_density=100
moveable_density=0
land_coast_bobs=
land_inner_bobs=sstone_bobs
land_upper_bobs=
wasteland_inner_bobs=sstone_bobs
wasteland_outer_bobs=sstone_bobs

[tree_bobs]
immovable_bobs=tree1,tree2,tree3,tree4,tree5,tree6,tree7,tree8
moveable_bobs=brownbear,chamois,deer,elk,lynx,reindeer,wolf

[bush_bobs]
immovable_bobs=bush1,bush2,bush3,bush4,bush5
moveable_bobs=wisent,wolf,bunny,elk,fox,lynx,wolf,marten,sheep,sheep

[grass_bobs]
immovable_bobs=grass1,grass2,grass3
moveable_bobs=bunny,elk,fox,lynx,wolf,marten,sheep,sheep

[sstone_bobs]
immovable_bobs=sstones1,sstones2,sstones3,sstones4,sstones5,sstones6,sstones7
moveable_bobs=

[stone_bobs]
immovable_bobs=stones1,stones2,stones3,stones4,stones5,stones5,stones6,stones6,stones6
moveable_bobs=

[wasteland_bobs]
immovable_bobs=skeleton1,skeleton2,skeleton3,cactus1,cactus1,cactus2,cactus2,fallentree,deadtree1,deadtree2,deadtree3,deadtree4
moveable_bobs=

------------------------
--  Former blackland  --
------------------------


##### This info is needed by the random map generator #####

[heights]
ocean=10
shelf=10
shallow=10
coast=11
upperland=14
mountainfoot=15
mountain=16
snow=39
summit=45

[areas]
water=water
land=hardground1,hardground2,ashes
wasteland=wasteland1,wasteland2
mountains=mountains


[water]
ocean_terrains=water
shelf_terrains=water
shallow_terrains=water


[hardground1]
weight=2

coast_terrains=strand
land_terrains=hardground1,hardground2
upper_terrains=hardground1,hardground2


[hardground2]
weight=2

coast_terrains=strand
land_terrains=hardground3,hardground4
upper_terrains=hardground3,hardground4


[ashes]
weight=1

coast_terrains=ashes2
land_terrains=ashes
upper_terrains=ashes


[mountains]
mountainfoot_terrains=hardlava
mountain_terrains=mountain1,mountain2,mountain3,mountain4
snow_terrains=lava,lava-stone1,lava-stone2


[wasteland1]
weight=1

inner_terrains=lava,lava-stone1
outer_terrains=hardlava,lava-stone2


[wasteland2]
weight=2

inner_terrains=hardlava,lava-stone2
outer_terrains=hardlava,lava-stone2


#####################################
########## BOB definitions ##########
#####################################


# TODO: Check the existence of the bobs

# The postfix "_lr" means "land resource"

[bobs]
areas=forest_lr,bushland_lr,meadow_lr,stone_lr,sstone_lr
bob_kinds=tree_bobs,bush_bobs,grass_bobs,stone_bobs,sstone_bobs,wasteland_bobs

[forest_lr]
weight=6
immovable_density=40
moveable_density=20
land_coast_bobs=bush_bobs
land_inner_bobs=tree_bobs
land_upper_bobs=bush_bobs
wasteland_inner_bobs=wasteland_bobs
wasteland_outer_bobs=bush_bobs

[bushland_lr]
weight=2
immovable_density=50
moveable_density=20
land_coast_bobs=grass_bobs
land_inner_bobs=bush_bobs
land_upper_bobs=grass_bobs
wasteland_inner_bobs=wasteland_bobs
wasteland_outer_bobs=grass_bobs

[meadow_lr]
weight=2
immovable_density=10
moveable_density=20
land_coast_bobs=grass_bobs
land_inner_bobs=grass_bobs
land_upper_bobs=grass_bobs
wasteland_inner_bobs=wasteland_bobs
wasteland_outer_bobs=grass_bobs

[stone_lr]
weight=4
immovable_density=100
moveable_density=0
land_coast_bobs=
land_inner_bobs=stone_bobs
land_upper_bobs=
wasteland_inner_bobs=stone_bobs
wasteland_outer_bobs=stone_bobs

[sstone_lr]
weight=2
immovable_density=100
moveable_density=0
land_coast_bobs=
land_inner_bobs=sstone_bobs
land_upper_bobs=
wasteland_inner_bobs=sstone_bobs
wasteland_outer_bobs=sstone_bobs

[tree_bobs]
immovable_bobs=tree1,tree2,tree3,tree4,tree5,tree6,tree7,tree8
moveable_bobs=deer,reindeer

[bush_bobs]
immovable_bobs=bush1,bush2,bush3,bush4,bush5
moveable_bobs=bunny,fox,sheep,sheep

[grass_bobs]
immovable_bobs=grass1,grass2,grass3
moveable_bobs=bunny,fox,sheep,sheep

[sstone_bobs]
immovable_bobs=sstones1,sstones2,sstones3,sstones4,sstones5,sstones6,sstones7
moveable_bobs=

[stone_bobs]
immovable_bobs=stones1,stones2,stones3,stones4,stones5,stones5,stones6,stones6,stones6
moveable_bobs=

[wasteland_bobs]
immovable_bobs=skeleton1,skeleton2,skeleton3,cactus1,cactus1,cactus2,cactus2,deadtree1,deadtree2,deadtree3,deadtree4
moveable_bobs=

---------------------
--  Former desert  --
---------------------


##### This info is needed by the random map generator #####

[heights]
ocean=10
shelf=10
shallow=10
coast=11
upperland=14
mountainfoot=15
mountain=16
snow=35
summit=45

[areas]
water=water
land=meadow_land
wasteland=desert
mountains=mountains

[water]
ocean_terrains=wasser
shelf_terrains=wasser
shallow_terrains=wasser

[desert]
inner_terrains=desert1,desert2,desert3,desert4
outer_terrains=desert1,desert2,desert3,desert4

[mountains]
mountainfoot_terrains=mountainmeadow
mountain_terrains=mountain1,mountain2,mountain3,mountain4
snow_terrains=mountain1,mountain2,mountain3,mountain4

[meadow_land]
coast_terrains=beach
land_terrains=meadow
upper_terrains=mountainmeadow


#####################################
########## BOB definitions ##########
#####################################


# The postfix "_lr" means "land resource"

[bobs]
areas=forest_lr,bushland_lr,meadow_lr,stone_lr,sstone_lr
bob_kinds=tree_bobs,bush_bobs,grass_bobs,stone_bobs,sstone_bobs,wasteland_bobs

[forest_lr]
weight=6
immovable_density=40
moveable_density=20
land_coast_bobs=bush_bobs
land_inner_bobs=tree_bobs
land_upper_bobs=bush_bobs
wasteland_inner_bobs=wasteland_bobs
wasteland_outer_bobs=bush_bobs

[bushland_lr]
weight=2
immovable_density=50
moveable_density=20
land_coast_bobs=grass_bobs
land_inner_bobs=bush_bobs
land_upper_bobs=grass_bobs
wasteland_inner_bobs=wasteland_bobs
wasteland_outer_bobs=grass_bobs

[meadow_lr]
weight=2
immovable_density=10
moveable_density=20
land_coast_bobs=grass_bobs
land_inner_bobs=grass_bobs
land_upper_bobs=grass_bobs
wasteland_inner_bobs=wasteland_bobs
wasteland_outer_bobs=grass_bobs

[stone_lr]
weight=4
immovable_density=100
moveable_density=0
land_coast_bobs=
land_inner_bobs=stone_bobs
land_upper_bobs=
wasteland_inner_bobs=stone_bobs
wasteland_outer_bobs=stone_bobs

[sstone_lr]
weight=2
immovable_density=100
moveable_density=0
land_coast_bobs=
land_inner_bobs=sstone_bobs
land_upper_bobs=
wasteland_inner_bobs=sstone_bobs
wasteland_outer_bobs=sstone_bobs

[tree_bobs]
immovable_bobs=tree1,tree2,tree3,tree4,tree5,tree6,tree7,tree8
moveable_bobs=deer

[bush_bobs]
immovable_bobs=bush1,bush4,bush5
moveable_bobs=bunny,fox,sheep,sheep

[grass_bobs]
immovable_bobs=grass1,grass2,grass3,grass1,grass2,grass3,ruin1,ruin2,ruin3,ruin4,ruin5
moveable_bobs=bunny,fox,sheep,sheep

[sstone_bobs]
immovable_bobs=sstones1,sstones2,sstones3,sstones4,sstones5,sstones6,sstones7
moveable_bobs=

[stone_bobs]
immovable_bobs=stones1,stones2,stones3,stones4,stones5,stones5,stones6,stones6,stones6
moveable_bobs=

[wasteland_bobs]
immovable_bobs=skeleton1,skeleton2,skeleton3,cactus1,cactus1,cactus2,cactus2,deadtree1,deadtree2,deadtree3,deadtree4
moveable_bobs=

-------------------------
--  Former winterland  --
-------------------------


##### This info is needed by the random map generator #####

[heights]
ocean=10
shelf=10
shallow=10
coast=11
upperland=14
mountainfoot=15
mountain=16
snow=35
summit=45

[areas]
water=water,water_iceflows,water_ice
land=land
wasteland=wasteland
mountains=mountains

[water]
ocean_terrains=water
shelf_terrains=water
shallow_terrains=water

[water_iceflows]
ocean_terrains=ice_flows,ice_flows2
shelf_terrains=ice_flows,ice_flows2
shallow_terrains=ice_flows,ice_flows2

[water_ice]
ocean_terrains=ice
shelf_terrains=ice
shallow_terrains=ice

[land]
coast_terrains=strand
land_terrains=tundra,tundra2,tundra3
upper_terrains=tundra,tundra2,tundra3

[mountains]
mountainfoot_terrains=tundra_taiga
mountain_terrains=mountain1,mountain2,mountain3,mountain4
snow_terrains=snow

[wasteland]
inner_terrains=tundra_taiga
outer_terrains=tundra


#####################################
########## BOB definitions ##########
#####################################


# The postfix "_lr" means "land resource"

[bobs]
areas=forest_lr,bushland_lr,meadow_lr,stone_lr,sstone_lr
bob_kinds=tree_bobs,bush_bobs,grass_bobs,stone_bobs,sstone_bobs,wasteland_bobs

[forest_lr]
weight=6
immovable_density=100
moveable_density=20
land_coast_bobs=bush_bobs
land_inner_bobs=tree_bobs
land_upper_bobs=bush_bobs
wasteland_inner_bobs=wasteland_bobs
wasteland_outer_bobs=bush_bobs

[bushland_lr]
weight=2
immovable_density=50
moveable_density=20
land_coast_bobs=grass_bobs
land_inner_bobs=bush_bobs
land_upper_bobs=grass_bobs
wasteland_inner_bobs=wasteland_bobs
wasteland_outer_bobs=grass_bobs

[meadow_lr]
weight=2
immovable_density=10
moveable_density=20
land_coast_bobs=grass_bobs
land_inner_bobs=grass_bobs
land_upper_bobs=grass_bobs
wasteland_inner_bobs=wasteland_bobs
wasteland_outer_bobs=grass_bobs

[stone_lr]
weight=4
immovable_density=100
moveable_density=0
land_coast_bobs=
land_inner_bobs=stone_bobs
land_upper_bobs=
wasteland_inner_bobs=stone_bobs
wasteland_outer_bobs=stone_bobs

[sstone_lr]
weight=2
immovable_density=100
moveable_density=0
land_coast_bobs=
land_inner_bobs=sstone_bobs
land_upper_bobs=
wasteland_inner_bobs=sstone_bobs
wasteland_outer_bobs=sstone_bobs

[tree_bobs]
immovable_bobs=tree1,tree2,tree3,tree4,tree5,tree6,tree7,tree8
moveable_bobs=deer,elk,lynx,reindeer,wolf

[bush_bobs]
immovable_bobs=bush1,bush2,bush3,bush4,bush5
moveable_bobs=wisent,wolf,bunny,elk,fox,lynx,wolf,marten,sheep,sheep

[grass_bobs]
immovable_bobs=grass1,grass2,grass3
moveable_bobs=bunny,elk,fox,lynx,wolf,marten,sheep,sheep

[sstone_bobs]
immovable_bobs=sstones1,sstones2,sstones3,sstones4,sstones5,sstones6,sstones7
moveable_bobs=

[stone_bobs]
immovable_bobs=stones1,stones2,stones3,stones4,stones5,stones5,stones6,stones6,stones6
moveable_bobs=

[wasteland_bobs]
immovable_bobs=skeleton1,skeleton2,skeleton3,deadtree1,deadtree2,deadtree3,deadtree4
moveable_bobs=

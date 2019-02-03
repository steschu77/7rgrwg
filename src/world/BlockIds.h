#pragma once

// from
// "C:\Program Files (x86)\Steam\steamapps\common\7 Days To Die\Data\Config\blocks.xml"

enum BlockIds
{
  idAir = 0,
  idStone = 1,
  idGrass = 2,
  idDirt = 3,
  idPlainsGround = 4,
  idRadiated = 5,
  idPotassiumNitrate = 6,
  idIronOre = 7,
  idSand = 8,
  idSnow = 9,
  idClay = 10,
  idLeadOre = 11,
  idBedrock = 12,
  idSandStone = 13,
  idDesertGround = 14,
  idAsphalt = 15,
  idGravel = 16,
  // 17 ?
  idIce = 18,
  idFertileDirt = 19,
  idFertileGrass = 20,
  // idGore = 21,
  idFertileFarmland = 22,
  idSliverOre = 23,
  idBurntForestGround = 24,
  idForestGround = 25,
  idDestroyedStone = 26,
  idCoalOre = 27,
  idTerrainFiller = 28,
  idGoldOre = 29,
  idSod = 30,
  idHardSod = 31,
  idWoodDebris = 32,
  idOilDeposit = 33,
  idDiamondOre = 34,
  idFertilizedFarmland = 35,

  idGrassFromDirt = 37,
  idPlainsGroundFromDirt = 38,
  idPlainsGroundWGrass1 = 39,
  idPlainsGroundWGrass2 = 40,
  idBurntForestGroundFromDirt = 41,
  idBurntForestGroundWGrass1 = 42,
  idBurntForestGroundWGrass2 = 43,
  idForestGroundFromDirt = 44,
  idForestGroundWGrass1 = 45,
  idForestGroundWGrass2 = 46,
  idClayInSandstone = 47,
  idLootStone = 48,

  idPlantedBlueberry1 = 257,
  idPlantedBlueberry2 = 262,
  idPlantedBlueberry3Harvest = 258,
  idPlantedPotato1 = 259,
  idPlantedPotato2 = 260,
  idPlantedPotato3Harvest = 261,

  idLootWastelandHelper = 263,

  idPlantedHop1 = 527,
  idPlantedHop2 = 528,
  idPlantedHop3Harvest = 529,

  idPlantedMushroom1 = 530,
  idPlantedMushroom2 = 531,
  idMushroom3Harvest = 532,

  idPlantedYucca1 = 533,
  idPlantedYucca2 = 534,

  idTreeJuniper6m = 559,
  idTreeMountainPine8m = 560,
  idTreeMountainPine13m = 561,
  idTreeMountainPine16m = 562,
  idTreeMountainPine19m = 563,
  idTreeMountainPine20m = 564,
  idTreeMountainPineDry = 565,

  idTreeBirch10m = 566,
  idTreeBirch12m = 567,
  idTreeBirch15m = 568,

  idTreeDeadShrub = 569,

  idTreeMaple6m = 570,
  idTreeMaple13m = 571,
  idTreeMaple15m = 573,
  idTreeMaple16m = 574,
  idTreeMaple17m = 575,

  idTreeBurntPine01 = 577,
  idTreeBurntPine02 = 578,
  idTreeBurntPine03 = 579,

  idTreeDeadTree01 = 581,
  idTreeDeadTree02 = 582,

  idTreeWinterEverGreen = 586,
  idTreeWinterPine01 = 587,
  idTreeWinterPine02 = 589,
  idTreeWinterPine03 = 590,

  idCntTreeStump = 588,

  idTreeDesertShrub = 591,

  idRockResource02 = 640, // (3x2x3)
    // 10100a80 10100280 10104a80
    // 10000a80 00001280 10004a80
    // 10900a80 10900280 10904a80

    // 11100a80 11100280 11104a80
    // 11000a80 11000280 11004a80
    // 11900a80 11900280 11904a80

  idPlantedCotton1 = 713,
  idPlantedCotton3Harvest = 714,

  idPlantedCoffee1 = 759,
  idPlantedCoffee2 = 760,
  idPlantedCoffee3Harvest = 761,

  idPlantedGoldenrod1 = 762,
  idPlantedGoldenrod2 = 763,
  idPlantedGoldenrod3Harvest = 764,

  idWaterMovingBucket = 823,
  idWaterStaticBucket = 824,
  idWaterMoving = 825,
  idWater = 826,

  idPlantedAloe1 = 855,
  idPlantedAloe2 = 856,
  idSnowberry3Harvest = 857,

  idHayBaleBlock = 861,

  idTreePlainsTree2 = 863,

  idPlantedChrysanthemum1 = 869,
  idPlantedChrysanthemum2 = 870,

  idTreeDeadPineLeaf = 871,

  idPlantedCorn1 = 900,
  idPlantedCorn2 = 901,
  idPlantedCorn3Harvest = 902,
  idPlantedCornDead = 903,

  idTreePlantedGrass = 909,

  idTreeTallGrassDiagonal = 768,
  idTreeForestGrassDiagonal = 842, // 00x0034a, x=0,1,2,3,4,5

  idTreeAzalea = 1402,

  idRandomCarsHelper = 1452,
  idCntCar03SedanDamage0 = 1453,
  idCntCar03SedanDamage1 = 1454,
  idCntCar03SedanDamage2 = 1455, // wreck, no color
};

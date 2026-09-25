.pragma library

const spriteCreatures = [
    "minotaur",
    "minotaur_archer",
    "minotaur_guard",
    "minotaur_mage",
    "orc",
    "orc_spearman",
    "orc_warrior",
    "orc_shaman",
    "dwarf",
    "dwarf_geomancer",
    "dwarf_guard",
    "dwarf_soldier",
    "deer",
    "troll",
    "wolf"
]

function sheetIdentifier(creatureIdentifier) {
    return spriteCreatures.indexOf(creatureIdentifier) !== -1
        ? creatureIdentifier
        : "placeholder"
}

function sheetSource(creatureIdentifier) {
    const identifier = sheetIdentifier(creatureIdentifier)
    return "qrc:/assets/creatures/" + identifier + "/" + identifier + ".json"
}

function isPlaceholder(creatureIdentifier) {
    return sheetIdentifier(creatureIdentifier) === "placeholder"
}

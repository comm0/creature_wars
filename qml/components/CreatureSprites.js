.pragma library

const spriteCreatures = ["minotaur", "orc", "dwarf"]

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

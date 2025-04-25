'use strict';
const Items = require('@wfcd/items');
const relics = new Items({ category: ['Relics'] });
const fs = require('fs');
let relicjson = fs.readFileSync("relics.json");

var relicJSOBJ = JSON.parse(relicjson);
console.log(relicJSOBJ);

//relics.slice(0, 3).forEach((relic) => {
//    var relicObj = {
//        name: relic.name,
//        reward: relic.rewards,
//        count: 0
//    }

//    relicArray.push(relicObj);
//    relicjson = JSON.stringify(relicArray);
//    fs.writeFileSync("relics.json", relicjson);

//    //relic.rewards.forEach((reward) => {
//    //    console.log("\t", reward.item.name, "\tChance: ", `${reward.chance}%`);

//    //    })
//});

relics.forEach((relic) => {
    var curRelicName = relic.name;

    delete relic["category"]
    delete relic["description"]
    delete relic["masterable"]
    delete relic["marketInfo"]
    delete relic["type"]
    delete relic["name"]
    delete relic["uniqueName"]

    relic.count = 0;
    
    relic.rewards.forEach((reward) => {
        delete reward.item["uniqueName"]
        delete reward.item["warframeMarket"]
        if (reward.chance == 25.33) {
            reward.rarity = "Common";
            if (curRelicName.includes("Exceptional")) {
                reward.chance = 23.33;
            }
            else if (curRelicName.includes("Flawless")) {
                reward.chance = 20;
            }
            else if (curRelicName.includes("Radiant")) {
                reward.chance = 16.67;
            }
        }
        else if (reward.chance == 11) {
            if (curRelicName.includes("Exceptional")) {
                reward.chance = 13;
            }
            else if (curRelicName.includes("Flawless")) {
                reward.chance = 17;
            }
            else if (curRelicName.includes("Radiant")) {
                reward.chance = 20;
            }
        }
        else {
            reward.rarity = "Rare";
            if (curRelicName.includes("Exceptional")) {
                reward.chance = 4;
            }
            else if (curRelicName.includes("Flawless")) {
                reward.chance = 6;
            }
            else if (curRelicName.includes("Radiant")) {
                reward.chance = 10;
            }
        }
    })

    
    relicJSOBJ[curRelicName] = relic;
    
});

console.log(relicJSOBJ)
relicJSOBJ = JSON.stringify(relicJSOBJ);
fs.writeFileSync("relics.json", relicJSOBJ);

//relics.forEach((relic) => {
//    console.log(relic.name);
//    relic.rewards.forEach((reward) => {
//        console.log("\t", reward.item.name, "\tChance: ", `${reward.chance}%`);
//    })
//});
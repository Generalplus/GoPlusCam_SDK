"use strict";
#import "Includes.js"

//connect
var target = UIATarget.localTarget();
target.frontMostApp().mainWindow().buttons()["Connect"].tap();

var monkey = new UIAutoMonkey();
monkey.config.numberOfEvents = false;
monkey.config.delayBetweenEvents = 0.5;
monkey.config.minutesToRun = 10;
monkey.RELEASE_THE_MONKEY();

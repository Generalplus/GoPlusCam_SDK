
var target = UIATarget.localTarget();
UIATarget.localTarget().delay(2);
target.setDeviceOrientation(UIA_DEVICE_ORIENTATION_FACEUP);
UIATarget.localTarget().delay(2);
target.frontMostApp().mainWindow().buttons()["Connect"].tap();
UIATarget.localTarget().delay(2);
target.tap({x:163.00, y:155.00});
UIATarget.localTarget().delay(2);
target.frontMostApp().mainWindow().buttons()["File"].tap();
UIATarget.localTarget().delay(2);


for (var i = 0; i < 100 ; i++) {
    UIATarget.localTarget().delay(2);
    target.tap({x:197.00, y:240.50});
    UIATarget.localTarget().delay(2);
    target.frontMostApp().mainWindow().tableViews()[0].cells()["Play"].tap();
    UIATarget.localTarget().delay(5);
    target.frontMostApp().navigationBar().leftButton().tap();
}

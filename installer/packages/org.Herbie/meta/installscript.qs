/*****************************************************
 
 Welcome to the Herbie install script.
 
 Created on: 	23.11.2015
 Author:		Markus Diem
 
 *****************************************************/

function Component()
{
	installer.installationFinished.connect(this, Component.prototype.installationFinishedPageIsShown);
    installer.finishButtonClicked.connect(this, Component.prototype.installationFinished);

    installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, false);
	installer.setDefaultPageVisible(QInstaller.LicenseCheck, false);
	installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
}

Component.prototype.isDefault = function()
{
    // select the component by default
    return true;
}

Component.prototype.createOperations = function()
{
    try {
	
		// call the base create operations function
        component.createOperations();

		component.addOperation("CreateShortcut", "@TargetDir@/Herbie.exe",   "@StartMenuDir@/Herbie.lnk", "workingDirectory=@TargetDir@");
		
		// add to auto start
		component.addOperation("CreateShortcut", "@TargetDir@/Herbie.exe",   "@StartMenuDir@/../Startup/Herbie.lnk", "workingDirectory=@TargetDir@");
		
		console.log("added to startup ---------------------------");
		console.log("added to: @StartMenuDir@/../Startup/Herbie.lnk");

	} catch (e) {
        console.log(e);
    }
	
	console.log("operations created...");

}

Component.prototype.installationFinishedPageIsShown = function()
{
    try {
        if (installer.isInstaller() && installer.status == QInstaller.Success) {
            installer.addWizardPageItem( component, "OpenAppCheckBoxForm", QInstaller.InstallationFinished );
        }
		
    } catch(e) {
        console.log(e);
    }
}

Component.prototype.installationFinished = function()
{
    try {
        if (installer.isInstaller() && installer.status == QInstaller.Success) {
			
			// open app if needed
			var isOpenAppCheckBoxChecked = component.userInterface( "OpenAppCheckBoxForm" ).openAppCheckBox.checked;
            if (isOpenAppCheckBoxChecked) {
                QDesktopServices.openUrl("file:///" + installer.value("TargetDir") + "/Herbie.exe");
				console.log("Opening App...");
            }        
        }
		
		console.log("installation finished...");

    } catch(e) {
        console.log(e);
    }
}
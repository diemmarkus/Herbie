/*****************************************************
 
 Welcome to the Herbie install script.
 
 Created on: 	23.11.2015
 Author:		Markus Diem
 
 *****************************************************/

function Controller() {
	console.log("Running Controller script...");
}

Controller.prototype.IntroductionPageCallback = function() {

	var widget = gui.currentPageWidget();
    if (widget != null) {
        widget.MessageLabel.setText("Herbie is a light-weight diagnostics tool. It shows the current CPU and RAM usage on your desktop. Customizable colors allow you to perfectly integrate it with your wallpaper.<br>It is licensed under the GNU General Public License v3.");
    }
	else
		Console.log("Cannot load custom introduction...");
}

// skip installation finished
Controller.prototype.PerformInstallationPageCallback = function()
{
    installer.setAutomatedPageSwitchEnabled(true);
    gui.clickButton(buttons.NextButton);
}
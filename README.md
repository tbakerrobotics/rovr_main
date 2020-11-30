## ROVR Social VR Platform

This repository does not include the Vivox Plugin requried for audio communication. Please ensure you copy a version of this into the Plugins folder before building. 

If you experience build errors, follow the steps below:

1.Delete the following folders: Binaries, Build, Intermediate, Saved

2.Delete the Visual Studio or Xcode solution file (.sln or .xcodeproj)

3.Right click on the uproject file and click 'Generate Visual Studio Files'

4.Open the sln file and build the c++ project.

3.Re-open Unreal

# Tasks to complete:
- Add in-game menu
  - Client menu has leave session
  - Server menu has return to lobby session and switch ma
- Move vivox tokens over to production ready solution
- Remove Steam and integrate standard online sub systems (user specifying a specific IP address)  
- UI replace/improve  
- Allow users to upload own images
- Include "Are you sure you want to quit?" dialog when quit is clicked

# Once complete:

MIGRATE TO ANDROID
1. Add UI functionality (look at test project)
2. Ensure settings are correct for android (scalebale quality etc)
2. Work out save files and how they work on android  

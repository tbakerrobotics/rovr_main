## ROVR Social VR Platform

This repository does not include the Vivox Plugin requried for audio communication. Please ensure you copy a version of this into the Plugins folder before building. 

If you experience build errors, follow the steps below:

1.Delete the following folders: Binaries, Build, Intermediate, Saved

2.Delete the Visual Studio or Xcode solution file (.sln or .xcodeproj)

3.Right click on the uproject file and click 'Generate Visual Studio Files'

4.Open the sln file and build the c++ project.

3.Re-open Unreal

# Known Bugs
- If the server starts the session without anyone in, and then returns to lobby, 'ReturnToLobby' boolean will be true so joining users will not have the setup run properly.
- Returning to lobby does not make the already selected characters unavailable


# Tasks to complete
- Fix name plates in lobby. Game works but lobby doesn't
- Determine way to update ReturnToLobby boolean for everyone

# Migrate To Android
1. Remove Steam and integrate standard online sub systems (user specifying a specific IP address)   
2. Add UI functionality (look at test project)
3. Ensure settings are correct for android (scalebale quality etc)
4. Work out save files and how they work on android  

# Future Developments
- Move vivox tokens over to production ready solutionw
- Allow users to upload own images
- Add "connected players to vivox voice channel" on E press - In Progess - TB to finish
- Tweak vivox positional values for volume close and drop off after

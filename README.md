## ROVR Social VR Platform

Platform Functions on both Android & PC. If building for PC ensure that the Pico SDK are not enabled in the Plugins window, and you have removed the Pico Neo2 controller triggers as inputs under 'Grab Right'.

Unreal Version : 4.25.3\
Vivox SDK Version : 5.13.0.unr.1\
Pico SDK Version : v1.2.3

# January Reminder
- Return To Lobby Issue for Server (caused by on swap player controller event - firing too quickly one after another).
- Vivox Positional Audio - Check Fuctionality 
- Android Lobby UI 
- Android Movement 
- Android Volume Needs Increasing
- Android leave game not working - event end play not handled

# Known Bugs/Fixes Required
- If the server starts the session without anyone in, and then returns to lobby, 'ReturnToLobby' boolean will be true so joining users will not have the setup run properly.
- Returning to lobby does not make the already selected characters unavailable

# Tasks to complete
- Fix name plates in lobby. Game works but lobby doesn't
- Determine way to update ReturnToLobby boolean for everyone
- Internet network error handling

# Future Developments
- Move vivox tokens over to production ready solutionw
- Allow users to upload own images
- Add "connected players to vivox voice channel" on E press - In Progess - TB to finish
- Tweak vivox positional values for volume close and drop off after
- Vivox audio gain needs increasing

# If You Have Issues Building : 

This repository does not include the Vivox Plugin requried for audio communication. Please ensure you copy a version of this into the Plugins folder before building. 

If you experience build errors, follow the steps below:

1.Delete the following folders: Binaries, Build, Intermediate, Saved\
2.Delete the Visual Studio or Xcode solution file (.sln or .xcodeproj)\
3.Right click on the uproject file and click 'Generate Visual Studio Files'\
4.Open the sln file and build the c++ project.\
5.Re-open Unreal

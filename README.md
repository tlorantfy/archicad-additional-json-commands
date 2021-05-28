# Additional JSON/Python Commands Add-On for Archicad

This Add-On extends the JSON interface of Archicad by implementing new JSON commands.  
These JSON commands are **callable via Python**, see examples below.

**Requires Archicad 25 or later.**

# Implemented Commands

- [Publish](#publish)
- [GetProjectInfo](#getprojectinfo)
- [TeamworkReceive](#teamworkreceive)
- [GetArchicadLocation](#getarchicadlocation)
- [Quit](#quit)
- [ReloadLibraries](#reloadlibraries)

## Publish
Performs a publish operation on the currently opened project. Only the given publisher set will be published.
### Parameters
* publisherSetName (required)
  * Type: string
  * The name of the publisher set.
* outputPath
  * Type: string
  * Full local or LAN path for publishing. Optional, by default the path set in the settings of the publiser set will be used.
### Response
* errorMessage
  * Type: string
  * The error message upon error. If the command executed successfully, then there is no response.
### Python Example
```python
from archicad import ACConnection

conn = ACConnection.connect ()

acc = conn.commands
act = conn.types

parameters = { 'publisherSetName': publisherSetName }
acc.ExecuteAddOnCommand (act.AddOnCommandId ('AdditionalJSONCommands', 'Publish'), parameters)
```

## GetProjectInfo
Retrieves the location of the currently running Archicad executable.
### Parameters
### Response
* isUntitled (required)
  * Type: string
  * True, if the project is not saved yet.
* isTeamwork (required)
  * Type: string
  * True, if the project is a Teamwork (BIMcloud) project.
* projectLocation
  * Type: string
  * The location of the project in the filesystem or a BIMcloud project reference.
* projectPath
  * Type: string
  * The path of the project. A filesystem path or a BIMcloud server relative path.
* projectName
  * Type: string
  * The name of the project.
### Python Example
```python
from archicad import ACConnection

conn = ACConnection.connect ()

acc = conn.commands
act = conn.types

response = acc.ExecuteAddOnCommand (act.AddOnCommandId ('AdditionalJSONCommands', 'GetProjectInfo'))
isTeamwork = response['isTeamwork']
if not response['isUntitled']:
  projectLocation = response['projectLocation']
```

## TeamworkReceive
Performs a receive operation on the currently opened Teamwork (BIMcloud) project.
### Parameters
### Response
* errorMessage
  * Type: string
  * The error message upon error. If the command executed successfully, then there is no response.
### Python Example
```python
from archicad import ACConnection

conn = ACConnection.connect ()

acc = conn.commands
act = conn.types

acc.ExecuteAddOnCommand (act.AddOnCommandId ('AdditionalJSONCommands', 'TeamworkReceive'))
```

## GetArchicadLocation
Retrieves the location of the currently running Archicad executable.
### Parameters
### Response
* archicadLocation (required)
  * Type: string
  * The location of the Archicad executable in the filesystem.
### Python Example
```python
from archicad import ACConnection

conn = ACConnection.connect ()

acc = conn.commands
act = conn.types

response = acc.ExecuteAddOnCommand (act.AddOnCommandId ('AdditionalJSONCommands', 'GetArchicadLocation'))
archicadLocation = response['archicadLocation']
```

## Quit
Performs a quit operation on the currently running Archicad instance.
### Parameters
### Response
* errorMessage
  * Type: string
  * The error message upon error. If the command executed successfully, then there is no response.
### Python Example
```python
from archicad import ACConnection

conn = ACConnection.connect ()

acc = conn.commands
act = conn.types

acc.ExecuteAddOnCommand (act.AddOnCommandId ('AdditionalJSONCommands', 'TeamworkReceive'))
```

## ReloadLibraries
Resets and loads the active libraries.  
All the active libraries (included the BIMcloud and BIM Server Libraries too) will be reloaded.
### Parameters
### Response
* errorMessage
  * Type: string
  * The error message upon error. If the command executed successfully, then there is no response.
### Python Example
```python
from archicad import ACConnection

conn = ACConnection.connect ()

acc = conn.commands
act = conn.types

acc.ExecuteAddOnCommand (act.AddOnCommandId ('AdditionalJSONCommands', 'ReloadLibraries'))
```

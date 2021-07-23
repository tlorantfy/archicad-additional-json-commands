# Additional JSON/Python Commands Add-On for Archicad

This Add-On extends the JSON interface of Archicad by implementing new JSON commands.  
These JSON commands are **callable via Python**, see examples below.

* [Download the Add-On for Windows](https://github.com/tlorantfy/archicad-additional-json-commands/releases/download/25.3/archicad-additional-json-commands.apx)
* [Download the Add-On for macOS](https://github.com/tlorantfy/archicad-additional-json-commands/releases/download/25.3/archicad-additional-json-commands.bundle.zip)

**Requires Archicad 25 or later.**

# Implemented Commands

- [Publish](#publish)
- [GetProjectInfo](#getprojectinfo)
- [TeamworkReceive](#teamworkreceive)
- [GetArchicadLocation](#getarchicadlocation)
- [Quit](#quit)
- [ReloadLibraries](#reloadlibraries)
- [MoveElements](#moveelements)

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

## MoveElements
Moves elements with a given movement vector.
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

objects = acc.GetElementsByType ('Object')
elementsWithMoveVectors = [{'elementId': {'guid': str (object.elementId.guid)}, 'moveVector': {'x': 1.0, 'y': 1.0, 'z': 0.0}} for object in objects]

acc.ExecuteAddOnCommand (act.AddOnCommandId ('AdditionalJSONCommands', 'MoveElements'), {'elementsWithMoveVectors': elementsWithMoveVectors})
```

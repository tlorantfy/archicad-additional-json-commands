# Additional JSON/Python Commands Add-On for Archicad

This Add-On extends the JSON interface of Archicad by implementing new JSON commands.  
These JSON commands are **callable via Python**, see examples below.

* [Download the Add-On for Windows](https://github.com/tlorantfy/archicad-additional-json-commands/releases/download/26.1/archicad-additional-json-commands.apx)

**Requires Archicad 25 or later.**

# Implemented Commands

- [Publish](#publish)
- [GetProjectInfo](#getprojectinfo)
- [TeamworkReceive](#teamworkreceive)
- [GetArchicadLocation](#getarchicadlocation)
- [Quit](#quit)
- [ReloadLibraries](#reloadlibraries)
- [MoveElements](#moveelements)
- [CreateColumns](#createcolumns)
- [GetHotlinks](#gethotlinks)
- [GetGDLParametersOfElements](#getgdlparametersofelements)
- [ChangeGDLParametersOfElements](#changegdlparametersofelements)

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
Reloads the libraries of the current Archicad project.
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

## MoveElements
Moves elements with a given movement vector.
### Parameters
* elementsWithMoveVectors (required)
  * Type: array
  * Items:
    * Type: object
    * Fields:
      * elementId (required)
        * Type: Element identifier object (guid field)
      * moveVector (required)
        * Type: 3D vector object (x, y, z fields).
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

## CreateColumns
Creates columns. The given coordinates will be origos of the columns.
### Parameters
* coordinates (required)
  * Type: array of x,y values
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

origosOfNewColumns = [{'x': 1.0, 'y': 1.0}, {'x': 5.0, 'y': 5.0}]

acc.ExecuteAddOnCommand (act.AddOnCommandId ('AdditionalJSONCommands', 'CreateColumns'), {'coordinates': origosOfNewColumns})
```

## GetHotlinks
Get the file system locations (path) of the hotlink modules. The hotlinks can have tree hierarchy in the project.
### Response
* hotlinks (required)
  * Type: array
  * List of hotlinks with locations and the children in the tree hierarchy.
### Python Example
```python
from archicad import ACConnection

conn = ACConnection.connect ()

acc = conn.commands
act = conn.types

print (acc.ExecuteAddOnCommand (act.AddOnCommandId ('AdditionalJSONCommands', 'GetHotlinks')))
```

## GetGDLParametersOfElements
Get all the GDL parameters (name, type, value) of the given elements.
### Parameters
* elements (required)
  * Type: array
  * Items:
    * Type: object
    * Fields:
      * elementId (required)
        * Type: Element identifier object (guid field)
### Response
* gdlParametersOfElements (required)
  * Type: array of GDL parameters dictionary.
### Python Example
```python
from archicad import ACConnection

conn = ACConnection.connect ()

acc = conn.commands
act = conn.types

elements = [ { 'elementId' : { 'guid' : str (e.elementId.guid) } } for e in acc.GetElementsByType ('Object') ]

print (acc.ExecuteAddOnCommand (act.AddOnCommandId ('AdditionalJSONCommands', 'GetGDLParametersOfElements', { 'elements' : elements })))
```

## ChangeGDLParametersOfElements
Changes the given GDL parameters of the given elements.
### Parameters
* elementsWithGDLParameters (required)
  * Type: array
  * Items:
    * Type: object
    * Fields:
      * elementId (required)
        * Type: Element identifier object (guid field)
      * gdlParameters (required)
        * Type: The dictionary of GDL parameters with the new values.
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

elementsWithGDLParameters = [ { 'elementId' : { 'guid' : str (e.elementId.guid) }, 'gdlParameters' : { 'gs_cont_pen' : 95 } } for e in acc.GetElementsByType ('Object') ]

print (acc.ExecuteAddOnCommand (act.AddOnCommandId ('AdditionalJSONCommands', 'ChangeGDLParametersOfElements', { 'elementsWithGDLParameters' : elementsWithGDLParameters })))
```
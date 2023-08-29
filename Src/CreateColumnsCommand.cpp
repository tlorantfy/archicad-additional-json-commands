#include	"CreateColumnsCommand.hpp"
#include	"ObjectState.hpp"
#include	"OnExit.hpp"


GS::String CreateColumnsCommand::GetName () const
{
	return "CreateColumns";
}


constexpr const char* CoordinatesParameterField = "coordinates";


GS::Optional<GS::UniString> CreateColumnsCommand::GetInputParametersSchema () const
{
	return GS::UniString::Printf (R"({
	"type": "object",
	"properties": {
		"%s": {
			"type": "array",
			"description": "The 3D coordinates of the new columns' origos.",
			"items": {
				"type": "object",
				"description" : "3D coordinate.",
				"properties" : {
					"x": {
						"type": "number",
						"description" : "X value of the coordinate."
					},
					"y" : {
						"type": "number",
						"description" : "Y value of the coordinate."
					},
					"z" : {
						"type": "number",
						"description" : "Z value of the coordinate."
					}
				},
				"additionalProperties": false,
				"required" : [
					"x",
					"y",
					"z"
				]
			}
		}
	},
	"additionalProperties": false,
	"required": [
	"%s"
	]
})",
CoordinatesParameterField,
CoordinatesParameterField);
}


static API_Coord3D GetCoordinateFromObjectState (const GS::ObjectState& objectState)
{
	API_Coord3D coordinate = {};
	objectState.Get ("x", coordinate.x);
	objectState.Get ("y", coordinate.y);
	objectState.Get ("z", coordinate.z);
	return coordinate;
}


GS::ObjectState	CreateColumnsCommand::Execute (const GS::ObjectState& parameters, GS::ProcessControl& /*processControl*/) const
{
	GS::Array<GS::ObjectState> coordinates;
	parameters.Get (CoordinatesParameterField, coordinates);

	API_Coord3D apiCoordinate = {};
	const APIErrCodes err = (APIErrCodes) ACAPI_CallUndoableCommand ("Create Columns", [&] () -> GSErrCode {
		API_Element element = {};
		API_ElementMemo memo = {};
		const GS::OnExit guard ([&memo] () { ACAPI_DisposeElemMemoHdls (&memo); });

#ifdef ServerMainVers_2600
		element.header.type   = API_ColumnID;
#else
		element.header.typeID = API_ColumnID;
#endif
		APIErrCodes err = (APIErrCodes) ACAPI_Element_GetDefaults (&element, &memo);

		const GS::Array<GS::Pair<short, double>> storyLevels = Utilities::GetStoryLevels ();

		for (const GS::ObjectState& coordinate : coordinates) {
			apiCoordinate = GetCoordinateFromObjectState (coordinate);

			element.header.floorInd = Utilities::GetFloorIndexAndOffset (apiCoordinate.z, storyLevels, element.column.bottomOffset);
			element.column.origoPos.x = apiCoordinate.x;
			element.column.origoPos.y = apiCoordinate.y;
			err = (APIErrCodes) ACAPI_Element_Create (&element, &memo);

			if (err != NoError) {
				return err;
			}
		}

		return NoError;
	});

	if (err != NoError) {
		const GS::UniString errorMsg = GS::UniString::Printf ("Failed to create column to coordinate: {%.2f, %.2f, %.2f}!", apiCoordinate.x, apiCoordinate.y, apiCoordinate.z);
		return CreateErrorResponse (err, errorMsg);
	}

	return {};
}
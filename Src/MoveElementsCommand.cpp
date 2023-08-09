#include	"MoveElementsCommand.hpp"
#include	"ObjectState.hpp"


GS::String MoveElementsCommand::GetName () const
{
	return "MoveElements";
}


constexpr const char* ElementsWithMoveVectorsParameterField = "elementsWithMoveVectors";
constexpr const char* MoveVectorField = "moveVector";
constexpr const char* CopyField = "copy";


GS::Optional<GS::UniString> MoveElementsCommand::GetInputParametersSchema () const
{
	return GS::UniString::Printf (R"({
	"type": "object",
	"properties": {
		"%s": {
			"type": "array",
			"description": "The elements with move vector pairs.",
			"items": {
				"type": "object",
				"properties": {
					"%s": {
						"$ref": "APITypes.json#/definitions/ElementId"
					},
					"%s": {
						"type": "object",
						"description" : "Move vector of a 3D point.",
						"properties" : {
							"x": {
								"type": "number",
								"description" : "X value of the vector."
							},
							"y" : {
								"type": "number",
								"description" : "Y value of the vector."
							},
							"z" : {
								"type": "number",
								"description" : "Z value of the vector."
							}
						},
						"additionalProperties": false,
						"required" : [
							"x",
							"y",
							"z"
						]
					},
					"%s": {
						"type": "boolean",
						"description" : "Optional parameter. If true, then a copy of the element will be moved. By default it's false."
					}
				},
				"additionalProperties": false,
				"required": [
					"%s",
					"%s"
				]
			}
		}
	},
	"additionalProperties": false,
	"required": [
	"%s"
	]
})",
ElementsWithMoveVectorsParameterField,
ElementIdField,
MoveVectorField,
CopyField,
ElementIdField, MoveVectorField,
ElementsWithMoveVectorsParameterField);
}


static API_Vector3D GetVector3DFromMoveVectorField (const GS::ObjectState& os)
{
	API_Vector3D v;
	os.Get ("x", v.x);
	os.Get ("y", v.y);
	os.Get ("z", v.z);
	return v;
}


static GSErrCode MoveElement (const API_Guid& elemGuid, const API_Vector3D& moveVector, bool withCopy)
{
	GS::Array<API_Neig> elementsToEdit = { API_Neig (elemGuid) };

	API_EditPars editPars = {};
	editPars.typeID = APIEdit_Drag;
	editPars.endC = moveVector;
	editPars.withDelete = !withCopy;

	return ACAPI_Element_Edit (&elementsToEdit, editPars);
}


GS::ObjectState	MoveElementsCommand::Execute (const GS::ObjectState& parameters, GS::ProcessControl& /*processControl*/) const
{
	GS::Array<GS::ObjectState> elementsWithMoveVectors;
	parameters.Get (ElementsWithMoveVectorsParameterField, elementsWithMoveVectors);

	API_Guid elemGuid;
	const APIErrCodes err = (APIErrCodes) ACAPI_CallUndoableCommand ("Move Elements", [&] () -> GSErrCode {
		for (const GS::ObjectState& elementWithMoveVector : elementsWithMoveVectors) {
			const GS::ObjectState* elementId = elementWithMoveVector.Get (ElementIdField);
			const GS::ObjectState* moveVector = elementWithMoveVector.Get (MoveVectorField);
			if (elementId == nullptr || moveVector == nullptr) {
				continue;
			}

			elemGuid = GetGuidFromElementIdField (*elementId);

			bool copy = false;
			elementWithMoveVector.Get (CopyField, copy);

			const GSErrCode err = MoveElement (elemGuid,
											   GetVector3DFromMoveVectorField (*moveVector),
											   copy);
			if (err != NoError) {
				return err;
			}
		}

		return NoError;
	});

	if (err != NoError) {
		const GS::UniString errorMsg = GS::UniString::Printf ("Failed to move element with guid %T!", APIGuidToString (elemGuid).ToPrintf ());
		return CreateErrorResponse (err, errorMsg);
	}

	return {};
}
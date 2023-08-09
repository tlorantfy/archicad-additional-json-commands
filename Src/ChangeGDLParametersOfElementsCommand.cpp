#include	"ChangeGDLParametersOfElementsCommand.hpp"
#include	"ObjectState.hpp"


GS::String ChangeGDLParametersOfElementsCommand::GetName () const
{
	return "ChangeGDLParametersOfElements";
}


constexpr const char* GDLParametersDictionarySchemaName = "GDLParametersDictionary";
constexpr const char* GDLParameterDetailsSchemaName = "GDLParameterDetails";
constexpr const char* ParameterTypeFieldName = "type";
constexpr const char* ParameterIndex1FieldName = "index1";
constexpr const char* ParameterIndex2FieldName = "index2";
constexpr const char* ParameterValueFieldName = "value";
constexpr const char* AttributeIndexFieldName = "index";
constexpr const char* AttributeNameFieldName = "name";


GS::Optional<GS::UniString> ChangeGDLParametersOfElementsCommand::GetSchemaDefinitions () const
{
	return GS::UniString::Printf (R"({
    "%s": {
      "type": "object",
      "description": "The dictionary of GDL parameters. The name of the parameter is the key and the details of the parameter are in the value."
    },
    "%s": {
      "type": "object",
      "description": "Details of GDL parameter with value.",
      "properties": {
        "%s": {
          "type": "string",
          "description": "The 1st index of array (in case of array value)."
        },
        "%s": {
          "type": "string",
          "description": "The 2nd index of array (in case of array value)."
        }
      },
      "additionalProperties": true,
      "required": [
      ]
    }
})",
GDLParametersDictionarySchemaName,
GDLParameterDetailsSchemaName,
ParameterIndex1FieldName,
ParameterIndex2FieldName
);
}


constexpr const char* ElementsWithGDLParametersDictionaryParameterField = "elementsWithGDLParameters";
constexpr const char* GDLParametersDictionaryField = "gdlParameters";


GS::Optional<GS::UniString> ChangeGDLParametersOfElementsCommand::GetInputParametersSchema () const
{
	return GS::UniString::Printf (R"({
	"type": "object",
	"properties": {
		"%s": {
			"type": "array",
			"description": "The elements with GDL parameters dictionary pairs.",
			"items": {
				"type": "object",
				"properties": {
					"%s": {
						"$ref": "APITypes.json#/definitions/ElementId"
					},
					"%s": {
						"type": "object",
						"description": "The dictionary of GDL parameters. The name of the parameter is the key and the details of the parameter are in the value."
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
ElementsWithGDLParametersDictionaryParameterField,
ElementIdField,
GDLParametersDictionaryField,
ElementIdField,
GDLParametersDictionaryField,
ElementsWithGDLParametersDictionaryParameterField
);
}


static void SetValueInteger (API_ChangeParamType& changeParam,
							 const GS::ObjectState&	parameterDetails)
{
	Int32 value;
	parameterDetails.Get (ParameterValueFieldName, value);
	changeParam.realValue = value;
}


static void SetValueDouble (API_ChangeParamType& changeParam,
							const GS::ObjectState&	parameterDetails)
{
	double value;
	parameterDetails.Get (ParameterValueFieldName, value);
	changeParam.realValue = value;
}


static void SetValueOnOff (API_ChangeParamType& changeParam,
							   const GS::ObjectState&	parameterDetails)
{
	GS::String value;
	parameterDetails.Get (ParameterValueFieldName, value);
	changeParam.realValue = (value == "Off" ? 0 : 1);
}
 

static void SetValueBool (API_ChangeParamType& changeParam,
							   const GS::ObjectState&	parameterDetails)
{
	bool value;
	parameterDetails.Get (ParameterValueFieldName, value);
	changeParam.realValue = (value ? 0 : 1);
}


static void SetValueString (API_ChangeParamType& changeParam,
							const GS::ObjectState&	parameterDetails)
{
	GS::UniString value;
	parameterDetails.Get (ParameterValueFieldName, value);

	constexpr USize MaxStrValueLength = 512;

	static GS::uchar_t strValuePtr[MaxStrValueLength];
	GS::ucscpy (strValuePtr, value.ToUStr (0, GS::Min(value.GetLength (), MaxStrValueLength)).Get ());

	changeParam.uStrValue = strValuePtr;
}


GS::ObjectState	ChangeGDLParametersOfElementsCommand::Execute (const GS::ObjectState& parameters, GS::ProcessControl& /*processControl*/) const
{
	GS::Array<GS::ObjectState> elementsWithGDLParameters;
	parameters.Get (ElementsWithGDLParametersDictionaryParameterField, elementsWithGDLParameters);

	bool invalidParameter = false;
	bool notAbleToChangeParameter = false;
	GS::String badParameterName;

	API_Guid elemGuid;
	const APIErrCodes err = (APIErrCodes) ACAPI_CallUndoableCommand ("Change GDL Parameters of Elements", [&] () -> GSErrCode {
		GSErrCode err = NoError;
		for (const GS::ObjectState& elementWithGDLParameters : elementsWithGDLParameters) {
			const GS::ObjectState* elementId = elementWithGDLParameters.Get (ElementIdField);
			if (elementId == nullptr) {
				continue;
			}

			const GS::ObjectState* gdlParameters = elementWithGDLParameters.Get (GDLParametersDictionaryField);
			if (gdlParameters == nullptr) {
				continue;
			}

			elemGuid = GetGuidFromElementIdField (*elementId);

			API_ParamOwnerType   paramOwner = {};

			paramOwner.libInd = 0;
			paramOwner.type   = API_ObjectID;
			paramOwner.guid   = elemGuid;

			err = ACAPI_Goodies (APIAny_OpenParametersID, &paramOwner);
			if (err == NoError) {
				API_GetParamsType getParams = {};
				err = ACAPI_Goodies (APIAny_GetActParametersID, &getParams);
				if (err == NoError) {
					const GSSize nParams = BMGetHandleSize ((GSHandle) getParams.params) / sizeof (API_AddParType);
					GS::HashTable<GS::String, API_AddParID> gdlParametersTypeDictionary;
					for (GSIndex ii = 0; ii < nParams; ++ii) {
						const API_AddParType&	actParam = (*getParams.params)[ii];

						if (actParam.typeID != APIParT_Separator) {
							gdlParametersTypeDictionary.Add (GS::String (actParam.name), actParam.typeID);
						}
					}

					GS::HashTable<GS::String, const GS::ObjectState*> changeParamsDictionary;
					gdlParameters->EnumerateFields ([&] (const GS::String& parameterName) {
						changeParamsDictionary.Add (parameterName, gdlParameters->Get (parameterName));
					});

					API_ChangeParamType changeParam = {};
					for (const auto& kv : changeParamsDictionary) {
						const GS::String& parameterName = *kv.key;
						const GS::ObjectState&	parameterDetails = **kv.value;


						if (!gdlParametersTypeDictionary.ContainsKey (parameterName)) {
							invalidParameter = true;
							badParameterName = parameterName;
							return APIERR_BADPARS;
						}

						CHTruncate (parameterName.ToCStr (), changeParam.name, sizeof (changeParam.name));
						if (parameterDetails.Contains (ParameterIndex1FieldName)) {
							parameterDetails.Get (ParameterIndex1FieldName, changeParam.ind1);
							if (parameterDetails.Contains (ParameterIndex2FieldName)) {
								parameterDetails.Get (ParameterIndex2FieldName, changeParam.ind2);
							}
						}

						switch (gdlParametersTypeDictionary[parameterName]) {
							case APIParT_Integer:
							case APIParT_PenCol:				SetValueInteger (changeParam, parameterDetails);	break;
							case APIParT_ColRGB:
							case APIParT_Intens:
							case APIParT_Length:
							case APIParT_RealNum:
							case APIParT_Angle:					SetValueDouble (changeParam, parameterDetails);		break;
							case APIParT_LightSw:				SetValueOnOff (changeParam, parameterDetails); 		break;
							case APIParT_Boolean: 				SetValueBool (changeParam, parameterDetails);		break;
							case APIParT_LineTyp:
							case APIParT_Mater:
							case APIParT_FillPat:
							case APIParT_BuildingMaterial:
							case APIParT_Profile: 				SetValueInteger (changeParam, parameterDetails);	break;
							case APIParT_CString:
							case APIParT_Title: 				SetValueString (changeParam, parameterDetails);		break;
							default:
							case APIParT_Dictionary:
								// Not supported by the Archicad API yet
								break;
						}

						err = ACAPI_Goodies (APIAny_ChangeAParameterID, &changeParam);
						if (err != NoError) {
							notAbleToChangeParameter = true;
							badParameterName = parameterName;
							return APIERR_BADPARS;
						}

						ACAPI_DisposeAddParHdl (&getParams.params);
						ACAPI_Goodies (APIAny_GetActParametersID, &getParams);
					}

					API_Element	element = {};
					element.header.guid = elemGuid;

					err = ACAPI_Element_Get (&element);
					if (err == NoError) {
						API_Element 	mask = {};
						API_ElementMemo memo = {};

						ACAPI_ELEMENT_MASK_CLEAR (mask);

						switch (element.header.type.typeID) {
							case API_ObjectID:
								element.object.xRatio = getParams.a;
								element.object.yRatio = getParams.b;
								ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, xRatio);
								ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, yRatio);
								break;
							case API_WindowID:
							case API_DoorID:
								element.window.openingBase.width  = getParams.a;
								element.window.openingBase.height = getParams.b;
								ACAPI_ELEMENT_MASK_SET (mask, API_WindowType, openingBase.width);
								ACAPI_ELEMENT_MASK_SET (mask, API_WindowType, openingBase.height);
								break;
							case API_SkylightID:
								element.skylight.openingBase.width  = getParams.a;
								element.skylight.openingBase.height = getParams.b;
								ACAPI_ELEMENT_MASK_SET (mask, API_SkylightType, openingBase.width);
								ACAPI_ELEMENT_MASK_SET (mask, API_SkylightType, openingBase.height);
								break;
							default:
								// Not supported yet
								break;
						}

						memo.params = getParams.params;
						err = ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_AddPars, true);
					}
				}

				ACAPI_Goodies (APIAny_CloseParametersID);
				ACAPI_DisposeAddParHdl (&getParams.params);
			}
		}

		return err;
	});

	if (err != NoError) {
		GS::UniString errorMsg;
		if (invalidParameter) {
			errorMsg = GS::UniString::Printf ("Invalid input: %s is not a GDL parameter of element %T", badParameterName.ToCStr (), APIGuidToString (elemGuid).ToPrintf ());
		} else if (notAbleToChangeParameter) {
			errorMsg = GS::UniString::Printf ("Failed to change parameter %s of element with guid %T", badParameterName.ToCStr (), APIGuidToString (elemGuid).ToPrintf ());
		}

		errorMsg = GS::UniString::Printf ("Failed to change parameters of element with guid %T", APIGuidToString (elemGuid).ToPrintf ());
		return CreateErrorResponse ((APIErrCodes) err, errorMsg);
	}

	return {};
}
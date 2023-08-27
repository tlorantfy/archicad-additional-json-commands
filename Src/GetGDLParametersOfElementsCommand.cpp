#include	"GetGDLParametersOfElementsCommand.hpp"
#include	"ObjectState.hpp"


GS::String GetGDLParametersOfElementsCommand::GetName () const
{
	return "GetGDLParametersOfElements";
}


constexpr const char* GDLParametersDictionarySchemaName = "GDLParametersDictionary";
constexpr const char* GDLParameterDetailsSchemaName = "GDLParameterDetails";
constexpr const char* ParameterIndexFieldName = "index";
constexpr const char* ParameterTypeFieldName = "type";
constexpr const char* ParameterDimension1FieldName = "dimension1";
constexpr const char* ParameterDimension2FieldName = "dimension2";
constexpr const char* ParameterValueFieldName = "value";
constexpr const char* AttributeIndexFieldName = "index";
constexpr const char* AttributeNameFieldName = "name";


GS::Optional<GS::UniString> GetGDLParametersOfElementsCommand::GetSchemaDefinitions () const
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
          "description": "The index of the parameter."
        },
        "%s": {
          "type": "string",
          "description": "The type of the parameter."
        },
        "%s": {
          "type": "string",
          "description": "The 1st dimension of array (in case of array value)."
        },
        "%s": {
          "type": "string",
          "description": "The 2nd dimension of array (in case of array value)."
        }
      },
      "additionalProperties": true,
      "required": [
        "%s",
        "%s"
      ]
    }
})",
GDLParametersDictionarySchemaName,
GDLParameterDetailsSchemaName,
ParameterIndexFieldName,
ParameterTypeFieldName,
ParameterDimension1FieldName,
ParameterDimension2FieldName,
ParameterIndexFieldName,
ParameterTypeFieldName
);
}


constexpr const char* ElementsParameterField = "elements";


GS::Optional<GS::UniString> GetGDLParametersOfElementsCommand::GetInputParametersSchema () const
{
	return GS::UniString::Printf (R"({
	"type": "object",
	"properties": {
		"%s": {
			"$ref": "APITypes.json#/definitions/Elements"
		}
	},
	"additionalProperties": false,
	"required": [
	"%s"
	]
})",
ElementsParameterField,
ElementsParameterField
);
}


constexpr const char* GDLParametersOfElementsResponseFieldName = "gdlParametersOfElements";


GS::Optional<GS::UniString> GetGDLParametersOfElementsCommand::GetResponseSchema () const
{
	return GS::UniString::Printf (R"({
	"type": "object",
	"properties": {
		"%s": {
			"type": "array",
			"description": "The GDL parameters of elements.",
			"items": {
				"$ref": "#/%s"
			}
		}
	},
	"additionalProperties": false,
	"required": [
		"%s"
	]
})",
GDLParametersOfElementsResponseFieldName,
GDLParametersDictionarySchemaName,
GDLParametersOfElementsResponseFieldName);
}


static GS::UniString ConvertAddParIDToString (API_AddParID addParID)
{
	switch (addParID) {
		case APIParT_Integer:			return "Integer";
		case APIParT_Length:			return "Length";
		case APIParT_Angle:				return "Angle";
		case APIParT_RealNum:			return "RealNum";
		case APIParT_LightSw:			return "LightSwitch";
		case APIParT_ColRGB:			return "ColorRGB";
		case APIParT_Intens:			return "Intens";
		case APIParT_LineTyp:			return "LineType";
		case APIParT_Mater:				return "Material";
		case APIParT_FillPat:			return "FillPattern";
		case APIParT_PenCol:			return "PenCol";
		case APIParT_CString:			return "String";
		case APIParT_Boolean:			return "Boolean";
		case APIParT_Separator:			return "Separator";
		case APIParT_Title:				return "Title";
		case APIParT_BuildingMaterial:	return "BuildingMaterial";
		case APIParT_Profile:			return "Profile";
		case APIParT_Dictionary:		return "Dictionary";
		default:						return "UNKNOWN";
	}
}


static API_AttrTypeID ConvertAddParIDToAttrTypeID (API_AddParID addParID)
{
	switch (addParID) {
		case APIParT_FillPat:			return API_FilltypeID;
		case APIParT_Mater:				return API_MaterialID;
		case APIParT_BuildingMaterial:	return API_BuildingMaterialID;
		case APIParT_Profile:			return API_ProfileID;
		case APIParT_LineTyp:			return API_LinetypeID;
		default:						return API_ZombieAttrID;
	}
}


static GS::UniString GetAttributeName (API_AttrTypeID	  typeID,
							 		   API_AttributeIndex index)
{
	API_Attribute	attrib = {};

	GS::UniString name;
	attrib.header.typeID = typeID;
	attrib.header.index = index;
	attrib.header.uniStringNamePtr = &name;

	ACAPI_Attribute_Get (&attrib);

	if (typeID == API_MaterialID && attrib.material.texture.fileLoc != nullptr) {
		delete attrib.material.texture.fileLoc;
		attrib.material.texture.fileLoc = nullptr;
	}

	return name;
}


static GS::ObjectState GetAttributeObjectState (API_AttrTypeID	   typeID,
							 		   			API_AttributeIndex index)
{
	GS::ObjectState attribute;
	attribute.Add (AttributeIndexFieldName, index);
	attribute.Add (AttributeNameFieldName, GetAttributeName (typeID, index));
	return attribute;
}


static void AddValueInteger (GS::ObjectState& 	   gdlParameterDetails,
							 const API_AddParType& actParam)
{
	if (actParam.typeMod == API_ParSimple) {
		gdlParameterDetails.Add (ParameterValueFieldName, static_cast<Int32> (actParam.value.real));
	} else {
		const auto& arrayValueItemAdder = gdlParameterDetails.AddList<Int32> (ParameterValueFieldName);
		Int32 arrayIndex = 0;
		for (Int32 i1 = 1; i1 <= actParam.dim1; i1++) {
			for (Int32 i2 = 1; i2 <= actParam.dim2; i2++) {
				arrayValueItemAdder (static_cast<Int32> (((double*)*actParam.value.array) [arrayIndex++]));
			}
		}
	}
}


static void AddValueDouble (GS::ObjectState& 	  gdlParameterDetails,
							const API_AddParType& actParam)
{
	if (actParam.typeMod == API_ParSimple) {
		gdlParameterDetails.Add (ParameterValueFieldName, actParam.value.real);
	} else {
		const auto& arrayValueItemAdder = gdlParameterDetails.AddList<double> (ParameterValueFieldName);
		Int32 arrayIndex = 0;
		for (Int32 i1 = 1; i1 <= actParam.dim1; i1++) {
			for (Int32 i2 = 1; i2 <= actParam.dim2; i2++) {
				arrayValueItemAdder (((double*)*actParam.value.array) [arrayIndex++]);
			}
		}
	}
}


static void AddValueAttribute (GS::ObjectState& 	 gdlParameterDetails,
							   const API_AddParType& actParam)
{
	if (actParam.typeMod == API_ParSimple) {
		gdlParameterDetails.Add (ParameterValueFieldName,
								 GetAttributeObjectState (ConvertAddParIDToAttrTypeID (actParam.typeID),
														  static_cast<API_AttributeIndex> (actParam.value.real)));
	} else {
		const auto& arrayValueItemAdder = gdlParameterDetails.AddList<GS::ObjectState> (ParameterValueFieldName);
		Int32 arrayIndex = 0;
		for (Int32 i1 = 1; i1 <= actParam.dim1; i1++) {
			for (Int32 i2 = 1; i2 <= actParam.dim2; i2++) {
				arrayValueItemAdder (GetAttributeObjectState (ConvertAddParIDToAttrTypeID (actParam.typeID),
														      static_cast<API_AttributeIndex> (((double*)*actParam.value.array) [arrayIndex++])));
			}
		}
	}
}

template<typename T>
static void AddValueTrueFalseOptions (GS::ObjectState& 	 gdlParameterDetails,
							   const API_AddParType& actParam,
											   T  optionTrue,
											   T  optionFalse)
{
	if (actParam.typeMod == API_ParSimple) {
		gdlParameterDetails.Add (ParameterValueFieldName, static_cast<Int32> (actParam.value.real) == 0 ? optionFalse : optionTrue);
	} else {
		const auto& arrayValueItemAdder = gdlParameterDetails.AddList<T> (ParameterValueFieldName);
		Int32 arrayIndex = 0;
		for (Int32 i1 = 1; i1 <= actParam.dim1; i1++) {
			for (Int32 i2 = 1; i2 <= actParam.dim2; i2++) {
				arrayValueItemAdder (static_cast<Int32> (((double*)*actParam.value.array) [arrayIndex++]) == 0 ? optionFalse : optionTrue);
			}
		}
	}
}


static void AddValueOnOff (GS::ObjectState& 	 gdlParameterDetails,
							   const API_AddParType& actParam)
{
	AddValueTrueFalseOptions (gdlParameterDetails, actParam, GS::String ("On"), GS::String ("Off"));
}
 

static void AddValueBool (GS::ObjectState& 	 gdlParameterDetails,
							   const API_AddParType& actParam)
{
	AddValueTrueFalseOptions (gdlParameterDetails, actParam, true, false);
}


static void AddValueString (GS::ObjectState& 	 gdlParameterDetails,
							const API_AddParType& actParam)
{
	if (actParam.typeMod == API_ParSimple) {
		gdlParameterDetails.Add (ParameterValueFieldName, GS::UniString (actParam.value.uStr));
	} else {
		const auto& arrayValueItemAdder = gdlParameterDetails.AddList<GS::UniString> (ParameterValueFieldName);
		Int32 arrayIndex = 0;
		for (Int32 i1 = 1; i1 <= actParam.dim1; i1++) {
			for (Int32 i2 = 1; i2 <= actParam.dim2; i2++) {
				GS::uchar_t* uValueStr = (reinterpret_cast<GS::uchar_t*>(*actParam.value.array)) + arrayIndex;
				arrayIndex += GS::ucslen32 (uValueStr) + 1;
				arrayValueItemAdder (GS::UniString (uValueStr));
			}
		}
	}
}


GS::ObjectState	GetGDLParametersOfElementsCommand::Execute (const GS::ObjectState& parameters, GS::ProcessControl& /*processControl*/) const
{
	GS::Array<GS::ObjectState> elements;
	parameters.Get (ElementsParameterField, elements);

	GS::ObjectState response;
	const auto& listAdder = response.AddList<GS::ObjectState> (GDLParametersOfElementsResponseFieldName);

	API_Guid elemGuid;
	for (const GS::ObjectState& element : elements) {
		const GS::ObjectState* elementId = element.Get (ElementIdField);
		if (elementId == nullptr) {
			continue;
		}

		elemGuid = GetGuidFromElementIdField (*elementId);

		API_ParamOwnerType paramOwner = {};
		paramOwner.libInd = 0;
#ifdef ServerMainVers_2600
		paramOwner.type   = API_ObjectID;
#else
		paramOwner.typeID = API_ObjectID;
#endif
		paramOwner.guid   = elemGuid;

		API_ElementMemo memo = {};
		GSErrCode err = ACAPI_Element_GetMemo (elemGuid, &memo, APIMemoMask_AddPars);
		if (err == NoError) {
			const GSSize nParams = BMGetHandleSize ((GSHandle) memo.params) / sizeof (API_AddParType);
			GS::ObjectState gdlParametersDictionary;
			for (GSIndex ii = 0; ii < nParams; ++ii) {
				const API_AddParType& actParam = (*memo.params)[ii];

				if (actParam.typeID == APIParT_Separator) {
					continue;
				}

				GS::ObjectState gdlParameterDetails;
				gdlParameterDetails.Add (ParameterIndexFieldName, actParam.index);
				gdlParameterDetails.Add (ParameterTypeFieldName, ConvertAddParIDToString (actParam.typeID));
				if (actParam.typeMod == API_ParArray) {
					gdlParameterDetails.Add (ParameterDimension1FieldName, actParam.dim1);
					gdlParameterDetails.Add (ParameterDimension2FieldName, actParam.dim2);
				}

				switch (actParam.typeID) {
					case APIParT_Integer:
					case APIParT_PenCol:			AddValueInteger (gdlParameterDetails, actParam);	break;
					case APIParT_ColRGB:
					case APIParT_Intens:
					case APIParT_Length:
					case APIParT_RealNum:
					case APIParT_Angle:				AddValueDouble (gdlParameterDetails, actParam);		break;
					case APIParT_LightSw:			AddValueOnOff (gdlParameterDetails, actParam); 		break;
					case APIParT_Boolean: 			AddValueBool (gdlParameterDetails, actParam);		break;
					case APIParT_LineTyp:
					case APIParT_Mater:
					case APIParT_FillPat:
					case APIParT_BuildingMaterial:
					case APIParT_Profile: 			AddValueAttribute (gdlParameterDetails, actParam);	break;
					case APIParT_CString:
					case APIParT_Title: 			AddValueString (gdlParameterDetails, actParam);		break;
					default:
					case APIParT_Dictionary:
						// Not supported by the Archicad API yet
						break;
				}

				gdlParametersDictionary.Add (actParam.name, gdlParameterDetails);
			}
			listAdder (gdlParametersDictionary);
			ACAPI_DisposeAddParHdl (&memo.params);
		} else {
			const GS::UniString errorMsg = GS::UniString::Printf ("Failed to get parameters of element with guid %T!", APIGuidToString (elemGuid).ToPrintf ());
			return CreateErrorResponse ((APIErrCodes) err, errorMsg);
		}
	}

	return response;
}
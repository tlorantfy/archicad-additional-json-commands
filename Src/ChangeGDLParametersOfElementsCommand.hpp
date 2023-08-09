#if !defined (CHANGEGDLPARAMETERSOFELEMENTSCOMMAND_HPP)
#define CHANGEGDLPARAMETERSOFELEMENTSCOMMAND_HPP

#pragma once

#include	"AdditionalJSONCommand.hpp"


class ChangeGDLParametersOfElementsCommand : public AdditionalJSONCommand {
public:
	virtual GS::String							GetName () const override;
	virtual GS::Optional<GS::UniString>			GetSchemaDefinitions () const override;
	virtual GS::Optional<GS::UniString>			GetInputParametersSchema () const override;
	virtual GS::ObjectState						Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;
};


#endif
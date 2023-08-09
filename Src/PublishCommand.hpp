#if !defined (PUBLISHCOMMAND_HPP)
#define PUBLISHCOMMAND_HPP

#pragma once

#include	"AdditionalJSONCommand.hpp"


class PublishCommand : public AdditionalJSONCommand {
public:
	virtual GS::String							GetName () const override;
	virtual GS::Optional<GS::UniString>			GetInputParametersSchema () const override;
	virtual GS::ObjectState						Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;
};


#endif
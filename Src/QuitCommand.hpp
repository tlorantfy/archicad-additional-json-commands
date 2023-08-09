#if !defined (QUITCOMMAND_HPP)
#define QUITCOMMAND_HPP

#pragma once

#include	"AdditionalJSONCommand.hpp"


class QuitCommand : public AdditionalJSONCommand {
public:
	virtual GS::String							GetName () const override;
	virtual GS::ObjectState						Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;
};


#endif
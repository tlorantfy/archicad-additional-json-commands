#if !defined (TEAMWORKRECEIVECOMMAND_HPP)
#define TEAMWORKRECEIVECOMMAND_HPP

#pragma once

#include	"AdditionalJSONCommand.hpp"


class TeamworkReceiveCommand : public AdditionalJSONCommand {
public:
	virtual GS::String							GetName () const override;
	virtual GS::ObjectState						Execute (const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;
};


#endif
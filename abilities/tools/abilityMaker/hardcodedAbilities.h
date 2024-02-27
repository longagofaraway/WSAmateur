#pragma once

#include "abilities.h"

asn::Ability brainstormDeck();
asn::Ability brainstormDraw();
asn::Ability encore();
asn::Ability bond();

asn::Trigger fromHandToStagePreset();
asn::Trigger thisCardAttacks();
asn::Trigger thisCardBecomesReversed();
asn::Trigger climaxIsPlaced();
asn::Trigger startOfOppAttackPhase();

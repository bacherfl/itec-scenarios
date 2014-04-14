#ifndef PACKETBASEDDROPPINGPOLICY_H
#define PACKETBASEDDROPPINGPOLICY_H

#include "droppingpolicy.h"

class PacketBasedDroppingPolicy : public DroppingPolicy
{
public:
    PacketBasedDroppingPolicy();

protected:
    /*!
      Calculates the dropping probability - protected virtual method, needs
      to be overwritten by the extending class; Called by Feed()
      \see Feed
      */
    virtual void CalculateDroppingProbabilities(LevelStatistics& L);
};

#endif // PACKETBASEDDROPPINGPOLICY_H

#ifndef NETWORKGENERATOR_H
#define NETWORKGENERATOR_H

#include "ndnbritehelper.h"
#include "map"
#include "ns3/random-variable-stream.h"
#include "ns3/point-to-point-module.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

#include "ns3/names.h"
#include "ns3/log.h"
#include "boost/lexical_cast.hpp"

namespace ns3
{
namespace  ndn
{

class NetworkGenerator
{
public:

  enum NodePlacement
  {
    LeafNode,
    ASNode
  };

  NetworkGenerator(std::string conf_file);
  void randomlyPlaceNodes(int nodeCount, std::string setIdentifier, NodePlacement place, PointToPointHelper *p2p);
  void randomlyPlaceNodes(int nodeCount, std::string setIdentifier, NodePlacement place, PointToPointHelper *p2p, std::vector<int> ASnumbers);

  void randomlyAddConnectionsBetweenAllAS(int numberOfConnectionsPerAsPair, int minBW_kbits, int maxBw_kbits, int minDelay, int maxDelay);

  int getNumberOfAS();
  int getNumberOfNodesInAS(int ASnumber);

  /**
   * @brief getAllASNodes
   * @return all nodes in of the current topology excluding custom nodes.
   */
  NodeContainer getAllASNodes();

  /**
   * @brief getAllASNodesFromAS
   * @param ASnumber
   * @return all nodes of AS ASnumber excluding custom nodes.
   */
  NodeContainer getAllASNodesFromAS(int ASnumber);

  /**
   * @brief getAllLeafNodes
   * @return all LeafNodes excluding custom nodes. Note that the intersection of LeafNodes and ASNodes is not NULL!
   */
  NodeContainer getAllLeafNodes();

  /**
   * @brief getAllLeafNodesFromAS
   * @param ASnumber
   * @return all LeafNodes for the current AS excluding custom nodes. Note that the intersection of LeafNodes and ASNodes is not NULL!
   */
  NodeContainer getAllLeafNodesFromAS(int ASnumber);

  NodeContainer getCustomNodes(std::string setIdentifier);

  void creatRandomLinkFailure(double minTimestamp, double maxTimestamp, double minDuration, double maxDuration);


private:
  NDNBriteHelper *briteHelper;

  typedef
  std::map<
  std::string /*label*/,
  NodeContainer/*nodes*/
  > CustomNodesMap;

  CustomNodesMap nodeContainerMap;

  Ptr<UniformRandomVariable> rvariable;

};
}
}
#endif // NETWORKGENERATOR_H

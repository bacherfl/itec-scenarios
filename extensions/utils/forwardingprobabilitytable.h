#ifndef FORWARDINGPROBABILITYTABLE_H
#define FORWARDINGPROBABILITYTABLE_H

#include "ns3/random-variable.h"
#include "ns3/ndn-face.h"
#include "ns3/ndn-interest.h"
#include "ns3/log.h"
#include "ns3/simple-ref-count.h"

#include <vector>
#include <stdio.h>

#include "forwardingstatistics.h"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

#include "parameterconfiguration.h"

#include <signal.h>

#define FACE_NOT_FOUND -1
namespace ns3
{
namespace ndn
{
class ForwardingProbabilityTable : public SimpleRefCount<ForwardingProbabilityTable>
{
public:
  ForwardingProbabilityTable(std::vector<int> faceIds, std::vector<int> preferedFacesIds = std::vector<int>());

  int determineOutgoingFace(Ptr<ndn::Face> inFace, Ptr<const Interest> interest, int ilayer, std::vector<int> blocked_faces);

  void updateColumns(Ptr<ForwardingStatistics> stats);

  void syncDroppingPolicy(Ptr<ForwardingStatistics> stats);

  double getForwardingProbability(int faceId, int layer){return table(determineRowOfFace(faceId), layer);}

  void addFace(int faceId);
  void removeFace(int faceId);

protected:

  boost::numeric::ublas::matrix<double> table;
  std::vector<int> faceIds;
  std::map<int /*layer*/, bool /*punished*/> jammed;

  void initTable(std::vector<int> preferedFacesIds);
  boost::numeric::ublas::matrix<double> removeFaceFromTable(Ptr<ndn::Face> face, boost::numeric::ublas::matrix<double> tab, std::vector<int> faces);
  boost::numeric::ublas::matrix<double> removeFaceFromTable (int faceId, boost::numeric::ublas::matrix<double> tab, std::vector<int> faces);
  boost::numeric::ublas::matrix<double> addFaceToTable (Ptr<ndn::Face> face);
  boost::numeric::ublas::matrix<double> addFaceToTable (int faceId);
  boost::numeric::ublas::matrix<double> normalizeColumns(boost::numeric::ublas::matrix<double> m);
  int chooseFaceAccordingProbability(boost::numeric::ublas::matrix<double> m, int layer_of_interest, std::vector<int> faceList);

  double getSumOfWeightedForwardingProbabilities(std::vector<int> set_of_faces, int layer, Ptr<ForwardingStatistics> stats);
  double getSumOfActualForwardingProbabilities(std::vector<int> set_of_faces, int layer, Ptr<ForwardingStatistics> stats);

  void updateColumn(std::vector<int> faces, int layer,Ptr<ForwardingStatistics> stats, double utf, bool shift_traffic);
  void probeColumn(std::vector<int> faces, int layer, Ptr<ForwardingStatistics> stats, bool useDroppingProbabilityFromFWT);
  void shiftDroppingTraffic(std::vector<int> faces, int layer,Ptr<ForwardingStatistics> stats);

  int determineRowOfFace(Ptr<ndn::Face> face, boost::numeric::ublas::matrix<double> tab, std::vector<int> faces);
  int determineRowOfFace(int face_uid, boost::numeric::ublas::matrix<double> tab, std::vector<int> faces);

  int determineRowOfFace(Ptr<ndn::Face> face);
  int determineRowOfFace(int face_uid);

  double calcWeightedUtilization(int faceId, int layer, Ptr<ForwardingStatistics> stats);

  int getFirstDroppingLayer();
  int getLastDroppingLayer();

  ns3::UniformVariable randomVariable;
};

}
}
#endif // FORWARDINGPROBABILITYTABLE_H


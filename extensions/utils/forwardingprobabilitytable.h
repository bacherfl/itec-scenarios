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

#define RELIABILITY_THRESHOLD 0.95
#define PROBING_TRAFFIC 0.03
#define PROBING_THRESHOLD 0.01
#define SHIFT_TRAFFIC 0.20

namespace ns3
{
namespace ndn
{
class ForwardingProbabilityTable : public SimpleRefCount<ForwardingProbabilityTable>
{
public:
  ForwardingProbabilityTable(std::vector<int> faceIds);

  int determineOutgoingFace(Ptr<ndn::Face> inFace, Ptr<const Interest> interest, int ilayer);

  void updateColumns(Ptr<ForwardingStatistics> stats);

protected:

  boost::numeric::ublas::matrix<double> table;
  std::vector<int> faceIds;

  void initTable();
  boost::numeric::ublas::matrix<double> removeFaceFromTable(Ptr<ndn::Face> face);
  boost::numeric::ublas::matrix<double> normalizeColumns(boost::numeric::ublas::matrix<double> m);
  int chooseFaceAccordingProbability(boost::numeric::ublas::matrix<double> m, int layer_of_interest, std::vector<int> faceList);

  double getSumOfForwardingProbabilities(std::vector<int> set_of_faces, int layer);

  void updateColumn(std::vector<int> faces, int layer,Ptr<ForwardingStatistics> stats, bool shift_traffic);
  void probeColumn(std::vector<int> faces, int layer,Ptr<ForwardingStatistics> stats);
  void shiftDroppingTraffic(std::vector<int> faces, int layer,Ptr<ForwardingStatistics> stats);

  int determineRowOfFace(Ptr<ndn::Face> face);
  int determineRowOfFace(int face_uid);

  ns3::UniformVariable randomVariable;
};

}
}
#endif // FORWARDINGPROBABILITYTABLE_H


#include "forwardingprobabilitytable.h"

using namespace ns3::ndn;
using namespace boost::numeric::ublas;

NS_LOG_COMPONENT_DEFINE ("ForwardingProbabilityTable");

ForwardingProbabilityTable::ForwardingProbabilityTable(std::vector<int> faceIds)
{
  this->faceIds = faceIds;
  initTable ();
}

void ForwardingProbabilityTable::initTable ()
{
  table = matrix<double> (faceIds.size (), MAX_LAYERS);

  // fill matrix column-wise /* table(i,j) = i-th row, j-th column*/
  for (unsigned j = 0; j < table.size2 (); ++j) /* columns */
  {
    for (unsigned i = 0; i < table.size1 (); ++i) /* rows */
    {
      //fprintf(stderr, " faceIds.at (i) %d\n",faceIds.at (i));
      if(faceIds.at (i) == DROP_FACE_ID)
        table(i,j) = 0.0;
      else
        table(i,j) = (1.0 / ((double)faceIds.size () - 1.0)); /*set default value to 1 / (d - 1) */
    }
  }

   std::cout << table << std::endl; /* prints matrix line by line ( (first line), (second line) )*/
}

int ForwardingProbabilityTable::determineOutgoingFace(Ptr<ndn::Face> inFace, Ptr<const Interest> interest)
{
  // determine layer of interest

  int ilayer = 0; //TODO

  // normalize column of layer for all possible outgoing faces
  matrix<double> normalized = removeFaceFromTable(inFace);

  // choose one face as outgoing according to the probability
}

matrix<double> ForwardingProbabilityTable::removeFaceFromTable(Ptr<ndn::Face> face)
{
  //determine column of face
  int faceColumn = -1;

  for(int i = 0; i < faceIds.size () ; i++)
  {
    if(faceIds.at (i) == face->GetId ())
    {
      faceColumn = i;
      break;
    }
  }

  if(faceColumn == -1)
  {
    NS_LOG_UNCOND("ERROR: Invalid faceID.");
    return table;
  }

}


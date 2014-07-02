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
  table = matrix<double> (faceIds.size () /*rows*/, MAX_LAYERS /*columns*/);

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

   //std::cout << table << std::endl; /* prints matrix line by line ( (first line), (second line) )*/
}

int ForwardingProbabilityTable::determineOutgoingFace(Ptr<ndn::Face> inFace, Ptr<const Interest> interest, int ilayer)
{
  // determine layer of interest

  // normalize column of layer for all possible outgoing faces
  matrix<double> normalized = removeFaceFromTable(inFace);

  //create a copy of the faceIds that excludes the incoming face
  std::vector<int> face_list(faceIds);
  int offset = determineRowOfFace(inFace);

  face_list.erase (face_list.begin ()+offset);

  // choose one face as outgoing according to the probability
  return chooseFaceAccordingProbability(normalized, ilayer, face_list);
}

int ForwardingProbabilityTable::determineRowOfFace(Ptr<ndn::Face> face)
{
  //determine row of face
  int faceRow = -1;

  for(int i = 0; i < faceIds.size () ; i++)
  {
    if(faceIds.at (i) == face->GetId ())
    {
      faceRow = i;
      break;
    }
  }

  if(faceRow == -1)
  {
    NS_LOG_UNCOND("ERROR: Invalid faceID.");
  }

  return faceRow;
}

matrix<double> ForwardingProbabilityTable::removeFaceFromTable(Ptr<ndn::Face> face)
{

  int faceRow = determineRowOfFace (face);

  if(faceRow == -1)
    return table;

  matrix<double> m (table.size1 () - 1, table.size2 ());

  for (unsigned j = 0; j < table.size2 (); ++j) /* columns */
  {
    for (unsigned i = 0; i < table.size1 (); ++i) /* rows */
    {
      if(i < faceRow)
      {
        m(i,j) = table(i,j);
      }
      /*else if(faceRow == i)
      {
        // skip i-th row.
      }*/
      else if (i > faceRow)
      {
        m(i-1,j) = table(i,j);
      }
    }
  }

  return normalizeColumns (m);
}

boost::numeric::ublas::matrix<double> ForwardingProbabilityTable::normalizeColumns(boost::numeric::ublas::matrix<double> m)
{
  for (unsigned j = 0; j < m.size2 (); ++j) /* columns */
  {
    double colSum= 0;
    for (unsigned i = 0; i < m.size1 (); ++i) /* rows */
    {
      colSum += m(i,j);
    }

    for (unsigned i = 0; i < m.size1 (); ++i) /* rows */
    {
      m(i,j) /= colSum;
    }
  }

  return m;
}

int ForwardingProbabilityTable::chooseFaceAccordingProbability(boost::numeric::ublas::matrix<double> m, int layer_of_interest, std::vector<int> faceList)
{
  /*Example:
   *
   *random variable has value R and matrix:
   *F0: 0.23  <--- R < 0.23
   *F1: 0.37  <--- 0.23 < R < 0.23 + 0.37
   *F3: 0.40  <--- 0.23 + 0.37 < R < 0.23 + 0.37 + 0.40
   */

  double rvalue = randomVariable.GetValue ();
  double sum = 0.0;

  if(faceList.size () != m.size1 ())
  {
    NS_LOG_UNCOND("Error ForwardingMatrix has not the same amount of rows as the facelist!");
    return DROP_FACE_ID;
  }

  if(faceList.size () == 1 && faceList.at (0) == DROP_FACE_ID) //Todo think if we should do something about this? maybe send a nack?
    return DROP_FACE_ID;

  for(int i = 0; i < m.size1 (); i++)
  {
    sum += m(i, layer_of_interest);

    if(rvalue <= sum)
      return faceList.at (i);
  }

  NS_LOG_UNCOND("Error in Face selection!");
  return DROP_FACE_ID;
}

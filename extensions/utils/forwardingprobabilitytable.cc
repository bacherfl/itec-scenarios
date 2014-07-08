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
  return determineRowOfFace (face->GetId ());
}

int ForwardingProbabilityTable::determineRowOfFace(int face_id)
{
  //determine row of face
  int faceRow = -1;

  for(int i = 0; i < faceIds.size () ; i++)
  {
    if(faceIds.at (i) == face_id)
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
      if(m(i,j) > 0)
        colSum += m(i,j);
    }

    if(colSum == 0) // means we have removed the only face that was able to transmitt the traffic
    {
      //what should we do? drop all ? split the probabilities amoung all other faces??
      //split
      for (unsigned i = 0; i < m.size1 (); ++i) /* rows */
        m(i,j) = 1.0 /((double)m.size1 ());
    }
    else
    {
      for (unsigned i = 0; i < m.size1 (); ++i) /* rows */
      {
        if(m(i,j) < 0)
          m(i,j) = 0;
        else
          m(i,j) /= colSum;
      }
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

  //if(faceList.size () == 1 && faceList.at (0) == DROP_FACE_ID) //Todo think if we should do something about this? maybe send a nack?
    //return DROP_FACE_ID;

  for(int i = 0; i < m.size1 (); i++)
  {
    //fprintf(stderr, "row = %d\n", i);
    //fprintf(stderr, "layer_of_interest = %d\n", layer_of_interest);
    sum += m(i, layer_of_interest);

    if(rvalue <= sum)
      return faceList.at (i);
  }

  NS_LOG_UNCOND("Error in Face selection!");
  return DROP_FACE_ID;
}

void ForwardingProbabilityTable::updateColumns(Ptr<ForwardingStatistics> stats)
{

  fprintf(stderr,"Forwarding Matrix before update:\n");
  std::cout << table << std::endl;

  fprintf(stderr, "UTF = %f\n", stats->getUnstatisfiedTrafficFraction (0));

  std::vector<int> r_faces;
  std::vector<int> ur_faces;

  for(int i = 0; i < MAX_LAYERS; i++) // for each layer
  {
    //determine the set of reliable faces
    r_faces = stats->getReliableFaces (i, RELIABILITY_THRESHOLD);

    //determine the set of unreliable faces
    ur_faces = stats->getUnreliableFaces (i, RELIABILITY_THRESHOLD);

    //utf = unstatisfied_trafic_fraction
    double utf = stats->getUnstatisfiedTrafficFraction (i);

    //check if we need to shift traffic
    if(utf > 0)
    {

      double r_faces_actual_fowarding_prob = 0.0;
      //check if relialbe faces act forwarding Prob > 0
      for(std::vector<int>::iterator it = r_faces.begin(); it != r_faces.end(); ++it) // for each r_face
      {
        r_faces_actual_fowarding_prob += stats->getActualForwardingProbability (*it,i);
      }

      fprintf(stderr, "r_faces.size () = %d and r_faces_actual_fowarding_prob = %f\n",r_faces.size (),r_faces_actual_fowarding_prob );

      //if we have no place to shift; or no interests can be forwarded on reliable faces
      if(r_faces.size () == 0 || r_faces_actual_fowarding_prob == 0.0) // we drop everything in this case
      {

        double actual_d_face_prob = ((double)stats->getDroppedInterests (i)) / (double)(stats->getDroppedInterests (i) + stats->getForwardedInterests (i));

        fprintf(stderr, "actual_d_face_prob = %f\n", actual_d_face_prob);

        double forwarded_fraction = ((double)stats->getForwardedInterests (i)) / (double)(stats->getDroppedInterests (i) + stats->getForwardedInterests (i));

        table(determineRowOfFace(DROP_FACE_ID), i) = actual_d_face_prob + utf * forwarded_fraction;
        updateColumn (ur_faces, i, stats, false);
      }
      else
      {
        //add traffic
        updateColumn (r_faces, i, stats, true);

        //remove traffic
        updateColumn (ur_faces, i, stats, false);
       }
    }
    else if(r_faces.size () > 0)
    {
      if(table(determineRowOfFace(DROP_FACE_ID),i) > 0)
      {
        fprintf(stderr,"WE SHOULD DECREASE DROPPING TRAFFIC\n");
        fprintf(stderr,"Dropped %d interests for laye %d\n", stats->getDroppedInterests(i), i);

        //check if we should do probing or shift traffic
        std::vector<int> shift_faces;
        for(std::vector<int>::iterator it = r_faces.begin(); it != r_faces.end(); ++it) // for each r_face
        {
          if(stats->getActualForwardingProbability (*it,i) > PROBING_THRESHOLD)
            shift_faces.push_back (*it);
        }

        if(shift_faces.size () == 0) // probing
          probeColumn(ur_faces, i, stats);
        else //shift traffic
          shiftDroppingTraffic(shift_faces, i, stats);

      }
    }
  }

  fprintf(stderr,"Forwarding Matrix after update:\n");
  std::cout << table << std::endl;

  //finally just normalize to remove the error introduced by threashold RELIABILITY_THRESHOLD
  table = normalizeColumns(table);

  fprintf(stderr,"Forwarding Matrix after normalization:\n");
  std::cout << table << std::endl << std::endl;
}

//shift == true means shift trafic towards faces, else remove traffic from faces.
void ForwardingProbabilityTable::updateColumn(std::vector<int> faces, int layer, Ptr<ForwardingStatistics> stats, bool shift_traffic)
{
  double sum_reliabilities = 0.0;

  if(shift_traffic)
    sum_reliabilities = stats->getSumOfReliabilies (faces, layer);
  else
    sum_reliabilities = stats->getSumOfUnreliabilies (faces, layer);

  double sum_fwProbs = getSumOfForwardingProbabilities (faces, layer);
  double utf = stats->getUnstatisfiedTrafficFraction (layer);

  if(sum_reliabilities == 0)
  {
    NS_LOG_UNCOND("Error sum_reliabilities == 0.\n");
  }

  if(sum_fwProbs == 0 && shift_traffic)
  {
    NS_LOG_UNCOND("Error sum_fwProbs == 0 for shift_traffic == true.\n");
  }

  double normalization_value = 0.0;
  for(std::vector<int>::iterator it = faces.begin(); it != faces.end(); ++it) // for each r_face
  {
    if(shift_traffic)
      normalization_value +=
        (table(determineRowOfFace (*it), layer) / sum_fwProbs) * (stats->getLinkReliability (*it,layer) / sum_reliabilities);
    else if(sum_fwProbs == 0)  // special case when forwarding probabilities are all 0 for all non relialbe faces. e.g. (0, 0, 1) where f3() = 1 is the incoming face of the interests
      normalization_value +=
        (1.0 /((double)faces.size ())) * ((1 - stats->getLinkReliability (*it,layer)) / sum_reliabilities);
    else
      normalization_value +=
        (table(determineRowOfFace (*it), layer) / sum_fwProbs) * ((1 - stats->getLinkReliability (*it,layer)) / sum_reliabilities);
  }

  if(normalization_value == 0)
  {
    NS_LOG_UNCOND("Error normalization_value == 0.\n");
  }

  for(std::vector<int>::iterator it = faces.begin(); it != faces.end(); ++it) // for each r_face
  {

    double actualFWProb = stats->getActualForwardingProbability (*it, layer);

    fprintf(stderr, "actual fwProb = %f\n", actualFWProb);

    //if(actualFWProb == 0) // means no traffic for this content has been observed
      //actualFWProb = table(determineRowOfFace (*it), layer); // then use old probs.

    if(shift_traffic)
    {
      table(determineRowOfFace(*it), layer) = actualFWProb +
        utf * (table(determineRowOfFace (*it), layer) / sum_fwProbs) * (stats->getLinkReliability (*it,layer) / sum_reliabilities) / normalization_value;
    }
    else if (sum_fwProbs == 0) // special case
    {
      table(determineRowOfFace(*it), layer) = actualFWProb -
          utf * (1.0 /((double)faces.size ())) * ( (1 - stats->getLinkReliability (*it,layer)) / sum_reliabilities / normalization_value);
    }
    else
    {
      table(determineRowOfFace(*it), layer) = actualFWProb -
          utf * (table(determineRowOfFace (*it), layer) / sum_fwProbs) * ( (1 - stats->getLinkReliability (*it,layer)) / sum_reliabilities / normalization_value);
    }
  }
}

void ForwardingProbabilityTable::probeColumn(std::vector<int> faces, int layer,Ptr<ForwardingStatistics> stats)
{
  fprintf(stderr, "PROBING\n");
  // we take 3% of the dropping traffic and for each reliable face and probe on them..
  double actual_d_face_prob = ((double)stats->getDroppedInterests (layer)) / (double)(stats->getDroppedInterests (layer) + stats->getForwardedInterests (layer));
  double probe = actual_d_face_prob * (PROBING_TRAFFIC * faces.size ());
  table(determineRowOfFace (DROP_FACE_ID), layer) = actual_d_face_prob - probe;

  //split the forwarding percents....
  for(std::vector<int>::iterator it = faces.begin(); it != faces.end(); ++it) // for each r_face
  {
    table(determineRowOfFace (*it), layer) = stats->getActualForwardingProbability (*it, layer) + (probe / ((double)faces.size ()));
  }
}

void ForwardingProbabilityTable::shiftDroppingTraffic(std::vector<int> faces, int layer,Ptr<ForwardingStatistics> stats)
{
  /*//calcualte how much traffic we can take
  int forwarded_interests = 0;
  for(std::vector<int>::iterator it = faces.begin(); it != faces.end(); ++it) // for each r_face
  {
    forwarded_interests += (int) stats->getForwardedInterests (face, layer);
  }

  int dropped = stats->getDroppedInterests (layer);
  int shift = 0;

  if(forwarded_interests * SHIFT_TRAFFIC > dropped)
  {
    //shift everthing
    table(determineRowOfFace(DROP_FACE_ID), i) = 0;
  }
  else
  {
    //shift SHIFT_TRAFFIC
  }

  //split the forwarding percents....
  for(std::vector<int>::iterator it = faces.begin(); it != faces.end(); ++it) // for each r_face
  {
    //table(determineRowOfFace (*it), layer) += fwPercent;
  }*/
}

double ForwardingProbabilityTable::getSumOfForwardingProbabilities(std::vector<int> set_of_faces, int layer)
{
  double sum = 0.0;
  for(std::vector<int>::iterator it = set_of_faces.begin(); it != set_of_faces.end(); ++it)
  {
    sum += table(determineRowOfFace (*it), layer);
  }
  return sum;
}

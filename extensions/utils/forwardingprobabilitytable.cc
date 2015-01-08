#include "forwardingprobabilitytable.h"

using namespace ns3::ndn;
using namespace boost::numeric::ublas;

NS_LOG_COMPONENT_DEFINE ("ForwardingProbabilityTable");

ForwardingProbabilityTable::ForwardingProbabilityTable(std::vector<int> faceIds, std::vector<int> preferedFacesIds)
{
  /*for(std::vector<int>::iterator it=faceIds.begin (); it!=faceIds.end (); it++)
    fprintf(stderr, "Having FaceID = %d\n", *it);*/

  this->curReliability = ParameterConfiguration::getInstance ()->getParameter("RELIABILITY_THRESHOLD_MIN");
  this->faceIds = faceIds;
  initTable (preferedFacesIds);
}

void ForwardingProbabilityTable::initTable (std::vector<int> preferedFacesIds)
{
  this->preferedFacesIds=preferedFacesIds;
  std::sort(this->faceIds.begin(), this->faceIds.end());//order

  table = matrix<double> (faceIds.size () /*rows*/, (int)ParameterConfiguration::getInstance ()->getParameter ("MAX_LAYERS") /*columns*/);

  // fill matrix column-wise /* table(i,j) = i-th row, j-th column*/
  for (unsigned j = 0; j < table.size2 (); ++j) /* columns */
  {
    for (unsigned i = 0; i < table.size1 (); ++i) /* rows */
    {
      if(faceIds.at (i) == DROP_FACE_ID)
        table(i,j) = 0.0;
      else if(preferedFacesIds.size () == 0)
      {
        table(i,j) = (1.0 / ((double)faceIds.size () - 1.0)); /*set default value to 1 / (d - 1) */
      }
      else
      {
        if(std::find(preferedFacesIds.begin (), preferedFacesIds.end (), faceIds.at (i)) != preferedFacesIds.end ())
        {
          table(i,j) = (1.0 / ((double)preferedFacesIds.size () - 1.0)); /*set default value to 1 / (d - 1) */
        }
        else
        {
          table(i,j) = 0; /*set default value to 1 / (d - 1) */
        }
      }
    }
  }

  for(int i = 0; i < (int)ParameterConfiguration::getInstance ()->getParameter ("MAX_LAYERS"); i++) // set all as non-jammed
    jammed[i] = false;

   //std::cout << table << std::endl; /* prints matrix line by line ( (first line), (second line) )*/
}

int ForwardingProbabilityTable::determineOutgoingFace(std::vector<Ptr<Face> > inFaces, Ptr<const Interest> interest, int ilayer, std::vector<int> blocked_faces)
{
  //copy data structs
  matrix<double> tmp_matrix(table);
  std::vector<int> face_list(faceIds);
  int offset = 0;

  //first remove the inface(s)
  for(std::vector<Ptr<Face> >::iterator i = inFaces.begin (); i != inFaces.end ();++i)
  {
    offset = determineRowOfFace(*i, tmp_matrix, face_list);
    if(offset != FACE_NOT_FOUND)
    {
      //then remove the row
      tmp_matrix = removeFaceFromTable(*i, tmp_matrix, face_list);
      //then remove the face from the list
      face_list.erase (face_list.begin ()+offset);
    }
    else
    {
      NS_LOG_UNCOND("Could not remove inface[i]=" << *i <<". There is something serousliy wrong. Returning the dropping face now...\n");
      return DROP_FACE_ID;
    }
  }

  //lets check if sum(Fi in blockedFaces > R)
  double fw_prob = 0.0;
  int row = -1;
  for(std::vector<int>::iterator i = blocked_faces.begin (); i != blocked_faces.end ();++i)
  {
    row = determineRowOfFace(*i, tmp_matrix, face_list);
    if(row != FACE_NOT_FOUND)
      fw_prob += tmp_matrix(row,ilayer);
    else// if(*i != inFaces->GetId ())
    {
      NS_LOG_UNCOND("Could not find blocked face " << *i << ". There is something serousliy wrong. Returning the dropping face now...\n");
      return DROP_FACE_ID;
    }
  }

  if(fw_prob >= curReliability)
    return DROP_FACE_ID;

  //ok now remove the blocked faces
  offset = 0;
  for(std::vector<int>::iterator i = blocked_faces.begin (); i != blocked_faces.end ();++i)
  {
    offset = determineRowOfFace(*i, tmp_matrix, face_list);
    if(offset != FACE_NOT_FOUND)
    {
      //then remove the row
      tmp_matrix = removeFaceFromTable(*i, tmp_matrix, face_list);

      //then remove the face from the list
      face_list.erase (face_list.begin ()+offset);
    }
    else// if(*i != inFaces->GetId ())
    {
      NS_LOG_UNCOND("Could not remove blocked face[i]=" << *i <<". There is something serousliy wrong. Returning the dropping face now...\n");
      return DROP_FACE_ID;
    }
  }

  // choose one face as outgoing according to the probability
  return chooseFaceAccordingProbability(tmp_matrix, ilayer, face_list);
}

int ForwardingProbabilityTable::determineRowOfFace(Ptr<ndn::Face> face)
{
  return determineRowOfFace (face->GetId (), table, faceIds);
}

int ForwardingProbabilityTable::determineRowOfFace(int face_uid)
{
  return determineRowOfFace (face_uid, table, faceIds);
}

int ForwardingProbabilityTable::determineRowOfFace(Ptr<ndn::Face> face, boost::numeric::ublas::matrix<double> tab, std::vector<int> faces)
{
  return determineRowOfFace (face->GetId (), tab, faces);
}

int ForwardingProbabilityTable::determineRowOfFace(int face_id, boost::numeric::ublas::matrix<double> tab, std::vector<int> faces)
{
  if(tab.size1 () != faces.size ())
  {
    //raise(SIGSEGV);
    fprintf(stderr, "Invalid Table to faceIds\n");
    fprintf(stderr, "table size = %u\n", tab.size1 ());
    fprintf(stderr, "faceIds size = %u\n", faces.size ());
    return FACE_NOT_FOUND;
  }

  //determine row of face
  int faceRow = FACE_NOT_FOUND;

  std::sort(faces.begin(), faces.end());//order

  for(int i = 0; i < faces.size () ; i++)
  {
    if(faces.at (i) == face_id)
    {
      //fprintf(stderr, "inface=%d, curFace=%d\n",face_id, faces.at (i));
      faceRow = i;
      break;
    }
  }
  //fprintf(stderr, "faceRow=%d\n", faceRow);
  return faceRow;
}

matrix<double> ForwardingProbabilityTable::removeFaceFromTable(Ptr<ndn::Face> face, boost::numeric::ublas::matrix<double> tab, std::vector<int> faces)
{
  return removeFaceFromTable(face->GetId (), tab, faces);
}

boost::numeric::ublas::matrix<double> ForwardingProbabilityTable::removeFaceFromTable (int faceId, boost::numeric::ublas::matrix<double> tab, std::vector<int> faces)
{
  int faceRow = determineRowOfFace (faceId, tab, faces);

  if(faceRow == FACE_NOT_FOUND)
  {
    NS_LOG_UNCOND("Could not remove Face from Table as it does not exist");
    return tab;
  }

  matrix<double> m (tab.size1 () - 1, tab.size2 ());

  for (unsigned j = 0; j < tab.size2 (); ++j) /* columns */
  {
    for (unsigned i = 0; i < tab.size1 (); ++i) /* rows */
    {
      if(i < faceRow)
      {
        m(i,j) = tab(i,j);
      }
      /*else if(faceRow == i)
      {
        // skip i-th row.
      }*/
      else if (i > faceRow)
      {
        m(i-1,j) = tab(i,j);
      }
    }
  }

  return normalizeColumns (m);
}

matrix<double> ForwardingProbabilityTable::addFaceToTable(Ptr<ndn::Face> face)
{
  return addFaceToTable(face->GetId ());
}

boost::numeric::ublas::matrix<double> ForwardingProbabilityTable::addFaceToTable (int faceId)
{

  faceIds.push_back (faceId);
  std::sort(this->faceIds.begin(), this->faceIds.end());//order

  int faceRow = determineRowOfFace (faceId);

  matrix<double> m (table.size1 () + 1, table.size2 ());

  for (unsigned j = 0; j < table.size2 (); ++j) /* columns */
  {
    for (unsigned i = 0; i < table.size1 (); ++i) /* rows */
    {
      if(i < faceRow)
      {
        m(i,j) = table(i,j);
      }
      else if(faceRow == i)
      {
        m(i,j) = 1.0 / (double)(faceIds.size () - 1);
      }
      else if (i > faceRow)
      {
        m(i+1,j) = table(i,j);
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
      //split robabilities amoung all other faces
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
    //fprintf(stderr, "facelistsize =%d\n",faceList.size ());
    //fprintf(stderr, "m.size1 =%d\n",m.size1());
    NS_LOG_UNCOND("Error ForwardingMatrix has not the same amount of rows as the facelist!");
    return DROP_FACE_ID;
  }

  for(int i = 0; i < m.size1 (); i++)
  {
    //fprintf(stderr, "layer_of_interest = %d\n", layer_of_interest);
    sum += m(i, layer_of_interest);

    if(rvalue <= sum)
      return faceList.at (i);
  }

  NS_LOG_UNCOND("Error in Face selection, structs are:");
  NS_LOG_UNCOND("table\n" << m);
  NS_LOG_UNCOND("layer_of_interest " << layer_of_interest);
  NS_LOG_UNCOND("rvalue = " << rvalue);

  for(std::vector<int>::iterator it = faceList.begin (); it != faceList.end (); ++it)
    fprintf(stderr, "facelist[it]=%d\n",*it);

  return DROP_FACE_ID;
}

void ForwardingProbabilityTable::updateColumns(Ptr<ForwardingStatistics> stats)
{
  NS_LOG_DEBUG("Forwarding Matrix before update:\n" << table);

  //fprintf(stderr, "%d: current Reliability=%f\n",Simulator::Now ().GetMilliSeconds (),curReliability);
  NS_LOG_DEBUG("Reliabilty Threshold before update=" << curReliability);
  std::vector<int> r_faces;
  std::vector<int> ur_faces;

  for(int layer = 0; layer < (int)ParameterConfiguration::getInstance ()->getParameter ("MAX_LAYERS"); layer++) // for each layer
  {
    //determine the set of reliable faces
    r_faces = stats->getReliableFaces (layer, curReliability);

    //determine the set of unreliable faces
    ur_faces = stats->getUnreliableFaces (layer, curReliability);

    //utf = unstatisfied_trafic_fraction
    //double utf = stats->getUnstatisfiedTrafficFraction (layer);
    double utf = stats->getUnstatisfiedTrafficFractionOfUnreliableFaces (layer);
    utf *= ParameterConfiguration::getInstance ()->getParameter ("ALPHA");

    /*debug information*/
    NS_LOG_DEBUG("Layer " << layer << " UTF=" << stats->getUnstatisfiedTrafficFraction (layer) << " UTF*alpha=" << utf);
    for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it) // for each r_face
      NS_LOG_DEBUG("a(F_" << *it << ")= " << stats->getActualForwardingProbability (*it, layer) <<
                   "\t u(F_" << *it << ")= " << calcWeightedUtilization(*it,layer,stats));

    //check if we need to shift traffic
    if(utf > 0)
    {
      //determine the relialbe faces act forwarding Prob > 0
      double r_faces_actual_fowarding_prob = 0.0;
      for(std::vector<int>::iterator it = r_faces.begin(); it != r_faces.end(); ++it) // for each r_face
      {
        r_faces_actual_fowarding_prob += stats->getActualForwardingProbability (*it,layer);
      }

      //if we have no relible faces, or no interests can be forwarded to reliable faces
      if(r_faces.size () == 0 || r_faces_actual_fowarding_prob == 0.0) // we drop everything in this case
      {
        //fprintf(stderr, "CASE 1\n");
        table(determineRowOfFace(DROP_FACE_ID), layer) = calcWeightedUtilization(DROP_FACE_ID,layer,stats)+ utf;
        updateColumn (ur_faces, layer, stats, utf, false);

        if(!jammed[layer])  // only probe if not jammed
          probeColumn(r_faces, layer, stats, true);

        //lower reliability
        if(table(determineRowOfFace (DROP_FACE_ID),layer) > (1.0-curReliability) )
          decreaseReliabilityThreshold();
      }
      else
      {
        //fprintf(stderr, "CASE 2\n");
        //add traffic to relialbe faces
        updateColumn (r_faces, layer, stats, utf, true);
        //remove traffic from unreliable faces
        updateColumn (ur_faces, layer, stats, utf, false);
        //set dropping face
        table(determineRowOfFace(DROP_FACE_ID), layer) = calcWeightedUtilization(DROP_FACE_ID,layer,stats);
       }
    }
    else if (/*delta == 0* &&*/ !jammed[layer]) // utf == 0 and layer has not been jammed last time
    {

      if(stats->getTotalForwardedInterests (layer) == 0)
      {
        //fprintf(stderr, "CASE 3\n");

        /*if(r_faces.size () == 0 || table(determineRowOfFace (DROP_FACE_ID), layer) < ParameterConfiguration::getInstance ()->getParameter ("X_DROPPING")) //then use the old values
        {
           NS_LOG_DEBUG("No interests forwarded for layer " << layer << ". r_facers =" << r_faces.size () << " F(-1)=" << table(determineRowOfFace (DROP_FACE_ID), layer)
                        << "vs X_DROPPING=" << ParameterConfiguration::getInstance ()->getParameter ("X_DROPPING"));
          continue;
        }*/

        double probe = table(determineRowOfFace (DROP_FACE_ID), layer) * ParameterConfiguration::getInstance ()->getParameter ("PROBING_TRAFFIC");
         table(determineRowOfFace (DROP_FACE_ID), layer) -= probe;

        //distribute the traffic
        for(std::vector<int>::iterator it = r_faces.begin(); it != r_faces.end(); ++it) // for each ur_face
          table(determineRowOfFace (*it), layer) += (probe / ((double)r_faces.size ()));
      }
      else if(table(determineRowOfFace(DROP_FACE_ID),layer) == 0.0) // dropping prob == 0 or there are no reliable faces
      {
        //fprintf(stderr, "CASE 4\n");
        if(stats->getTotalForwardedInterests (layer) != 0)
          for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it)
            table(determineRowOfFace(*it), layer) = calcWeightedUtilization(*it,layer,stats);

        //increase Reliability Threshold
         increaseReliabilityThreshold();
      }
      else
      {
        NS_LOG_DEBUG("WE SHOULD DECREASE DROPPING TRAFFIC\n");

        //check if we should do probing or shifting and probing
        std::vector<int> shift_faces;
        std::vector<int> probe_faces;
        for(std::vector<int>::iterator it = r_faces.begin(); it != r_faces.end(); ++it)
        {
          //if(calcWeightedUtilization (*it,layer,stats) > SHIFT_THRESHOLD) start here
          if(stats->getActualForwardingProbability (*it,layer) > ParameterConfiguration::getInstance()->getParameter ("SHIFT_THRESHOLD"))
            shift_faces.push_back (*it);
          else
            probe_faces.push_back (*it);
        }

        if(shift_faces.size () == 0)
        {
          //needs normalization if, no interests have been transmitted at all.
          // however this might be good, as it lowers, dropping probability, and distributes forwarding prob at all other faces...
          //fprintf(stderr, "CASE 5\n");
          probeColumn(probe_faces, layer, stats, false); // do only probing
        }
        else
        {
          //fprintf(stderr, "CASE 6\n");
          shiftDroppingTraffic(shift_faces, layer, stats); //shift traffic
          probeColumn(probe_faces, layer, stats, true); // and probe then

          //increase Reliability Threshold
          if(table(determineRowOfFace (DROP_FACE_ID),layer) < (1.0-curReliability) )
            increaseReliabilityThreshold();
        }
      }
    }
  }

  NS_LOG_DEBUG("Forwarding Matrix after update:\n" << table);

  /*std::stringstream ss1;
  ss1 << table;
  std::string s1 = ss1.str();*/

  //finally just normalize to remove the error introduced by threashold RELIABILITY_THRESHOLD
  table = normalizeColumns(table);

  /*std::stringstream ss2;
  ss2 << table;
  std::string s2 = ss2.str();
  if(s2.compare (s1) != 0)
  {
    fprintf(stderr, "s1 = %s\n", s1.c_str ());
    fprintf(stderr, "s2 = %s\n\n", s2.c_str ());
  }*/

  NS_LOG_DEBUG("Forwarding Matrix after normalization:\n" << table);

  NS_LOG_DEBUG("Reliabilty Threshold after update =" << curReliability);
}

//shift == true means shift trafic towards faces, else remove traffic from faces.
void ForwardingProbabilityTable::updateColumn(std::vector<int> faces, int layer, Ptr<ForwardingStatistics> stats, double utf,
                                              bool shift_traffic/*true -> traffic will be shifted towards faces, false traffic will be shifted away*/)
{
  if(faces.size () == 0)
    return;

  double sum_reliabilities = 0.0;

  if(shift_traffic)
    sum_reliabilities = stats->getSumOfReliabilies (faces, layer);
  else
    sum_reliabilities = stats->getSumOfUnreliabilies (faces, layer);

  double sum_fwProbs = getSumOfWeightedForwardingProbabilities (faces, layer,stats);

  double normalization_value = 0.0;
  for(std::vector<int>::iterator it = faces.begin(); it != faces.end(); ++it) // for each r_face
  {
    if(shift_traffic && sum_fwProbs > 0)
    {
      normalization_value +=
        (calcWeightedUtilization(*it,layer,stats) / sum_fwProbs) * (stats->getLinkReliability (*it,layer) / sum_reliabilities);
    }
    else if(shift_traffic && sum_fwProbs == 0)  // special case when forwarding probabilities are all 0 for all non relialbe faces. e.g. (0, 0, 1) where f3() = 1 is the incoming face of the interests
    {
      normalization_value +=
        (1.0 /((double)faces.size ())) * (stats->getLinkReliability (*it,layer) / sum_reliabilities);
    }
    else if(!shift_traffic && sum_fwProbs == 0)
    {
      normalization_value +=
        (1.0 /((double)faces.size ())) * ((1 - stats->getLinkReliability (*it,layer)) / sum_reliabilities);
    }
    else
    {
      normalization_value +=
        (calcWeightedUtilization(*it,layer,stats) / sum_fwProbs) * ((1 - stats->getLinkReliability (*it,layer)) / sum_reliabilities);
    }
  }

  if(normalization_value == 0)
  {
    NS_LOG_UNCOND("Error normalization_value == 0.\n");
  }

  for(std::vector<int>::iterator it = faces.begin(); it != faces.end(); ++it) // for each r_face
  {

    double weightedUtil = calcWeightedUtilization(*it,layer,stats);

    if(shift_traffic && sum_fwProbs > 0)
    {
      table(determineRowOfFace(*it), layer) = weightedUtil +
        utf * (weightedUtil / sum_fwProbs) * (stats->getLinkReliability (*it,layer) / sum_reliabilities) / normalization_value;
    }
    else if (shift_traffic && sum_fwProbs == 0) // special case
    {
      table(determineRowOfFace(*it), layer) = weightedUtil +
          utf * (1.0 /((double)faces.size ())) * ( stats->getLinkReliability (*it,layer) / sum_reliabilities / normalization_value);
    }
    else if (!shift_traffic && sum_fwProbs == 0) // special case
    {
      table(determineRowOfFace(*it), layer) = weightedUtil -
          utf * (1.0 /((double)faces.size ())) * ( (1 - stats->getLinkReliability (*it,layer)) / sum_reliabilities / normalization_value);
    }
    else
    {
      table(determineRowOfFace(*it), layer) = weightedUtil -
          utf * (weightedUtil / sum_fwProbs) * ( (1 - stats->getLinkReliability (*it,layer)) / sum_reliabilities / normalization_value);
    }
  }
}

void ForwardingProbabilityTable::probeColumn(std::vector<int> faces, int layer, Ptr<ForwardingStatistics> stats, bool useDroppingProbabilityFromFWT)
{
  NS_LOG_DEBUG("PROBING");

  if(faces.size () == 0)
    return;

   double probe = 0.0;

  if(useDroppingProbabilityFromFWT)
    probe = table(determineRowOfFace (DROP_FACE_ID), layer) * ParameterConfiguration::getInstance ()->getParameter ("PROBING_TRAFFIC");
  else
    probe = calcWeightedUtilization(DROP_FACE_ID,layer,stats) * ParameterConfiguration::getInstance ()->getParameter ("PROBING_TRAFFIC");

  if(probe < 0.001) // if probe is zero return
    return;

  if(useDroppingProbabilityFromFWT)
    table(determineRowOfFace (DROP_FACE_ID), layer) -= probe;
  else
    table(determineRowOfFace (DROP_FACE_ID), layer) = calcWeightedUtilization(DROP_FACE_ID,layer,stats) - probe;

  //split the probe (forwarding percents)....
  for(std::vector<int>::iterator it = faces.begin(); it != faces.end(); ++it) // for each ur_face
  {

    //table(determineRowOfFace (*it), layer) = calcWeightedUtilization(*it,layer,stats) + (probe / ((double)faces.size ()));


    //norm factor
    //fprintf(stderr, "probe=%f\n", probe);
    double normFactor = 0.0;
    for(std::vector<int>::iterator it = faces.begin(); it != faces.end(); ++it) // for each ur_face
    {
      normFactor += table(determineRowOfFace (*it), 0);
    }

    for(std::vector<int>::iterator it = faces.begin(); it != faces.end(); ++it) // for each ur_face
    {
      //table(determineRowOfFace (*it), layer) = calcWeightedUtilization(*it,layer,stats) + (probe / ((double)faces.size ()));
      if(layer == 0 || normFactor == 0)
        table(determineRowOfFace (*it), layer) = calcWeightedUtilization(*it,layer,stats) + (probe / ((double)faces.size ()));
      else
      {
        //fprintf(stderr, "probefraction =%f\n", (table(determineRowOfFace (*it), 0) / normFactor));
        //fprintf(stderr, "value =%f\n", probe * (table(determineRowOfFace (*it), 0) / normFactor));
        table(determineRowOfFace (*it), layer) =
            calcWeightedUtilization(*it,layer,stats) + (probe * (table(determineRowOfFace (*it), 0) / normFactor));
      }
    }

  }
}

void ForwardingProbabilityTable::shiftDroppingTraffic(std::vector<int> faces, int layer,Ptr<ForwardingStatistics> stats)
{
  //calcualte how much traffic we can take
  double interests_to_shift = 0;
  for(std::vector<int>::iterator it = faces.begin(); it != faces.end(); ++it) // for each r_face
  {
    interests_to_shift += stats->getForwardedInterests (*it, layer);
  }

  interests_to_shift /= (double)stats->getTotalForwardedInterests (layer);
  interests_to_shift *= ParameterConfiguration::getInstance ()->getParameter ("SHIFT_TRAFFIC");

  //double dropped_interests = stats->getForwardedInterests(DROP_FACE_ID, layer);
  double dropped_interests = calcWeightedUtilization(DROP_FACE_ID,layer,stats);

  if(dropped_interests <= interests_to_shift)
  {
    interests_to_shift = dropped_interests;
  }

  NS_LOG_DEBUG("shiftDroppingTraffic utf = " << interests_to_shift);

  table(determineRowOfFace(DROP_FACE_ID), layer) = calcWeightedUtilization(DROP_FACE_ID,layer,stats) - interests_to_shift;
  updateColumn (faces, layer,stats,interests_to_shift,true);

}

double ForwardingProbabilityTable::getSumOfWeightedForwardingProbabilities(std::vector<int> set_of_faces, int layer, Ptr<ForwardingStatistics> stats)
{
  double sum = 0.0;
  for(std::vector<int>::iterator it = set_of_faces.begin(); it != set_of_faces.end(); ++it)
  {
    //sum += table(determineRowOfFace (*it), layer);
    sum += sum += calcWeightedUtilization(*it,layer,stats);
  }
  return sum;
}

double ForwardingProbabilityTable::getSumOfActualForwardingProbabilities(std::vector<int> set_of_faces, int layer, Ptr<ForwardingStatistics> stats)
{
  double sum = 0.0;
  for(std::vector<int>::iterator it = set_of_faces.begin(); it != set_of_faces.end(); ++it)
  {
    sum += stats->getActualForwardingProbability (*it, layer);
  }
  return sum;
}

void ForwardingProbabilityTable::syncDroppingPolicy(Ptr<ForwardingStatistics> stats)
{
  NS_LOG_DEBUG("syncDroppingPolicy before update:\n" <<table);

  //set all layers as not jammed
  for(int i=0; i < (int)ParameterConfiguration::getInstance ()->getParameter ("MAX_LAYERS"); i++)
    jammed[i] = false;

  int first = getFirstDroppingLayer ();
  int last = getLastDroppingLayer ();

  //jamm all layers until > last all layers

  if(first < last)
  {
    for(int i= (int)ParameterConfiguration::getInstance ()->getParameter ("MAX_LAYERS") -1; i > last; i--)
      jammed[i] = true;
  }

  //int iteration = 0;

  while(first < last)
  {

    /*iteration++;
    if(iteration > 6)
      exit(-1);

    fprintf(stderr, "first = %d\n", first);
    fprintf(stderr, "last = %d\n", last);*/

    //set last as jammed
    jammed[last] = true;

    // interest forwared to the dropping face of first
    double theta = table(determineRowOfFace (DROP_FACE_ID), first) * stats->getTotalForwardedInterests (first);

    //max traffic that can be shifted towards face(s) of last
    double chi = (1.0 - table(determineRowOfFace (DROP_FACE_ID), last)) * stats->getTotalForwardedInterests (last);

    //fprintf(stderr, "theta = %f\n", theta);
    //fprintf(stderr, "chi = %f\n", chi);

    if (theta == 0)
    {
      for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it)
      {
        if(*it == DROP_FACE_ID)
          table(determineRowOfFace (*it), first) = 0;
        else
          table(determineRowOfFace (*it), first) = 1.0 / (faceIds.size ()-1);
      }
    }
    else if(chi == 0) // nothing has been forwarded via the last face, set drop prob to 100%
    {
      for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it)
      {
        if(*it == DROP_FACE_ID)
          table(determineRowOfFace (*it), last) = 1;
        else
          table(determineRowOfFace (*it), last) = 0;
      }
    }
    else
    {
      if(chi > theta) //if we can shift more than we need to, we just shift how much we need
        chi = theta;

      //reduce dropping prob for lower layer
      table(determineRowOfFace (DROP_FACE_ID), first) -= (chi / stats->getTotalForwardedInterests (first));

      //increase dropping prob for higher layer
      table(determineRowOfFace (DROP_FACE_ID), last)  += (chi / stats->getTotalForwardedInterests (last));

      //calc n_frist , n_last normalization value without dropping face;
      double n_first = 0;
      double n_last = 0;

      for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it)
      {
        if(*it == DROP_FACE_ID)
          continue;

        n_first += table(determineRowOfFace (*it), first);
        n_last +=  table(determineRowOfFace (*it), last);
      }

      //increase & decrease other faces accordingly
      for(std::vector<int>::iterator it = faceIds.begin(); it != faceIds.end(); ++it)
      {
        if(*it == DROP_FACE_ID)
          continue;

        table(determineRowOfFace (*it), first) += (chi/stats->getTotalForwardedInterests (first)) * (table(determineRowOfFace (*it), first)/n_first);
        table(determineRowOfFace (*it), last) -= (chi/stats->getTotalForwardedInterests (last)) * (table(determineRowOfFace (*it), last)/n_last);
      }
    }

    first = getFirstDroppingLayer ();
    last = getLastDroppingLayer ();
  }

  NS_LOG_DEBUG("Forwarding Matrix after syncDroppingPolicy:\n" << table);
  table = normalizeColumns(table);
  NS_LOG_DEBUG("Forwarding Matrix after normalization:\n" << table);

  //TODO
  //********
  /*for(int i = 0; i < (int)ParameterConfiguration::getInstance ()->getParameter ("MAX_LAYERS"); i++) // set all as non-jammed
    jammed[i] = false;
  NS_LOG_DEBUG("JAMMING DISABLED FOR NOW");
  */
  //********

  for(int i = 0; i < jammed.size (); i++)
    NS_LOG_DEBUG("jammed[" << i << "]="<< jammed[i]);
}

int ForwardingProbabilityTable::getFirstDroppingLayer()
{
  for(int i = 0; i < (int)ParameterConfiguration::getInstance ()->getParameter ("MAX_LAYERS"); i++) // for each layer
  {
    if(table(determineRowOfFace (DROP_FACE_ID), i) > 0.0)
      return i;
  }
  return (int)ParameterConfiguration::getInstance ()->getParameter ("MAX_LAYERS")-1;
}

int ForwardingProbabilityTable::getLastDroppingLayer()
{
  for(int i = (int)ParameterConfiguration::getInstance ()->getParameter ("MAX_LAYERS") - 1; i >= 0; i--) // for each layer
  {
    if(table(determineRowOfFace (DROP_FACE_ID), i) < 1.0)
      return i;
  }

  return 0;
}

void ForwardingProbabilityTable::addFace (int faceId)
{
  if(determineRowOfFace (faceId) != FACE_NOT_FOUND)
  {
    NS_LOG_UNCOND("Trying to add already known face: " << faceId);
    return;
  }
}

void ForwardingProbabilityTable::removeFace (int faceId)
{
  if(determineRowOfFace (faceId) == FACE_NOT_FOUND)
  {
    NS_LOG_UNCOND("Trying to remove unknown face: " << faceId);
    return;
  }
  table = removeFaceFromTable (faceId, table, faceIds);

  //remove from vector...
  for(std::vector<int>::iterator it = faceIds.begin (); it != faceIds.end (); ++it)
  {
    if(*it == faceId)
    {
      faceIds.erase (it);
      std::sort(this->faceIds.begin(), this->faceIds.end());
      return;
    }
  }
  NS_LOG_UNCOND("Could not erase from vector: " << faceId);
}

double ForwardingProbabilityTable::calcWeightedUtilization(int faceId, int layer, Ptr<ForwardingStatistics> stats)
{
  double actual = stats->getActualForwardingProbability (faceId,layer);
  double old = table(determineRowOfFace (faceId), layer);

  return old + ( (actual - old) * ParameterConfiguration::getInstance ()->getParameter ("ALPHA") );
}

void ForwardingProbabilityTable::increaseReliabilityThreshold()
{
  updateReliabilityThreshold (true);
}

void ForwardingProbabilityTable::decreaseReliabilityThreshold()
{
  updateReliabilityThreshold (false);
}

void ForwardingProbabilityTable::updateReliabilityThreshold(bool mode)
{
  ParameterConfiguration *p = ParameterConfiguration::getInstance ();

  double new_t = 0.0;

  if(mode)
    new_t = curReliability + ((p->getParameter("RELIABILITY_THRESHOLD_MAX") - curReliability) * p->getParameter("ALPHA"));
  else
    new_t = curReliability - ((curReliability - p->getParameter("RELIABILITY_THRESHOLD_MIN")) * p->getParameter("ALPHA"));

  if(new_t > p->getParameter("RELIABILITY_THRESHOLD_MAX"))
    new_t = p->getParameter("RELIABILITY_THRESHOLD_MAX");

  if(new_t < p->getParameter("RELIABILITY_THRESHOLD_MIN"))
    new_t = p->getParameter("RELIABILITY_THRESHOLD_MIN");

  curReliability = new_t;
}


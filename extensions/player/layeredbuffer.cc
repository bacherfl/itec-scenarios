#include "layeredbuffer.h"

#include <stdio.h>

using namespace ns3::player;


LayeredBuffer::LayeredBuffer()
{
}



bool LayeredBuffer::AddToBuffer(unsigned int segment_number, unsigned int segment_level)
{
  LayerSegmentMap::iterator it = m_buffer.find (segment_level);

  // is segment_level in our buffer?
  if (it == m_buffer.end())
  { // if not, add it
    m_buffer.insert (LayerSegmentMap::value_type(segment_level, vector<unsigned int>()));
  }

  // add segment number to the vector
  m_buffer[segment_level].push_back(segment_number);

  // we can always return true here for now
  return true;
}

bool LayeredBuffer::IsInBuffer(unsigned int segment_number, unsigned int segment_level)
{
  LayerSegmentMap::iterator it = m_buffer.find (segment_level);

  // is segment_level in our buffer?
  if (it != m_buffer.end())
  {
    // find segment_number in buffer_for_level
    for (int i = 0; i < m_buffer[segment_level].size (); i++)
    {
      // is current element equal to segment_number?
      if (m_buffer[segment_level].at(i) == segment_number)
        return true;
    }
  }
  // else: not found
  return false;
}

bool LayeredBuffer::ConsumeFromBuffer(unsigned int segment_number, unsigned int segment_level)
{
  LayerSegmentMap::iterator it = m_buffer.find (segment_level);

  bool doDelete = false;

  // is segment_level in our buffer?
  if (it != m_buffer.end())
  {
    int i = 0;
    // find segment_number in buffer_for_level
    for (i = 0; i < m_buffer[segment_level].size (); i++)
    {
      // is current element equal to segment_number?
      if (m_buffer[segment_level].at(i) == segment_number)
      {
        doDelete = true;
        break;
      }
    }

    // make sure to delete that element (we only do this outside of the for loop
    // to not interfere with any pointer/looping that is going on
    if (doDelete)
    {
      // erase the i-th element
      m_buffer[segment_level].erase(m_buffer[segment_level].begin()+i);

      return true;
    }

  }
  // else: not found
  return false;
}



unsigned int LayeredBuffer::BufferSize(unsigned int segment_level)
{
  LayerSegmentMap::iterator it = m_buffer.find (segment_level);

  // is segment_level in our buffer?
  if (it != m_buffer.end())
  {
    return m_buffer[segment_level].size ();
  }
  // else: not found
  return 0;
}


// are they really orderd?
unsigned int LayeredBuffer::LastSegmentNumber(unsigned int segment_level)
{
  return m_buffer[segment_level].back();
}

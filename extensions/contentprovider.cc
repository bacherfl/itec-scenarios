#include "contentprovider.h"

namespace ns3{
  NS_OBJECT_ENSURE_REGISTERED (ContentProvider);

  // register NS-3 type
  TypeId ContentProvider::GetTypeId ()
  {
    static TypeId tid = TypeId ("ContentProvider")
        .SetParent<ndn::App>()
        .AddConstructor<ContentProvider>()
        .AddAttribute("ContentPath",
                      "Path where the offered content is stored on the file system.",
                      StringValue("/path/to/file"),
                      MakeStringAccessor(&ContentProvider::content_path),
                      MakeStringChecker ())
        .AddAttribute("Prefix",
                      "NDN content prefix.",
                      StringValue("/itec/"),
                      MakeStringAccessor(&ContentProvider::ndn_prefix),
                      MakeStringChecker ());
    return tid;
  }
}

using namespace ns3;

void ContentProvider::StartApplication ()
{
  NS_LOG_FUNCTION(this);

  // initialize ndn::App
  ndn::App::StartApplication ();

  //set homedir
  struct passwd *pw = getpwuid(getuid());
  content_path = std::string(pw->pw_dir).append(content_path);

  NS_LOG_INFO("Contentprovider::ContentPath: " << content_path);
  NS_LOG_INFO("Contentprovider::NDNPrefix: " << ndn_prefix);

  Ptr<ndn::Name> prefix = Create<ndn::Name> (ndn_prefix);

  //Creating FIB entry that ensures that we will receive incoming Interests
  Ptr<ndn::Fib> fib = GetNode ()->GetObject<ndn::Fib> ();
  Ptr<ndn::fib::Entry> fibEntry = fib->Add (*prefix, m_face, 0);
}

void ContentProvider::StopApplication ()
{
  NS_LOG_FUNCTION(this);
  ndn::App::StopApplication ();
}

void ContentProvider::OnInterest (Ptr<const ndn::Interest> interest)
{
  NS_LOG_FUNCTION("Contentprovider::OnInterest" << interest->GetName() << ", " << this);

  ndn::App::OnInterest (interest);

  std::string fname = interest->GetName().toUri();  // get the uri from interest
  fname = fname.substr(ndn_prefix.length(), fname.length()); // remove the prefix
  fname = std::string(content_path).append(fname); // prepend the data path
  std::string chunk_nr = fname.substr(fname.rfind ("/chunk_")+1).substr (6); // extract the chunk number remove .../chunk_X
  fname = fname.substr (0, fname.find_last_of ("/"));//remove "/chunk*"

  //fprintf(stderr, "FNAME = %s\n",fname.c_str ());
  //fprintf(stderr, "chunk_nr =%s\n", chunk_nr.c_str ());

  //remove virtual dataset identifier if exist...
  //fname = fname.substr (0, fname.rfind ("-set"));

  struct stat fstats;
  if(!(stat (fname.c_str(), &fstats) == 0))
  {
    //fprintf(stderr, "ContentProvider::OnInterest: File does NOT exist: %s\n", fname.c_str ());
    return;
  }

  int size = fstats.st_size;

  Ptr<ndn::Data> data;

  int result;
  std::stringstream(chunk_nr) >> result;

  if (result == 0)
  {
    // return the file size only with the first chunk (as a uint8_t array)
    data = Create<ndn::Data> (Create<Packet> ((uint8_t*)&size, sizeof(size)));
    data->SetName (Create<ndn::Name> (interest->GetName ())); // data will have the same name as Interests

    NS_LOG_FUNCTION("Sending first data packet" << data->GetName() << "with content " << size << this);
  }
  else
  {
    // return an actual data packet

    size -= MAX_PACKET_PAYLOAD * (result-1);

    if(size > MAX_PACKET_PAYLOAD)
      size = MAX_PACKET_PAYLOAD;

    if(size < 0)
    {
      // this isn't bad, it means that the client requested more files than we had.
      NS_LOG_INFO("INVALID FILE REQUEST. FILE IS SMALLER THAN REQUESTED\n");
      return;
    }

    data = Create<ndn::Data> (Create<Packet> (size));
    data->SetName (Create<ndn::Name> (interest->GetName ())); // data will have the same name as Interests

    // set svc level tag
    ns3::ndn::SVCLevelTag tag;
    Ptr<Packet> packet = ndn::Wire::FromInterest(interest);

    if (packet->PeekPacketTag(tag))
    {
      data->GetPayload()->AddPacketTag(tag);
    }

    NS_LOG_FUNCTION("Sending data packet" << data->GetName() << "size" << size << this);
  }

  // Call trace (for logging purposes)
  m_transmittedDatas (data, this, m_face);
  m_face->ReceiveData (data);
}

void ContentProvider::OnData (Ptr<const ndn::Data> contentObject)
{
  NS_LOG_FUNCTION(contentObject->GetName () << this);
}

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
  ndn::App::StopApplication ();
}

void ContentProvider::OnInterest (Ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest (interest);

  NS_LOG_INFO ("ContentProvider::OnInterest: Received Interest " << interest->GetName());

  std::string fname = interest->GetName().toUri();
  fname = fname.substr(ndn_prefix.length(), fname.length());
  fname = std::string(content_path).append(fname);

  struct stat fstats;
  if(!(stat (fname.c_str(), &fstats) == 0))
  {
    NS_LOG_WARN("ContentProvider::OnInterest: File does NOT exist: " << fname);
    return;
  }

  Ptr<ndn::Data> data = Create<ndn::Data> (Create<Packet> (fstats.st_size));
  data->SetName (Create<ndn::Name> (interest->GetName ())); // data will have the same name as Interests

  NS_LOG_INFO ("ContentProvider::OnInterest: Sending data packet " << data->GetName() << " payload " << fstats.st_size);

  // Call trace (for logging purposes)
  m_transmittedDatas (data, this, m_face);
  m_face->ReceiveData (data);
}

void ContentProvider::OnData (Ptr<const ndn::Data> contentObject)
{
}

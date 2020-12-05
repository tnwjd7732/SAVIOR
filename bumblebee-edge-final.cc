
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "ns3/mobility-module.h"
#include "ns3/constant-velocity-mobility-model.h"
#include "ns3/ns2-mobility-helper.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"

#include "ns3/wave-mac-helper.h"
#include "ns3/wifi-80211p-helper.h"


using namespace ns3;

/*Wifi app - (pedestrian side)*/
class MyApp : public Application 
{
public:

  MyApp ();
  virtual ~MyApp();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer; //packet destination

  uint32_t        m_packetSize; 
  uint32_t        m_nPackets; 
  DataRate        m_dataRate; 
  EventId         m_sendEvent; 
  bool            m_running; //is running?
  uint32_t        m_packetsSent;
  
};

MyApp::MyApp () //All property -> defalut num
  : m_socket (0), 
    m_peer (), 
    m_packetSize (0), 
    m_nPackets (0), 
    m_dataRate (0), 
    m_sendEvent (), 
    m_running (false), 
    m_packetsSent (0)
{
}

MyApp::~MyApp()
{
  m_socket = 0;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
MyApp::StartApplication (void) 
{

  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket (); //send
}

void 
MyApp::StopApplication (void)
{

  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void 
MyApp::SendPacket (void)
{

  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);
  NS_LOG_UNCOND("[pedestrian] WiFi packet sent time "<<Simulator::Now().GetSeconds());
 
  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();

    }
}

void 
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
     // Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      Time tNext (Seconds(1.0));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}


/*WAVE app - (edge side)*/
class MyApp2 : public Application 
{
public:

  MyApp2 ();
  virtual ~MyApp2();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer; //packet destination

  uint32_t        m_packetSize; 
  uint32_t        m_nPackets; 
  DataRate        m_dataRate; 
  EventId         m_sendEvent; 
  bool            m_running; //is running?
  uint32_t        m_packetsSent;
  
};

MyApp2::MyApp2 () //All property -> defalut num
  : m_socket (0), 
    m_peer (), 
    m_packetSize (0), 
    m_nPackets (0), 
    m_dataRate (0), 
    m_sendEvent (), 
    m_running (false), 
    m_packetsSent (0)
{
}

MyApp2::~MyApp2()
{
  m_socket = 0;
}

void
MyApp2::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
MyApp2::StartApplication (void) 
{
  //NS_LOG_FUNCTION(this);
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket (); //send
}

void 
MyApp2::StopApplication (void)
{
  //NS_LOG_FUNCTION(this);
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void 
MyApp2::SendPacket (void)
{

  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);
  NS_LOG_UNCOND("[edge] WAVE packet sent time "<<Simulator::Now().GetSeconds());
 
  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();

    }
}

void 
MyApp2::ScheduleTx (void)
{
  if (m_running)
    {
     // Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      Time tNext (Seconds(1.0));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp2::SendPacket, this);
    }
}


double p2eaverage_delay;
double p2etotal_delay=0;
int p2ecount=0;
double num=1.0;

void SinkRxTrace(std::string context, Ptr<const Packet> pkt, const Address &addr){
 // std::cout<<"Packet received at "<<Simulator::Now().GetSeconds()<<"s\n";
  double delay=Simulator::Now().GetSeconds();

  p2etotal_delay+=delay;
  p2ecount++;


  //std::cout<<"total delay is "<<total_delay<<"s\n";
}

double e2vaverage_delay;
double e2vtotal_delay=0;
int e2vcount=0;

void SinkRxTrace2(std::string context, Ptr<const Packet> pkt, const Address &addr){
 // std::cout<<"Packet received at "<<Simulator::Now().GetSeconds()<<"s\n";
  double delay=Simulator::Now().GetSeconds();
  int delay_int=delay/1;
  delay-=delay_int;

  e2vtotal_delay+=delay;
  e2vcount++;


  //std::cout<<"total delay is "<<total_delay<<"s\n";
}

  AnimationInterface * pAnim = 0;

int main (int argc, char *argv[])
{

  NS_LOG_COMPONENT_DEFINE ("bumblebee Project");
  std::string animFile = "1121_edge_final.xml";
  std::string traceFile_ped;
  std::string traceFile_veh;
  std::string logFile;
  int    nodeNum_ped;
  int    nodeNum_veh;
  double duration; 

  

  std::string phyMode ("OfdmRate6MbpsBW10MHz");
// LogComponentEnableAll(LOG_LEVEL_INFO);
// Parse command line attribute
  CommandLine cmd;
  cmd.AddValue ("traceFile_ped", "Ns2 movement trace file", traceFile_ped);
  cmd.AddValue ("nodeNum_ped", "Number of pedestrian nodes", nodeNum_ped);
  cmd.AddValue ("traceFile_veh", "Ns2 movement trace file", traceFile_veh);
  cmd.AddValue ("nodeNum_veh", "Number of nodes", nodeNum_veh);
  cmd.AddValue ("duration", "Duration of Simulation", duration);
  cmd.AddValue ("logFile", "Log file", logFile);
  cmd.Parse (argc,argv);

  Time::SetResolution (Time::NS);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO); //Packet sink 
  LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO); //Packet sink 

  // Check command line arguments
  if (traceFile_ped.empty () || nodeNum_ped <= 0 || traceFile_veh.empty () || nodeNum_veh <= 0 || duration <= 0 || logFile.empty ())
    {
      std::cout << "Usage of " << argv[0] << " :\n\n"
      "./waf --run \"scratch/bumblebee_proj"
      " --traceFile=src/mobility/examples/default.ns_movements"
      " --nodeNum=2 --duration=100.0 --logFile=ns2-mob.log\" \n\n"
      "NOTE: ns2-traces-file could be an absolute or relative path. You could use the file default.ns_movements\n"
      "      included in the same directory of this example file.\n\n"
      "NOTE 2: Number of nodes present in the trace file must match with the command line argument and must\n"
      "        be a positive number. Note that you must know it before to be able to load it.\n\n"
      "NOTE 3: Duration must be a positive number. Note that you must know it before to be able to load it.\n\n";

      return 0;
    }
//-------------------------------set mobility model-----------------------------------



  Ns2MobilityHelper ns2_ped = Ns2MobilityHelper (traceFile_ped);
  NodeContainer pedNodes; //pedestrian node (wifi station)
  pedNodes.Create (nodeNum_ped);
  ns2_ped.Install (); // configure movements for each node, while reading trace file


  Ns2MobilityHelper ns2_veh = Ns2MobilityHelper (traceFile_veh);
  NodeContainer vehNodes;
  vehNodes.Create (nodeNum_veh);
  ns2_veh.Install (); 

 
  /*create all nodes*/ 
  NodeContainer edgeNode;
  edgeNode.Create(1); //edge server = wifi AP 
  
  MobilityHelper edgemobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(0, 0.0, 0.0));
  edgemobility.SetPositionAllocator(positionAlloc);
  edgemobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  edgemobility.Install(edgeNode);

//----------------------------Wifi channel, Phy, Mac layer-------------------------

  /*WiFi channel & phy layer*/
  YansWifiChannelHelper channel;
  channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  channel.AddPropagationLoss ("ns3::RangePropagationLossModel","MaxRange",DoubleValue (250));
  //phy layer
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  //setting communication range -> 300m ?
  phy.Set("RxGain", DoubleValue(-10));
  phy.Set("TxPowerStart", DoubleValue(33));
  phy.Set("TxPowerEnd", DoubleValue(33));


  /*Wifi Mac layer*/
  WifiMacHelper mac;
  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                      "DataMode",StringValue (phyMode),
                                      "ControlMode",StringValue (phyMode));
  //mac.SetType("ns3::AdhocWifiMac");
  NetDeviceContainer pedDevices= wifi.Install (phy, mac, pedNodes);
  NetDeviceContainer edgeDevices = wifi.Install (phy, mac, edgeNode);


  //wireshark
  phy.EnablePcap("1121wifi_ped", pedDevices); 
  phy.EnablePcap("1121wifi_edge", edgeDevices);

//-------------------------------------- Wave ------------------------------------

  /*WiFi channel & phy layer*/
  YansWifiChannelHelper wchannel;
  wchannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wchannel.AddPropagationLoss ("ns3::RangePropagationLossModel","MaxRange",DoubleValue (250));
  //phy layer
  YansWifiPhyHelper wphy = YansWifiPhyHelper::Default ();
  wphy.SetChannel (wchannel.Create ());

  //setting communication range -> 300m ?
  wphy.Set("RxGain", DoubleValue(-10));
  wphy.Set("TxPowerStart", DoubleValue(33));
  wphy.Set("TxPowerEnd", DoubleValue(33));

  /*Wave Mac layer*/
  NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
 // wifi80211p.EnableLogComponents ();      // Turn on all Wifi 802.11p logging
  
  wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                      "DataMode",StringValue (phyMode),
                                      "ControlMode",StringValue (phyMode));
  NetDeviceContainer vehDevices = wifi80211p.Install (wphy, wifi80211pMac, vehNodes);
  NetDeviceContainer edgewDevices = wifi80211p.Install (wphy, wifi80211pMac, edgeNode);
  wphy.EnablePcap("wave_edge", edgewDevices);
  wphy.EnablePcap("wave_veh", vehDevices);

//---------------------------------- Network layer --------------------------------

  /*Internet stack*/
  InternetStackHelper stack;
  stack.Install (edgeNode);
  stack.Install (pedNodes);
  stack.Install (vehNodes);

  //Ipv4 - 10.1.1.0 (pedestrian, edge(pedestrian side))
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer pedInterfaces = address.Assign (pedDevices); 
  Ipv4InterfaceContainer edgeInterfaces_wifi = address.Assign (edgeDevices); 
  
  //Ipv4 - 10.1.2.0 (vehicle, edge(vehicle side))
  Ipv4AddressHelper address2;
  address2.SetBase ("10.1.2.0", "255.255.255.0"); 
  Ipv4InterfaceContainer vehInterfaces = address2.Assign (vehDevices); 
  Ipv4InterfaceContainer edgeInterfaces_wave = address2.Assign (edgewDevices); 


  /*Application install*/
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");


//--------------------------- Application:: pedestrian -> edge ---------------------

  uint32_t m_port = 9;

//pedestrian (Send to edge - unicast)
  OnOffHelper onoff1 ("ns3::UdpSocketFactory", InetSocketAddress(edgeInterfaces_wifi.GetAddress(0), m_port));
  onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff1.SetAttribute ("DataRate", StringValue (StringValue ("144bps")));


for(int i=0; i<nodeNum_ped; i++){
  onoff1.SetAttribute ("PacketSize", UintegerValue (18));
  ApplicationContainer onoffApp1 = onoff1.Install(pedNodes.Get(i));
  onoffApp1.Start (Seconds (0.0+0.01*i));
  onoffApp1.Stop (Seconds (30.0));
}




//edge (Receive from pedestrian)
  PacketSinkHelper sink1 ("ns3::UdpSocketFactory", 
InetSocketAddress(Ipv4Address::GetAny(), 9));

  ApplicationContainer sinkApp1 = sink1.Install(edgeNode.Get(0));
  Ptr<PacketSink> pktSink=StaticCast<PacketSink> (sinkApp1.Get(0));
  std::stringstream ss;
  ss<<"Some information";
  pktSink->TraceConnect("Rx", ss.str(), MakeCallback(&SinkRxTrace));

  sinkApp1.Start (Seconds (0.0));
  sinkApp1.Stop (Seconds (30.0));



//------------------------------- Application:: edge -> vehicle ---------------------


  //vehicle (receive from edge)
  for (int u = 0; u < nodeNum_veh ; u++)
    {
      //Address sinkAddress (InetSocketAddress (edgeInterfaces_wave.GetAddress(0), 80);
      PacketSinkHelper PacketSinkHelper_Rx ("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 80));
      ApplicationContainer sinkApp_wave=PacketSinkHelper_Rx.Install(vehNodes.Get(u));

      Ptr<PacketSink> pktSink2=StaticCast<PacketSink> (sinkApp_wave.Get(0));
      std::stringstream ss;
      ss<<"Some information";
      pktSink2->TraceConnect("Rx", ss.str(), MakeCallback(&SinkRxTrace2));

      sinkApp_wave.Start(Seconds(0.0));
      sinkApp_wave.Stop(Seconds(30.0));
      
    }

   //edge (broadcast to vehicle)
   InetSocketAddress remote = InetSocketAddress(Ipv4Address("10.1.2.255"), 80);
   Ptr<Socket> ns3UdpSocketw=Socket::CreateSocket(edgeNode.Get(0), UdpSocketFactory::GetTypeId()); 

   ns3UdpSocketw->SetAllowBroadcast(true);
   ns3UdpSocketw->Connect(remote);

   Ptr<MyApp2> app_wave=CreateObject<MyApp2>();
   app_wave->Setup(ns3UdpSocketw, remote, 450, 1000, DataRate("144bps"));
   edgeNode.Get(0)->AddApplication(app_wave);
   app_wave->SetStartTime(Seconds(1.0));
   app_wave->SetStopTime(Seconds(30.0));


//-------------------------------------------------------------------------------------

  pAnim = new AnimationInterface (animFile); //xml file(netanim)
  pAnim->EnablePacketMetadata (true);
  //pAnim->TrackIpv4L3ProtocolCounters();
  pAnim->EnableIpv4L3ProtocolCounters(Seconds (0.0),Seconds(10.0),Seconds(1));
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (30.0));
  Simulator::Run ();

  Simulator::Destroy ();

 // total_delay=total_delay-(0.06*(count/4)); //pedestrian 4 (test)
  p2etotal_delay=(p2etotal_delay-10962); //pedestrian 25

  p2eaverage_delay=p2etotal_delay/p2ecount;
  e2vaverage_delay=e2vtotal_delay/e2vcount;

  std::cout<<"Average Propagation Delay (pedestrian --> Edge server): "<<p2eaverage_delay<<"sec\n\n";



  std::cout<<"Average Propagation Delay (Edge server --> vehicles): "<<e2vaverage_delay<<"sec\n";

  return 0;
}



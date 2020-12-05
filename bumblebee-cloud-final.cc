/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
 /*
  * Copyright (c) 2006,2007 INRIA
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License version 2 as
  * published by the Free Software Foundation;
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  */ 
 
 #include "ns3/core-module.h"
 #include "ns3/network-module.h"
 #include "ns3/internet-module.h"
 #include "ns3/point-to-point-module.h"
 #include "ns3/mobility-module.h"
 #include "ns3/netanim-module.h"
 #include "ns3/wifi-module.h"
 #include "ns3/point-to-point-grid.h"
 #include "ns3/wave-mac-helper.h"
 #include "ns3/wifi-80211p-helper.h"
 #include "ns3/packet-sink-helper.h"
 #include "ns3/on-off-helper.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-nix-vector-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
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
  NS_LOG_UNCOND("At time "<<Simulator::Now().GetSeconds()<<" sec Central cloud server sent packet to local router");
 
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


double router_average_delay;
double router_total_delay=0;
int router_count=0;

void SinkRxTrace(std::string context, Ptr<const Packet> pkt, const Address &addr){
  //std::cout<<"Packet received at "<<Simulator::Now().GetSeconds()<<"s\n";
  double delay=Simulator::Now().GetSeconds();
  int delay_int=delay/1;
  delay-=delay_int;

  router_total_delay+=delay;
  router_count++;
}


double cloud_average_delay;
double cloud_total_delay=0;
int cloud_count=0;


void SinkRxTrace2(std::string context, Ptr<const Packet> pkt, const Address &addr){
  //std::cout<<"Packet received at "<<Simulator::Now().GetSeconds()<<"s\n";
  double delay=Simulator::Now().GetSeconds();
//  int delay_int=delay/1;

  cloud_total_delay+=delay;
  cloud_count++;
//  std::cout<<"!!!!now "<<delay<<"s\n";
 // std::cout<<"!!!! total delay is "<<cloud_total_delay<<"s\n";
}


 int main (int argc, char *argv[])
 {
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO); //Packet sink 
  LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO); //Packet sink 

  CommandLine cmd ;
  cmd.Parse (argc, argv);
   
  std::string animFile = "1121_Cloud_final.xml";
   
  NodeContainer Node;
  Node.Create(9); //node 0-7: local router, node 8: centralized cloud server
  
  //for global routing
  Ipv4NixVectorHelper nixRouting;
  Ipv4StaticRoutingHelper staticRouting;
  Ipv4ListRoutingHelper list;

  list.Add(staticRouting, 0);
  list.Add(nixRouting, 10);
      
   MobilityHelper mob_sg;
   Ptr<ListPositionAllocator> positionAlloc_sg = CreateObject<ListPositionAllocator>();
   mob_sg.SetPositionAllocator(positionAlloc_sg);
   mob_sg.SetMobilityModel("ns3::ConstantPositionMobilityModel");
   positionAlloc_sg->Add(Vector(0.0, 0.0, 0.0));

   mob_sg.Install(Node.Get(0));
   
   MobilityHelper mob_gw;
   Ptr<ListPositionAllocator> positionAlloc_gw = CreateObject<ListPositionAllocator>();
   mob_gw.SetPositionAllocator(positionAlloc_gw);
   mob_gw.SetMobilityModel("ns3::ConstantPositionMobilityModel");
   positionAlloc_gw->Add(Vector(150000, 0.0, 0.0));

   mob_gw.Install(Node.Get(1));
   
   MobilityHelper mob_cn;
   Ptr<ListPositionAllocator> positionAlloc_cn = CreateObject<ListPositionAllocator>();
   mob_cn.SetPositionAllocator(positionAlloc_cn);
   mob_cn.SetMobilityModel("ns3::ConstantPositionMobilityModel");
   positionAlloc_cn->Add(Vector(0, 150000, 0.0));
   
   mob_cn.Install(Node.Get(2));

   MobilityHelper mob_cb;
   Ptr<ListPositionAllocator> positionAlloc_cb = CreateObject<ListPositionAllocator>();
   mob_cb.SetPositionAllocator(positionAlloc_cb);
   mob_cb.SetMobilityModel("ns3::ConstantPositionMobilityModel");
   positionAlloc_cb->Add(Vector(150000, 150000, 0.0));
 
   mob_cb.Install(Node.Get(3));
  
   MobilityHelper mob_jb;
   Ptr<ListPositionAllocator> positionAlloc_jb = CreateObject<ListPositionAllocator>();
   mob_jb.SetPositionAllocator(positionAlloc_jb);
   mob_jb.SetMobilityModel("ns3::ConstantPositionMobilityModel");
   positionAlloc_jb->Add(Vector(0, 300000, 0.0));
 
   mob_jb.Install(Node.Get(4));
  
   MobilityHelper mob_gb;
   Ptr<ListPositionAllocator> positionAlloc_gb = CreateObject<ListPositionAllocator>();
   mob_gb.SetPositionAllocator(positionAlloc_gb);
   mob_gb.SetMobilityModel("ns3::ConstantPositionMobilityModel");
   positionAlloc_gb->Add(Vector(150000, 300000, 0.0));

   mob_gb.Install(Node.Get(5));
  
   MobilityHelper mob_jn;
   Ptr<ListPositionAllocator> positionAlloc_jn = CreateObject<ListPositionAllocator>();
   mob_jn.SetPositionAllocator(positionAlloc_jn);
   mob_jn.SetMobilityModel("ns3::ConstantPositionMobilityModel");
   positionAlloc_jn->Add(Vector(0, 450000, 0.0));

   mob_jn.Install(Node.Get(6));
  
   MobilityHelper mob_gn;
   Ptr<ListPositionAllocator> positionAlloc_gn = CreateObject<ListPositionAllocator>();
   mob_gn.SetPositionAllocator(positionAlloc_gn);
   mob_gn.SetMobilityModel("ns3::ConstantPositionMobilityModel");
   positionAlloc_gn->Add(Vector(150000, 450000, 0.0));

   mob_gn.Install(Node.Get(7));


  MobilityHelper edgemobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(50, 0.0, 0.0));
  edgemobility.SetPositionAllocator(positionAlloc);
  edgemobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  edgemobility.Install(Node.Get(8)); //edge
 
  
  NodeContainer r0r1=NodeContainer(Node.Get(0), Node.Get(1));
  NodeContainer r2r3=NodeContainer(Node.Get(2), Node.Get(3));
  NodeContainer r4r5=NodeContainer(Node.Get(4), Node.Get(5));
  NodeContainer r6r7=NodeContainer(Node.Get(6), Node.Get(7));
  NodeContainer r0r2=NodeContainer(Node.Get(0), Node.Get(2));
  NodeContainer r2r4=NodeContainer(Node.Get(2), Node.Get(4));
  NodeContainer r4r6=NodeContainer(Node.Get(4), Node.Get(6));
  NodeContainer r1r3=NodeContainer(Node.Get(1), Node.Get(3));
  NodeContainer r3r5=NodeContainer(Node.Get(3), Node.Get(5));
  NodeContainer r5r7=NodeContainer(Node.Get(5), Node.Get(7));



  InternetStackHelper stack;
  stack.SetRoutingHelper(list); //for routing
  stack.Install (Node);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer routerDevs_12=pointToPoint.Install(r0r1);
  NetDeviceContainer routerDevs_34=pointToPoint.Install(r2r3);
  NetDeviceContainer routerDevs_56=pointToPoint.Install(r4r5);
  NetDeviceContainer routerDevs_78=pointToPoint.Install(r6r7);
  NetDeviceContainer routerDevs_13=pointToPoint.Install(r0r2);
  NetDeviceContainer routerDevs_35=pointToPoint.Install(r2r4);
  NetDeviceContainer routerDevs_57=pointToPoint.Install(r4r6);
  NetDeviceContainer routerDevs_24=pointToPoint.Install(r1r3);
  NetDeviceContainer routerDevs_46=pointToPoint.Install(r3r5);
  NetDeviceContainer routerDevs_68=pointToPoint.Install(r5r7);

  
  



  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer rInterfaces1 =address.Assign(routerDevs_12);
  Ipv4InterfaceContainer rInterfaces2 =address.Assign(routerDevs_34);
  Ipv4InterfaceContainer  rInterfaces3 =address.Assign(routerDevs_56);
  Ipv4InterfaceContainer rInterfaces4 =address.Assign(routerDevs_78);
  Ipv4InterfaceContainer rInterfaces5 =address.Assign(routerDevs_13);
  Ipv4InterfaceContainer  rInterfaces6 =address.Assign(routerDevs_35);
  Ipv4InterfaceContainer  rInterfaces7 =address.Assign(routerDevs_57);
  Ipv4InterfaceContainer rInterfaces8 =address.Assign(routerDevs_24);
  Ipv4InterfaceContainer rInterfaces9 =address.Assign(routerDevs_46);
  Ipv4InterfaceContainer rInterfaces10 =address.Assign(routerDevs_68);
  
  NodeContainer edge_router;
  edge_router.Add(Node.Get(8));
  edge_router.Add(Node.Get(0));
  NetDeviceContainer erDev;
  erDev = pointToPoint.Install(edge_router);

  Ipv4InterfaceContainer erInterfaces =address.Assign(erDev);
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
  pointToPoint.EnablePcapAll("cloud_p2p");

//local router send to central cloud server=======================================================
  OnOffHelper onoff1 ("ns3::UdpSocketFactory", InetSocketAddress(erInterfaces.GetAddress (0), 8080));
  onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff1.SetAttribute ("DataRate", StringValue (StringValue ("36kbps"))); //250 packets per sec
  onoff1.SetAttribute ("PacketSize", UintegerValue (18));
  ApplicationContainer onoffApp;
  onoffApp.Add(onoff1.Install(Node.Get(0)));
  onoffApp.Start(Seconds(0.0));
  onoffApp.Stop(Seconds(40.0));

  OnOffHelper onoff2 ("ns3::UdpSocketFactory", InetSocketAddress(erInterfaces.GetAddress (0), 8080));
  onoff2.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff2.SetAttribute ("DataRate", StringValue (StringValue ("7200bps"))); //50 packets per sec
  onoff2.SetAttribute ("PacketSize", UintegerValue (18));
  ApplicationContainer onoffApp1;
  onoffApp1.Add(onoff2.Install(Node.Get(1)));
  onoffApp1.Start(Seconds(0.0));
  onoffApp1.Stop(Seconds(40.0));

  OnOffHelper onoff3 ("ns3::UdpSocketFactory", InetSocketAddress(erInterfaces.GetAddress (0), 8080));
  onoff3.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff3.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff3.SetAttribute ("DataRate", StringValue (StringValue ("10800bps"))); //75 packets per sec
  onoff3.SetAttribute ("PacketSize", UintegerValue (18));
  ApplicationContainer onoffApp2;
  onoffApp2.Add(onoff3.Install(Node.Get(2)));
  onoffApp2.Start(Seconds(0.0));
  onoffApp2.Stop(Seconds(40.0));

  OnOffHelper onoff4 ("ns3::UdpSocketFactory", InetSocketAddress(erInterfaces.GetAddress (0), 8080));
  onoff4.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff4.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff4.SetAttribute ("DataRate", StringValue (StringValue ("7200bps"))); //50 packets per sec
  onoff4.SetAttribute ("PacketSize", UintegerValue (18));
  ApplicationContainer onoffApp3;
  onoffApp3.Add(onoff4.Install(Node.Get(3)));
  onoffApp3.Start(Seconds(0.0));
  onoffApp3.Stop(Seconds(40.0));

  OnOffHelper onoff5 ("ns3::UdpSocketFactory", InetSocketAddress(erInterfaces.GetAddress (0), 8080));
  onoff5.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff5.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff5.SetAttribute ("DataRate", StringValue (StringValue ("7200bps"))); //50 packets per sec
  onoff5.SetAttribute ("PacketSize", UintegerValue (18));
  ApplicationContainer onoffApp4;
  onoffApp4.Add(onoff5.Install(Node.Get(4)));
  onoffApp4.Start(Seconds(0.0));
  onoffApp4.Stop(Seconds(40.0));

  OnOffHelper onoff6 ("ns3::UdpSocketFactory", InetSocketAddress(erInterfaces.GetAddress (0), 8080));
  onoff6.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff6.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff6.SetAttribute ("DataRate", StringValue (StringValue ("10800bps"))); //75 packets per sec
  onoff6.SetAttribute ("PacketSize", UintegerValue (18));
  ApplicationContainer onoffApp5;
  onoffApp5.Add(onoff6.Install(Node.Get(5)));
  onoffApp5.Start(Seconds(0.0));
  onoffApp5.Stop(Seconds(40.0));

  OnOffHelper onoff7 ("ns3::UdpSocketFactory", InetSocketAddress(erInterfaces.GetAddress (0), 8080));
  onoff7.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff7.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff7.SetAttribute ("DataRate", StringValue (StringValue ("7200bps"))); //50 packets per sec
  onoff7.SetAttribute ("PacketSize", UintegerValue (18));
  ApplicationContainer onoffApp6;
  onoffApp6.Add(onoff7.Install(Node.Get(6)));
  onoffApp6.Start(Seconds(0.0));
  onoffApp6.Stop(Seconds(40.0));

  OnOffHelper onoff8 ("ns3::UdpSocketFactory", InetSocketAddress(erInterfaces.GetAddress (0), 8080));
  onoff8.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff8.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff8.SetAttribute ("DataRate", StringValue (StringValue ("10800bps"))); //75 packets per sec
  onoff8.SetAttribute ("PacketSize", UintegerValue (18));
  ApplicationContainer onoffApp7;
  onoffApp7.Add(onoff8.Install(Node.Get(7)));
  onoffApp7.Start(Seconds(0.0));
  onoffApp7.Stop(Seconds(40.0));

//Central cloud server receive packet from local router==========================================
  PacketSinkHelper sink1 ("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 8080));
  ApplicationContainer sinkApp1 = sink1.Install(Node.Get(8));
  Ptr<PacketSink> pktSink=StaticCast<PacketSink> (sinkApp1.Get(0));
  std::stringstream ss;
  ss<<"Some information";
  pktSink->TraceConnect("Rx", ss.str(), MakeCallback(&SinkRxTrace2));

  sinkApp1.Start (Seconds (0.0));
  sinkApp1.Stop (Seconds (40.0));


//Central cloud server send packet to each local router==============================================
     int sinkPort=9;
     Address sinkAddress_udp1(InetSocketAddress ("10.1.1.22", sinkPort)); //sg
     Ptr<Socket> ns3UdpSocket1=Socket::CreateSocket(Node.Get(8), UdpSocketFactory::GetTypeId()); 
     Ptr<MyApp> app_udp1=CreateObject<MyApp>();
     app_udp1->Setup(ns3UdpSocket1, sinkAddress_udp1, 4500, 1000, DataRate("10Mbps"));
     Node.Get(8)->AddApplication(app_udp1);
     app_udp1->SetStartTime(Seconds(1.0));
     app_udp1->SetStopTime(Seconds(70.0));

     Address sinkAddress_udp2(InetSocketAddress ("10.1.1.2", sinkPort)); //gw
     Ptr<Socket> ns3UdpSocket2=Socket::CreateSocket(Node.Get(8), UdpSocketFactory::GetTypeId()); 
     Ptr<MyApp> app_udp2=CreateObject<MyApp>();
     app_udp2->Setup(ns3UdpSocket2, sinkAddress_udp2, 900, 1000, DataRate("10Mbps"));
     Node.Get(8)->AddApplication(app_udp2);
     app_udp2->SetStartTime(Seconds(1.0));
     app_udp2->SetStopTime(Seconds(70.0));

     Address sinkAddress_udp3(InetSocketAddress ("10.1.1.10", sinkPort)); //cn
     Ptr<Socket> ns3UdpSocket3=Socket::CreateSocket(Node.Get(8), UdpSocketFactory::GetTypeId()); 
     Ptr<MyApp> app_udp3=CreateObject<MyApp>();
     app_udp3->Setup(ns3UdpSocket3, sinkAddress_udp3, 1350, 1000, DataRate("10Mbps"));
     Node.Get(8)->AddApplication(app_udp3);
     app_udp3->SetStartTime(Seconds(1.0));
     app_udp3->SetStopTime(Seconds(70.0));

     Address sinkAddress_udp4(InetSocketAddress ("10.1.1.16", sinkPort));//cb
     Ptr<Socket> ns3UdpSocket4=Socket::CreateSocket(Node.Get(8), UdpSocketFactory::GetTypeId()); 
     Ptr<MyApp> app_udp4=CreateObject<MyApp>();
     app_udp4->Setup(ns3UdpSocket4, sinkAddress_udp4, 900, 1000, DataRate("10Mbps"));
     Node.Get(8)->AddApplication(app_udp4);
     app_udp4->SetStartTime(Seconds(1.0));
     app_udp4->SetStopTime(Seconds(70.0));

     Address sinkAddress_udp5(InetSocketAddress ("10.1.1.12", sinkPort)); //jb
     Ptr<Socket> ns3UdpSocket5=Socket::CreateSocket(Node.Get(8), UdpSocketFactory::GetTypeId()); 
     Ptr<MyApp> app_udp5=CreateObject<MyApp>();
     app_udp5->Setup(ns3UdpSocket5, sinkAddress_udp5, 900, 1000, DataRate("10Mbps"));
     Node.Get(8)->AddApplication(app_udp5);
     app_udp5->SetStartTime(Seconds(1.0));
     app_udp5->SetStopTime(Seconds(70.0));

     Address sinkAddress_udp6(InetSocketAddress ("10.1.1.18", sinkPort)); //gb
     Ptr<Socket> ns3UdpSocket6=Socket::CreateSocket(Node.Get(8), UdpSocketFactory::GetTypeId()); 
     Ptr<MyApp> app_udp6=CreateObject<MyApp>();
     app_udp6->Setup(ns3UdpSocket6, sinkAddress_udp6, 1350, 1000, DataRate("10Mbps"));
     Node.Get(8)->AddApplication(app_udp6);
     app_udp6->SetStartTime(Seconds(1.0));
     app_udp6->SetStopTime(Seconds(70.0));

     Address sinkAddress_udp7(InetSocketAddress ("10.1.1.14", sinkPort)); //jn
     Ptr<Socket> ns3UdpSocket7=Socket::CreateSocket(Node.Get(8), UdpSocketFactory::GetTypeId()); 
     Ptr<MyApp> app_udp7=CreateObject<MyApp>();
     app_udp7->Setup(ns3UdpSocket7, sinkAddress_udp7, 900, 1000, DataRate("10Mbps"));
     Node.Get(8)->AddApplication(app_udp7);
     app_udp7->SetStartTime(Seconds(1.0));
     app_udp7->SetStopTime(Seconds(70.0));

     Address sinkAddress_udp8(InetSocketAddress ("10.1.1.20", sinkPort)); //gn
     Ptr<Socket> ns3UdpSocket8=Socket::CreateSocket(Node.Get(8), UdpSocketFactory::GetTypeId()); 
     Ptr<MyApp> app_udp8=CreateObject<MyApp>();
     app_udp8->Setup(ns3UdpSocket8, sinkAddress_udp8, 1350, 1000, DataRate("10Mbps"));
     Node.Get(8)->AddApplication(app_udp8);
     app_udp8->SetStartTime(Seconds(1.0));
     app_udp8->SetStopTime(Seconds(70.0));

//local router receive packet from edge========================================================
  PacketSinkHelper rsink1 ("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 9));
  ApplicationContainer rsinkApp1 = rsink1.Install(Node.Get(0));
  Ptr<PacketSink> pktrSink1=StaticCast<PacketSink> (rsinkApp1.Get(0));
  std::stringstream rss1;
  ss<<"Some information";
  pktrSink1->TraceConnect("Rx", rss1.str(), MakeCallback(&SinkRxTrace));
  rsinkApp1.Start (Seconds (0.0));
  rsinkApp1.Stop (Seconds (40.0));

  PacketSinkHelper rsink2 ("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 9));
  ApplicationContainer rsinkApp2 = rsink2.Install(Node.Get(1));
  Ptr<PacketSink> pktrSink2=StaticCast<PacketSink> (rsinkApp2.Get(0));
  std::stringstream rss2;
  ss<<"Some information";
  pktrSink2->TraceConnect("Rx", rss2.str(), MakeCallback(&SinkRxTrace));
  rsinkApp2.Start (Seconds (0.0));
  rsinkApp2.Stop (Seconds (40.0));

  PacketSinkHelper rsink3 ("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 9));
  ApplicationContainer rsinkApp3 = rsink3.Install(Node.Get(2));
  Ptr<PacketSink> pktrSink3=StaticCast<PacketSink> (rsinkApp3.Get(0));
  std::stringstream rss3;
  ss<<"Some information";
  pktrSink3->TraceConnect("Rx", rss3.str(), MakeCallback(&SinkRxTrace));
  rsinkApp3.Start (Seconds (0.0));
  rsinkApp3.Stop (Seconds (40.0));

  PacketSinkHelper rsink4 ("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 9));
  ApplicationContainer rsinkApp4 = rsink4.Install(Node.Get(3));
  Ptr<PacketSink> pktrSink4=StaticCast<PacketSink> (rsinkApp4.Get(0));
  std::stringstream rss4;
  ss<<"Some information";
  pktrSink4->TraceConnect("Rx", rss4.str(), MakeCallback(&SinkRxTrace));
  rsinkApp4.Start (Seconds (0.0));
  rsinkApp4.Stop (Seconds (40.0));

  PacketSinkHelper rsink5 ("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 9));
  ApplicationContainer rsinkApp5 = rsink5.Install(Node.Get(4));
  Ptr<PacketSink> pktrSink5=StaticCast<PacketSink> (rsinkApp5.Get(0));
  std::stringstream rss5;
  ss<<"Some information";
  pktrSink5->TraceConnect("Rx", rss5.str(), MakeCallback(&SinkRxTrace));
  rsinkApp5.Start (Seconds (0.0));
  rsinkApp5.Stop (Seconds (40.0));

  PacketSinkHelper rsink6 ("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 9));
  ApplicationContainer rsinkApp6 = rsink6.Install(Node.Get(5));
  Ptr<PacketSink> pktrSink6=StaticCast<PacketSink> (rsinkApp6.Get(0));
  std::stringstream rss6;
  ss<<"Some information";
  pktrSink6->TraceConnect("Rx", rss6.str(), MakeCallback(&SinkRxTrace));
  rsinkApp6.Start (Seconds (0.0));
  rsinkApp6.Stop (Seconds (40.0));

  PacketSinkHelper rsink7 ("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 9));
  ApplicationContainer rsinkApp7 = rsink7.Install(Node.Get(6));
  Ptr<PacketSink> pktrSink7=StaticCast<PacketSink> (rsinkApp7.Get(0));
  std::stringstream rss7;
  ss<<"Some information";
  pktrSink7->TraceConnect("Rx", rss7.str(), MakeCallback(&SinkRxTrace));
  rsinkApp7.Start (Seconds (0.0));
  rsinkApp7.Stop (Seconds (40.0));

  PacketSinkHelper rsink8 ("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 9));
  ApplicationContainer rsinkApp8 = rsink8.Install(Node.Get(7));
  Ptr<PacketSink> pktrSink8=StaticCast<PacketSink> (rsinkApp8.Get(0));
  std::stringstream rss8;
  ss<<"Some information";
  pktrSink8->TraceConnect("Rx", rss8.str(), MakeCallback(&SinkRxTrace));
  rsinkApp8.Start (Seconds (0.0));
  rsinkApp8.Stop (Seconds (40.0));

//=========================================================================================================

  AnimationInterface anim (animFile); 
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  Simulator::Stop (Seconds (30.0));
  Simulator::Run ();

   Simulator::Destroy ();

/* for debugging
  std::cout<<"count_router: "<<router_count<<"count_cloud: "<<cloud_count<<" \n";
  std::cout<<"router_total delay: "<<router_total_delay<<"cloud total delay: "<<cloud_total_delay<<" \n";
*/ 

  cloud_average_delay=(cloud_total_delay-303629.999747)/cloud_count; //when simulation time= 30sec
  router_average_delay=router_total_delay/router_count;

 // std::cout<<"\n\nTotal Propagation Delay (local router --> Central cloud server): "<<cloud_total_delay<<"sec\n";
  std::cout<<"Average Propagation Delay (local router --> Central cloud server): "<<cloud_average_delay<<"sec\n\n";


// std::cout<<"Total Propagation Delay (Central cloud server --> local router): "<<router_total_delay<<"sec\n";
  std::cout<<"Average Propagation Delay (Central cloud server --> local router): "<<router_average_delay<<"sec\n";
   return 0;
 }


/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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



/*
          Network Topology
    
  n0 ----               ---- n4
         -             -
          n2---------n3
         -             -
  n1 ----               ---- n5
    


*/
#include <fstream>
#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpAnalysis");


class App : public Application
{
  private:
    virtual void StartApplication (void);
    virtual void StopApplication (void);
    void ScheduleTx ();
    void SendPacket ();

    Ptr<Socket>     m_socket;
    Address         m_peer;
    uint32_t        m_packetSize;
    uint32_t        m_n_Packets;
    DataRate        m_dataRate;
    EventId         m_sendEvent;
    bool            m_running;
    uint32_t        m_packetsSent;
  public:
    App ();
    virtual ~App ();
    void Init (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);
    void ChangeRate (DataRate rate);
    void Receive (int nbytes);
};

App::App () : m_socket (0), m_peer (), m_packetSize (0), m_n_Packets (0), m_dataRate (0),
    m_sendEvent (), m_running (false), m_packetsSent (0){
}

App::~App ()
{
  m_socket = 0;
}

void App::Init (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_n_Packets = nPackets;
  m_dataRate = dataRate;
}

void App::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void App::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    Simulator::Cancel (m_sendEvent);

  if (m_socket)
    m_socket->Close ();
}

void App::SendPacket ()
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_n_Packets)
    ScheduleTx ();  
}

void App::ScheduleTx ()
{
  if (m_running)
  {
    Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
    m_sendEvent = Simulator::Schedule (tNext, &App::SendPacket, this);
  }
}

void App::ChangeRate (DataRate rate)
{
  m_dataRate = rate;
}


ApplicationContainer exp1 (Address sinkAddress, uint16_t sinkPort, Ipv4Address address, Ptr<Node> sender, Ptr<Node> destination)
{
  uint32_t maxBytes = 20*1024;
  // uint packetSize = 1.2*1024;   // 1.2KB
  // uint numPackets = maxBytes / packetSize;
  // set tcp cubic as protocol to use
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpBic::GetTypeId()));

  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (destination);
  sinkApps.Start (Seconds (0.));

  BulkSendHelper bulkSender ("ns3::TcpSocketFactory", InetSocketAddress (address, sinkPort));
  bulkSender.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  ApplicationContainer app = bulkSender.Install (sender);
  // Ptr<App> app = CreateObject<App> ();
  // app->Init (bulkSender, sinkAddress, packetSize, numPackets, DataRate("1Gbps"));
  // sender->AddApplication (app);
  app.Start (Seconds (1.));

  return sinkApps;
}

int main (int argc, char *argv[])
{
  CommandLine cmd;

  cmd.Parse (argc, argv);

  Time::SetResolution (Time::NS); 

  NS_LOG_INFO ("Creating Nodes");
  NodeContainer nodes;
  nodes.Create (6);

  NodeContainer n0n2 = NodeContainer (nodes.Get (0), nodes.Get (2));
  NodeContainer n1n2 = NodeContainer (nodes.Get (1), nodes.Get (2));
  NodeContainer n2n3 = NodeContainer (nodes.Get (2), nodes.Get (3));
  NodeContainer n3n4 = NodeContainer (nodes.Get (3), nodes.Get (4));
  NodeContainer n3n5 = NodeContainer (nodes.Get (3), nodes.Get (5));

  // install protocols
  InternetStackHelper internet;
  internet.Install (nodes);

  // create channels
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  // creating channels
  NetDeviceContainer d0d2 = pointToPoint.Install (n0n2);
  NetDeviceContainer d1d2 = pointToPoint.Install (n1n2);
  NetDeviceContainer d2d3 = pointToPoint.Install (n2n3);
  NetDeviceContainer d3d4 = pointToPoint.Install (n3n4);
  NetDeviceContainer d3d5 = pointToPoint.Install (n3n5);

  // adding IP addresses
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i2 = address.Assign (d0d2);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i2 = address.Assign (d1d2);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i2i3 = address.Assign (d2d3);

  address.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer i3i4 = address.Assign (d3d4);

  address.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer i3i5 = address.Assign (d3d5);


  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


  // experiment 1 starts here
  uint16_t sinkPort_1 = 8080;
  Address sinkAddress_1 (InetSocketAddress (i3i5.GetAddress (1), sinkPort_1));
  // ApplicationContainer sinkApps_1 = exp1(sinkAddress_1, sinkPort_1, i3i5.GetAddress (1),nodes.Get (0), nodes.Get (4));
  uint32_t maxBytes = 20*1024;
  // uint packetSize = 1.2*1024;   // 1.2KB
  // uint numPackets = maxBytes / packetSize;
  // set tcp cubic as protocol to use
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpBic::GetTypeId()));

  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort_1));
  ApplicationContainer sinkApps_1 = packetSinkHelper.Install (nodes.Get (0));
  sinkApps_1.Start (Seconds (0.));
  sinkApps_1.Stop (Seconds (10.));
  BulkSendHelper bulkSender ("ns3::TcpSocketFactory", InetSocketAddress (i3i5.GetAddress (1), sinkPort_1));
  bulkSender.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  ApplicationContainer app = bulkSender.Install (nodes.Get (0));
  // Ptr<App> app = CreateObject<App> ();
  // app->Init (bulkSender, sinkAddress, packetSize, numPackets, DataRate("1Gbps"));
  // sender->AddApplication (app);
  app.Start (Seconds (1.));

  app.Stop (Seconds (10.));
  
/*



  // tcp from no to n4
  uint16_t sinkPort_1 = 8080;
  Address sinkAddress_1 (InetSocketAddress (i3i4.GetAddress (1), sinkPort_1));
  PacketSin kHelper packetSinkHelper_1 ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), sinkPort_1));
  ApplicationContainer sinkApps_1 = packetSinkHelper_1.Install (nodes.Get (4));
  sinkApps_1.Start (Seconds (0.));
  sinkApps_1.Stop (Seconds (100.));

  Ptr<Socket> ns3TcpSocket_1 = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());

  // create tcp application at n0
  Ptr<App> app_1 = CreateObject<App> ();
  app_1->Init (ns3TcpSocket_1, sinkAddress_1, 1040, 100000, DataRate ("1Gbps"));
  nodes.Get(0)->AddApplication (app_1);
  // app_1->SetStartTime (Seconds (1.));
  // app_1->SetStopTime (Seconds (100.));


  // tcp from n1 to n5
  uint16_t sinkPort_2 = 8080;
  Address sinkAddress_2 (InetSocketAddress (i3i5.GetAddress (1), sinkPort_2));
  PacketSinkHelper packetSinkHelper_2 ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), sinkPort_2));
  ApplicationContainer sinkApps_2 = packetSinkHelper_2.Install (nodes.Get (5));
  sinkApps_2.Start (Seconds (0.));
  sinkApps_2.Stop (Seconds (100.));

  Ptr<Socket> ns3TcpSocket_2 = Socket::CreateSocket (nodes.Get (1), TcpSocketFactory::GetTypeId ());

  // create tcp application at n0
  Ptr<App> app_2 = CreateObject<App> ();
  app_2->Init (ns3TcpSocket_2, sinkAddress_2, 1040, 100000, DataRate ("1Gbps"));
  nodes.Get(1)->AddApplication (app_2);
  // app_2->SetStartTime (Seconds (1.));
  // app_2->SetStopTime (Seconds (100.));
*/

  // Both the destination nodes have started thier service at this point


  AnimationInterface anim ("project.xml");
  anim.SetConstantPosition (nodes.Get(0), 10.0, 10.0);
  anim.SetConstantPosition (nodes.Get(1), 10.0, 70.0);
  anim.SetConstantPosition (nodes.Get(2), 40.0, 40.0);
  anim.SetConstantPosition (nodes.Get(3), 60.0, 40.0);
  anim.SetConstantPosition (nodes.Get(4), 80.0, 10.0);
  anim.SetConstantPosition (nodes.Get(4), 80.0, 70.0);

  // Simulator::Stop (Seconds(100.0));
  Simulator::Run ();
  Simulator::Destroy ();
  Ptr<PacketSink> check_sink = DynamicCast<PacketSink> (sinkApps_1.Get (0));
  std::cout << "Total Bytes Rcvd: " << check_sink->GetTotalRx () << std::endl;
  return 0;
}

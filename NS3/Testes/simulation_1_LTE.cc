/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 *
 * Author: Jaume Nin <jaume.nin@cttc.cat>
 */

#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/gnuplot.h"

#include "ns3/propagation-module.h"

#include <ns3/buildings-propagation-loss-model.h>

#include "ns3/antenna-model.h"

#include "ns3/isotropic-antenna-model.h"

#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/wifi-module.h"
#include <time.h>
#include "ns3/lte-enb-rrc.h"


#include "ns3/basic-energy-source.h"
#include "ns3/simple-device-energy-model.h"

//#include "ns3/gtk-config-store.h"

using namespace ns3;

/**
 * Sample simulation script for LTE+EPC. It instantiates several eNodeB,
 * attaches one UE per eNodeB starts a flow for each UE to  and from a remote host.
 * It also  starts yet another flow between each UE pair.
 */

NS_LOG_COMPONENT_DEFINE ("Lte_Simulation_1");

void ThroughputMonitor(FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon, Gnuplot2dDataset DataSet);
void DelayMonitor(FlowMonitorHelper *fmHelper, Ptr<FlowMonitor> flowMon, Gnuplot2dDataset Dataset2);
void LossMonitor(FlowMonitorHelper *fmHelper, Ptr<FlowMonitor> flowMon, Gnuplot2dDataset Dataset3);
void JitterMonitor(FlowMonitorHelper *fmHelper, Ptr<FlowMonitor> flowMon, Gnuplot2dDataset Dataset4);
    
uint16_t cenario = 4;
std::string gp = std::to_string(cenario);

int
main (int argc, char *argv[])
{

  uint16_t numberOfNodesENB = 1;
  uint16_t numberOfNodesUE = 100;


  // uint16_t numberOfNodes = numberOfNodesENB + numberOfNodesUE;


  double PacketInterval = 0.15;
  double MaxPacketSize = 1024;
  
  double simTime = 100;
// double interPacketInterval = 150.0;
// double simTime = 0.05;
  // double distance = 500.0;

  
//creation de l'objet epcHelper.
Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
lteHelper->SetAttribute ("PathlossModel", 
                         StringValue ("ns3::FriisPropagationLossModel"));
lteHelper->SetEnbAntennaModelType ("ns3::IsotropicAntennaModel");
Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (320));
lteHelper->SetEpcHelper (epcHelper);

Ptr<Node> pgw = epcHelper->GetPgwNode (); 

// creation  RemoteHost .
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);
  
// Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);

  // interface 0 is localhost, 1 is the p2p device
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
//creation des noeuds pour eNB et UE
NodeContainer enbNodes;
enbNodes.Create (numberOfNodesENB);

NodeContainer ueNodes;



  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                     "MinX", DoubleValue (800.0),
                                     "MinY", DoubleValue (2828.0),
                                     "DeltaX", DoubleValue (10.0),
                                     "DeltaY", DoubleValue (10.0),
                                     "GridWidth", UintegerValue (1),
                                     "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install(remoteHost);


  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector(750, 2828, 32));
  MobilityHelper mobility1;
  mobility1.SetPositionAllocator(positionAlloc);
  mobility1.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility1.Install(enbNodes);
  mobility1.Install(pgw);

  MobilityHelper mobilityUe;
  if(cenario == 1){
    ueNodes.Create (numberOfNodesUE/2);
    mobilityUe.SetPositionAllocator ("ns3::GridPositionAllocator",
                                   "MinX", DoubleValue (0.0),
                                   "MinY", DoubleValue (1414),
                                   "DeltaX", DoubleValue (5.0),
                                   "DeltaY", DoubleValue (5.0),
                                   "GridWidth", UintegerValue (10),
                                   "LayoutType", StringValue ("RowFirst"));
    }else{
          if(cenario == 2){
            ueNodes.Create (numberOfNodesUE/2);
            mobilityUe.SetPositionAllocator ("ns3::GridPositionAllocator",
                                             "MinX", DoubleValue (1500.0),
                                             "MinY", DoubleValue (0),
                                             "DeltaX", DoubleValue (5.0),
                                             "DeltaY", DoubleValue (5.0),
                                             "GridWidth", UintegerValue (10),
                                             "LayoutType", StringValue ("RowFirst"));
            }else{
                  if(cenario == 3){
                    ueNodes.Create (numberOfNodesUE);
                    mobilityUe.SetPositionAllocator ("ns3::GridPositionAllocator",
                                                   "MinX", DoubleValue (0),
                                                   "MinY", DoubleValue (4242),
                                                   "DeltaX", DoubleValue (5.0),
                                                   "DeltaY", DoubleValue (5.0),
                                                   "GridWidth", UintegerValue (10),
                                                   "LayoutType", StringValue ("RowFirst"));

                    }else{
                          if(cenario == 4){
                            ueNodes.Create (numberOfNodesUE);
                            mobilityUe.SetPositionAllocator ("ns3::GridPositionAllocator",
                                                         "MinX", DoubleValue (1500),
                                                         "MinY", DoubleValue (5656),
                                                         "DeltaX", DoubleValue (5.0),
                                                         "DeltaY", DoubleValue (5.0),
                                                         "GridWidth", UintegerValue (10),
                                                         "LayoutType", StringValue ("RowFirst"));
                          }
                        }
                  }
          }
   
  mobilityUe.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                           "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=10.0]"),
                           "Bounds", StringValue ("-6000|6000|-6000|6000"));
  mobilityUe.Install(ueNodes);

  //pour installer le protocol lte pour enbNodes et ueNodes.
NetDeviceContainer enbDevs;
enbDevs = lteHelper->InstallEnbDevice (enbNodes);
NetDeviceContainer ueDevs;
ueDevs = lteHelper->InstallUeDevice (ueNodes);
//BuildingsHelper::MakeMobilityModelConsistent ();
// Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));
  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

//pour attacher ueDevs avec enbDevs
//lteHelper->Attach (ueDevs, enbDevs.Get (0));
uint16_t j = 0;

for (uint16_t i = 0; i < ueNodes.GetN(); i++)
  {  
     if (j < numberOfNodesENB)
        {
          lteHelper->Attach (ueDevs.Get(i), enbDevs.Get(j));
          j++;
        }
      else
        {
          j = 0;
          lteHelper->Attach (ueDevs.Get(i), enbDevs.Get(j));
        }   
  }
// Attach all UEs to the closest eNodeB
//lteHelper->AttachToClosestEnb (ueDevs, enbDevs);
// Add X2 inteface
  //lteHelper->AddX2Interface (enbNodes);
// X2-based Handover
 // lteHelper->HandoverRequest (Seconds (0.100), ueDevs.Get (0), enbDevs.Get (0), enbDevs.Get (1));


//pour activer le support radio qui porte les données entre ueDevs et enbDevs
// Ptr<EpcTft> tft = Create<EpcTft> ();
// EpcTft::PacketFilter pf;
// pf.localPortStart = 1234;
// pf.localPortEnd = 1234;
// tft->Add (pf);
// lteHelper->ActivateDedicatedEpsBearer (ueDevs, EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), tft);


  // Install and start applications on UEs and remote host

  uint16_t dlPort = 1234;
  uint16_t ulPort = 2000;
  uint16_t otherPort = 3000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      ++ulPort;
      ++otherPort;
      PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
      PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
      PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), otherPort));
      serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get(u)));
      serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
      serverApps.Add (packetSinkHelper.Install (ueNodes.Get(u)));

      UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
      dlClient.SetAttribute ("MaxPackets", UintegerValue ((uint32_t)(simTime*(1/PacketInterval))));
      dlClient.SetAttribute ("Interval", TimeValue (Seconds (PacketInterval)));
      dlClient.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));

      UdpClientHelper ulClient (remoteHostAddr, ulPort);
      ulClient.SetAttribute ("MaxPackets", UintegerValue ((uint32_t)(simTime*(1/PacketInterval))));
      ulClient.SetAttribute ("Interval", TimeValue (Seconds (PacketInterval)));
      ulClient.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));

      UdpClientHelper client (ueIpIface.GetAddress (u), otherPort);
      client.SetAttribute ("MaxPackets", UintegerValue ((uint32_t)(simTime*(1/PacketInterval))));
      client.SetAttribute ("Interval", TimeValue (Seconds (PacketInterval)));
      client.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));

      clientApps.Add (dlClient.Install (remoteHost));
      clientApps.Add (ulClient.Install (ueNodes.Get(u)));
      if (u+1 < ueNodes.GetN ())
        {
          clientApps.Add (client.Install (ueNodes.Get(u+1)));
        }
      else
        {
          clientApps.Add (client.Install (ueNodes.Get(0)));
        }
    }
  serverApps.Start (Seconds (0.1));
  serverApps.Stop (Seconds (simTime));

  clientApps.Start (Seconds (0.1));
  clientApps.Stop (Seconds (simTime));
 
//FLOW-MONITOR
    

    //-----------------FlowMonitor-THROUGHPUT----------------
    std::string fileNameWithNoExtension = "lte_Flow_vs_Throughput_Group_" + std::to_string(cenario);
    std::string graphicsFileName        = fileNameWithNoExtension + ".png";
    std::string plotFileName            = fileNameWithNoExtension + ".plt";
    std::string plotTitle               = "Flow_vs_Throughput";
    std::string dataTitle               = "Throughput";

    Gnuplot gnuplot (graphicsFileName);
    gnuplot.SetTitle (plotTitle);
    gnuplot.SetTerminal ("png");
    gnuplot.SetLegend ("Flow", "Throughput");
     
    Gnuplot2dDataset dataset;
    dataset.SetTitle (dataTitle);
    dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);

    //flowMonitor declaration
    FlowMonitorHelper fmHelper;
    Ptr<FlowMonitor> allMon = fmHelper.InstallAll();

    // call the flow monitor function
    ThroughputMonitor(&fmHelper, allMon, dataset); 
                
     
    //-----------------FlowMonitor-Atraso--------------------

    std::string fileNameWithNoExtension2 = "lte_Flow_vs_Delay_Group_" + std::to_string(cenario);
    std::string graphicsFileName2      = fileNameWithNoExtension2 + ".png";
    std::string plotFileName2        = fileNameWithNoExtension2 + ".plt";
    std::string plotTitle2           = "Flow_vs_Delay";
    std::string dataTitle2           = "Delay";

    Gnuplot gnuplot2 (graphicsFileName2);
    gnuplot2.SetTitle(plotTitle2);
    gnuplot2.SetTerminal("png");
    gnuplot2.SetLegend("Flow", "Delay");

    Gnuplot2dDataset dataset2;
    dataset2.SetTitle(dataTitle2);
    dataset2.SetStyle(Gnuplot2dDataset::LINES_POINTS);

    //FlowMonitorHelper fmHelper;
    //Ptr<FlowMonitor> allMon = fmHelper.InstallAll();

    DelayMonitor(&fmHelper, allMon, dataset2);

    //-----------------FlowMonitor-LossPackets--------------------

    std::string fileNameWithNoExtension3 = "lte_Flow_vs_Loss_Group_" + std::to_string(cenario);
    std::string graphicsFileName3      = fileNameWithNoExtension3 + ".png";
    std::string plotFileName3        = fileNameWithNoExtension3 + ".plt";
    std::string plotTitle3           = "Flow_vs_Loss";
    std::string dataTitle3           = "Loss";

    Gnuplot gnuplot3 (graphicsFileName3);
    gnuplot3.SetTitle(plotTitle3);
    gnuplot3.SetTerminal("png");
    gnuplot3.SetLegend("Flow", "Loss");

    Gnuplot2dDataset dataset3;
    dataset3.SetTitle(dataTitle3);
    dataset3.SetStyle(Gnuplot2dDataset::LINES_POINTS);

    //FlowMonitorHelper fmHelper;
    //Ptr<FlowMonitor> allMon = fmHelper.InstallAll();

    LossMonitor(&fmHelper, allMon, dataset3);
   
    //-----------------FlowMonitor-JITTER--------------------

    std::string fileNameWithNoExtension4 = "lte_Flow_vs_Jitter_Group_" + std::to_string(cenario);
    std::string graphicsFileName4      = fileNameWithNoExtension4 + ".png";
    std::string plotFileName4        = fileNameWithNoExtension4 + ".plt";
    std::string plotTitle4           = "Flow_vs_Jitter";
    std::string dataTitle4           = "Jitter";

    Gnuplot gnuplot4 (graphicsFileName4);
    gnuplot4.SetTitle(plotTitle4);
    gnuplot4.SetTerminal("png");
    gnuplot4.SetLegend("Flow", "Jitter");

    Gnuplot2dDataset dataset4;
    dataset4.SetTitle(dataTitle4);
    dataset4.SetStyle(Gnuplot2dDataset::LINES_POINTS);

    //FlowMonitorHelper fmHelper;
    //Ptr<FlowMonitor> allMon = fmHelper.InstallAll();

    JitterMonitor(&fmHelper, allMon, dataset4);



//Install NetAnim
   AnimationInterface anim (gp + "_group_simulation_1_lte.xml"); // Mandatory
        
          anim.UpdateNodeDescription (ueNodes.Get(0), "ueNodes"); // Optional
          anim.UpdateNodeColor (ueNodes.Get(0), 255, 0, 0); // Coloração


          anim.UpdateNodeDescription (enbNodes.Get(0), "enbNodes"); // Optional
          anim.UpdateNodeColor (enbNodes.Get(0), 255, 255, 0); // Coloração
       
        anim.EnablePacketMetadata ();



  Simulator::Stop(Seconds(simTime));
  Simulator::Run();

  //Gnuplot ...continued
      gnuplot.AddDataset (dataset);
      std::ofstream plotFile (plotFileName.c_str());
      gnuplot.GenerateOutput (plotFile);
      plotFile.close ();

      gnuplot2.AddDataset(dataset2);;
      std::ofstream plotFile2 (plotFileName2.c_str());
      gnuplot2.GenerateOutput(plotFile2);
      plotFile2.close();

      gnuplot3.AddDataset(dataset3);;
      std::ofstream plotFile3 (plotFileName3.c_str());
      gnuplot3.GenerateOutput(plotFile3);
      plotFile3.close();

      gnuplot4.AddDataset(dataset4);;
      std::ofstream plotFile4 (plotFileName4.c_str());
      gnuplot4.GenerateOutput(plotFile4);
      plotFile4.close();

  Simulator::Destroy();
  return 0;
}


//-------------------------Metodo-VAZÃO---------------------------

  void ThroughputMonitor (FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon, Gnuplot2dDataset DataSet)
  {
    double localvazao = 0;
    std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats();
    Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());
      
      for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats)
        {
        Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);
        // if(fiveTuple.destinationAddress == "192.168.1.6")
        if(stats->first < 2)
         // if(fiveTuple.sourceAddress == "7.0.0.2" && fiveTuple.destinationAddress == "10.0.0.5")
        {
             std::cout<<"--------------------------------Vazao---------------------------------"<<std::endl;
              std::cout<<"Flow ID: " << stats->first <<"; "<< fiveTuple.sourceAddress <<" -----> "<<fiveTuple.destinationAddress<<std::endl;
              std::cout<<"Duration  : "<<(stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())<<std::endl;
              std::cout<<"Vazao: " <<  stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())/1024/1024<<" Mbps"<<std::endl;
              localvazao=  stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())/1024/1024;
              DataSet.Add((double)Simulator::Now().GetSeconds(),(double) localvazao);
              std::cout<<" "<<std::endl;
          }
        }
    
      Simulator::Schedule(Seconds(1), &ThroughputMonitor, fmhelper, flowMon, DataSet);
   //if(flowToXml)
      {
    flowMon->SerializeToXmlFile (gp + "_group_lte_Flow.xml", true, true);
      }
  }

//-------------------------Metodo-Atraso-------------------------
    void DelayMonitor(FlowMonitorHelper *fmHelper, Ptr<FlowMonitor> flowMon, Gnuplot2dDataset Dataset2)
  {
    double localDelay=0;
    
           std::map<FlowId, FlowMonitor::FlowStats> flowstats = flowMon->GetFlowStats();
           Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmHelper->GetClassifier());
           
      
             for(std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowstats.begin(); stats != flowstats.end(); ++stats)
             {
                Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);
                // if(fiveTuple.destinationAddress == "192.168.1.6")
                if(stats->first < 2)
                 // if(fiveTuple.sourceAddress == "7.0.0.2" && fiveTuple.destinationAddress == "10.0.0.5")
                {
                    std::cout<<"--------------------------------Atraso-------------------------------------"<<std::endl;
                    std::cout<<"Flow ID: "<< stats->first <<"; "<< fiveTuple.sourceAddress <<" ------> " <<fiveTuple.destinationAddress<<std::endl;
                    std::cout<<"Duration  : "<<(stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())<<std::endl;
                    std::cout<<"Atraso: "<< ((stats->second.timeLastRxPacket.GetSeconds()) - (stats->second.timeLastTxPacket.GetSeconds()))<<std::endl;
                    localDelay = ((stats->second.timeLastRxPacket.GetSeconds()) - (stats->second.timeLastTxPacket.GetSeconds()));
                    Dataset2.Add((double)Simulator::Now().GetSeconds(), (double) localDelay);
                    std::cout<<" "<<std::endl;
                }
             }
      
        
      Simulator::Schedule(Seconds(1), &DelayMonitor, fmHelper, flowMon, Dataset2);
      // {
      //   flowMon->SerializeToXmlFile("DelayMonitor.xml", true, true);
      // }
  }

  //-------------------------Metodo-Loss-------------------------
    void LossMonitor(FlowMonitorHelper *fmHelper, Ptr<FlowMonitor> flowMon, Gnuplot2dDataset Dataset3)
  {
    double localLoss=0;

           std::map<FlowId, FlowMonitor::FlowStats> flowstats = flowMon->GetFlowStats();
           Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmHelper->GetClassifier());
           
      
             for(std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowstats.begin(); stats != flowstats.end(); ++stats)
             {
                Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);
                // if(fiveTuple.destinationAddress == "192.168.1.6")
                if(stats->first < 2)
                 // if(fiveTuple.sourceAddress == "7.0.0.2" && fiveTuple.destinationAddress == "10.0.0.5")
                {
                    std::cout<<"--------------------------------Loss-------------------------------------"<<std::endl;
                    std::cout<<"    Flow ID: "<< stats->first <<"; "<< fiveTuple.sourceAddress <<" ------> " <<fiveTuple.destinationAddress<<std::endl;
                    std::cout<<"Tx Packets = " << stats->second.txPackets<<std::endl;
                    std::cout<<"Rx Packets = " << stats->second.rxPackets<<std::endl;
                    localLoss =stats->second.txPackets - stats->second.rxPackets;
                    std::cout<<"Perda de Pacotes: "<< localLoss<<std::endl;
                    Dataset3.Add((double)Simulator::Now().GetSeconds(), (double) localLoss);
                    std::cout<<" "<<std::endl;
                }

             }
              
      Simulator::Schedule(Seconds(1), &LossMonitor, fmHelper, flowMon, Dataset3);
      // {
      //   flowMon->SerializeToXmlFile("LossMonitor.xml", true, true);
      // }
  }

    void JitterMonitor(FlowMonitorHelper *fmHelper, Ptr<FlowMonitor> flowMon, Gnuplot2dDataset Dataset4)
    {
      double localJitter=0;
      double atraso1=0;
      double atraso2 =0;

           std::map<FlowId, FlowMonitor::FlowStats> flowstats = flowMon->GetFlowStats();
           Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmHelper->GetClassifier());
           for(std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowstats.begin(); stats != flowstats.end(); ++stats)
           {
            Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);
            // if(fiveTuple.destinationAddress == "192.168.1.6")
            if(stats->first < 2)
            {
                std::cout<<"--------------------------------Jitter-------------------------------------"<<std::endl;
                std::cout<<"Flow ID : "<< stats->first <<"; "<< fiveTuple.sourceAddress <<"------>" <<fiveTuple.destinationAddress<<std::endl;
                atraso2 = stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeLastTxPacket.GetSeconds();
                atraso1 = stats->second.timeFirstRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds();
                std::cout<<"Jitter: "<< atraso2-atraso1 <<std::endl;
                localJitter= atraso2-atraso1;//Jitter
                Dataset4.Add((double)Simulator::Now().GetSeconds(), (double) localJitter);
                std::cout<<" "<<std::endl;
                }
                atraso1 = atraso2;
           }

           Simulator::Schedule(Seconds(1),&JitterMonitor, fmHelper, flowMon, Dataset4);
           // {
           //   flowMon->SerializeToXmlFile("JitterMonitor.xml", true, true);
           // }
    }
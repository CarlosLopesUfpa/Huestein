/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Laboratory Of Development Systems
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
 * Author: Carlos Lopes <carloslopesufpa@gmail.com>
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


#include "ns3/propagation-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/wifi-module.h"
#include <time.h>
#include "ns3/gnuplot.h"

#include "ns3/basic-energy-source.h"
#include "ns3/simple-device-energy-model.h"

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/command-line.h"
#include "ns3/csma-module.h"

//#include "ns3/gtk-config-store.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Wifi_Test");

void ThroughputMonitor(FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon, Gnuplot2dDataset DataSet);
void DelayMonitor(FlowMonitorHelper *fmHelper, Ptr<FlowMonitor> flowMon, Gnuplot2dDataset Dataset2);
void LossMonitor(FlowMonitorHelper *fmHelper, Ptr<FlowMonitor> flowMon, Gnuplot2dDataset Dataset3);
void JitterMonitor(FlowMonitorHelper *fmHelper, Ptr<FlowMonitor> flowMon, Gnuplot2dDataset Dataset4);
    


int
main (int argc, char *argv[])
{
  

  //uint16_t numberOfNodes = 1;
  double simTime = 60;
  // double distance = 60.0;
  double interPacketInterval = 0.2;
  double Rx = 0;

  // int aux_energy = 0;
  int nAp = 1;
  int nSta = 5;
  
  CommandLine cmd;
  cmd.AddValue ("Rx", "Number of Packets", Rx);
  cmd.Parse (argc,argv);
  // double Energia;

  // Energia = 0;
  
  // 1. Create 3 nodes

      NodeContainer wifiApNodes;
      wifiApNodes.Create (nAp);
          
      NodeContainer wifiStaNodes;
      wifiStaNodes.Create (nSta);

      NodeContainer all;
      all.Add(wifiApNodes);
      all.Add(wifiStaNodes);

  // // 2. Place nodes somehow, this is required by every wireless simulation
  // for (size_t i = 0; i < wifiStaNodes.GetN(); ++i)
  //   {
  //     all.Get (i)->AggregateObject (CreateObject<ConstantPositionMobilityModel> ());
  //   }

  // // 3. Create propagation loss matrix
  // Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel> ();
  // lossModel->SetDefaultLoss (200); // set default loss to 200 dB (no link)
  // for(int u = 0; u < nSta; ++u){
  // lossModel->SetLoss (wifiApNodes.Get (0)->GetObject<MobilityModel> (), wifiStaNodes.Get (u)->GetObject<MobilityModel> (), 50); // set symmetric loss 0 <-> 1 to 50 dB
  // }



  // // 4. Create & setup wifi channel
  // Ptr<YansWifiChannel> wifiChannel = CreateObject <YansWifiChannel> ();
  // wifiChannel->SetPropagationLossModel (lossModel);
  // wifiChannel->SetPropagationDelayModel (CreateObject <ConstantSpeedPropagationDelayModel> ());
      YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
      YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
      wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
      wifiPhy.SetChannel (channel.Create ());

      WifiHelper wifi;
      wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("HtMcs7"), "ControlMode", StringValue ("HtMcs0"));
      WifiMacHelper wifiMac;

  NetDeviceContainer staDevice, apDevice;
  Ssid ssid;

  //Network A
  ssid = Ssid ("network-A");
  wifiPhy.Set ("ChannelNumber", UintegerValue (36));
  wifiMac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssid));
  apDevice = wifi.Install (wifiPhy, wifiMac, wifiStaNodes);


  wifiMac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid),
               "EnableBeaconJitter", BooleanValue (false));
  staDevice = wifi.Install (wifiPhy, wifiMac, wifiApNodes);


//   std::string phyMode ("DsssRate1Mbps");
//   bool verbose = false;
// // The below set of helpers will help us to put together the wifi NICs we want
//   WifiHelper wifi;
//   if (verbose)
//     {
//       wifi.EnableLogComponents ();  // Turn on all Wifi logging
//     }

// YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
//   // set it to zero; otherwise, gain will be added
//   wifiPhy.Set ("RxGain", DoubleValue (-10) );
//   // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
//   wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

//   YansWifiChannelHelper wifiChannel;
//   wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
//   wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
//   wifiPhy.SetChannel (wifiChannel.Create ());

//   // Add an upper mac and disable rate control
//   WifiMacHelper wifiMac;
//   wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
//   wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
//                                 "DataMode",StringValue (phyMode),
//                                 "ControlMode",StringValue (phyMode));
//   // Set it to adhoc mode
//   wifiMac.SetType ("ns3::AdhocWifiMac");
//   NetDeviceContainer apDevice = wifi.Install (wifiPhy, wifiMac, wifiApNodes);
//   NetDeviceContainer staDevice = wifi.Install (wifiPhy, wifiMac, wifiStaNodes);
      

      
  MobilityHelper mobilitywifiAp;
  mobilitywifiAp.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (30.0),
                                 "MinY", DoubleValue (5.0),
                                 "DeltaX", DoubleValue (10.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (1),
                                 "LayoutType", StringValue ("RowFirst"));

  mobilitywifiAp.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilitywifiAp.Install (wifiApNodes);


  MobilityHelper mobilitywifiSta;
  mobilitywifiSta.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (20.0),
                                 "MinY", DoubleValue (20.0),
                                 "DeltaX", DoubleValue (10.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (5),
                                 "LayoutType", StringValue ("RowFirst"));

  mobilitywifiSta.SetMobilityModel ("ns3::RandomWalk2dMobilityModel");
  mobilitywifiSta.Install (wifiStaNodes);

  // // Energy
  //     srand((unsigned)time(0));
  //     for (int l=0; l<nAp; ++l)
  //       {
  //         aux_energy = rand()%(100);
  //         Ptr<BasicEnergySource> energySource = CreateObject<BasicEnergySource>();
  //         Ptr<SimpleDeviceEnergyModel> energyModel = CreateObject<SimpleDeviceEnergyModel>();

  //         energySource->SetInitialEnergy (aux_energy);
  //         energyModel->SetEnergySource (energySource);
  //         energySource->AppendDeviceEnergyModel (energyModel);
  //         energyModel->SetCurrentA (20);

  //         // aggregate energy source to node
  //         wifiApNodes.Get(l)->AggregateObject (energySource);
  //         // energy[l][0] = energySource;
  //         Energia = aux_energy;
  //       }



  // // 6. Install TCP/IP stack & assign IP addresses
  InternetStackHelper internetw;
  internetw.Install (all);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer staInterface = ipv4.Assign (apDevice);
  Ipv4InterfaceContainer apInterface = ipv4.Assign (staDevice);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // ipv4.Assign (apDevice);
  // ipv4.Assign (staDevice);

  // uint16_t  port = 9;
  // // Time interPacketInterval = Seconds (0.1);

  // UdpEchoServerHelper apServer (port);
  // ApplicationContainer serverApp = apServer.Install (wifiApNodes.Get(0));
  // serverApp.Start (Seconds (1.0));
  // serverApp.Stop (Seconds(30.0));
      
  // UdpEchoClientHelper staClient (Ipv4Address ("192.168.1.1"), port);
  // staClient.SetAttribute ("MaxPackets", UintegerValue (Rx));
  // staClient.SetAttribute ("Interval", TimeValue (Seconds (interPacketInterval))); //packets/s
  // staClient.SetAttribute ("PacketSize", UintegerValue (1024));

  // // ApplicationContainer wifiApps;
  // for(uint16_t u = 0; u<wifiStaNodes.GetN(); ++u){
  //     serverApp = staClient.Install (wifiStaNodes.Get (u));
  // }
  // serverApp.Start (Seconds (2.0));
  // serverApp.Stop (Seconds(30.0));


//
// Create one udpServer applications on node one.

//
  uint16_t port = 4000;
  UdpServerHelper server (port);
  ApplicationContainer apps = server.Install (wifiApNodes.Get (0));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (60.0));

//
// Create one UdpClient application to send UDP datagrams from node zero to
// node one.
//
  uint32_t MaxPacketSize = 1024;
  uint32_t maxPacketCount = Rx;

  UdpClientHelper client (apInterface.GetAddress (0), port);
  client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client.SetAttribute ("Interval", TimeValue (Seconds (interPacketInterval)));
  client.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
  
  for(int u = 0; u<nSta; ++u){
  apps = client.Install (wifiStaNodes.Get(u));
  apps.Start (Seconds (2.0));
  apps.Stop (Seconds (60.0));
  }



//FLOW-MONITOR
    

    //-----------------FlowMonitor-THROUGHPUT----------------

    std::string fileNameWithNoExtension = "Flow_vs_Throughput";
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

    std::string fileNameWithNoExtension2 = "Flow_vs_Delay";
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

    LossMonitor(&fmHelper, allMon, dataset2);

    //-----------------FlowMonitor-LossPackets--------------------

    std::string fileNameWithNoExtension3 = "Flow_vs_Loss";
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

    DelayMonitor(&fmHelper, allMon, dataset3);
   
    //-----------------FlowMonitor-JITTER--------------------

    std::string fileNameWithNoExtension4 = "Flow_vs_Jitter";
    std::string graphicsFileName4      = fileNameWithNoExtension4 + ".png";
    std::string plotFileName4        = fileNameWithNoExtension4 + ".plt";
    std::string plotTitle4           = "Flow_vs_Jitter";
    std::string dataTitle4           = "Jitter";

    Gnuplot gnuplot4 (graphicsFileName2);
    gnuplot4.SetTitle(plotTitle4);
    gnuplot4.SetTerminal("png");
    gnuplot4.SetLegend("Flow", "Jitter");

    Gnuplot2dDataset dataset4;
    dataset4.SetTitle(dataTitle4);
    dataset4.SetStyle(Gnuplot2dDataset::LINES_POINTS);

    //FlowMonitorHelper fmHelper;
    //Ptr<FlowMonitor> allMon = fmHelper.InstallAll();

    JitterMonitor(&fmHelper, allMon, dataset4);

//NetAnim
   AnimationInterface anim ("wifi_test_3.xml"); // Mandatory
        
        for (uint32_t i = 0; i < wifiApNodes.GetN(); ++i)
        {
          anim.UpdateNodeDescription (wifiApNodes.Get(i), "Ap"); // Optional
          anim.UpdateNodeColor (wifiApNodes.Get(i), 0, 0, 0); // Coloração
        }
        for (uint32_t i = 0; i < wifiStaNodes.GetN(); ++i)
        {
          anim.UpdateNodeDescription (wifiStaNodes.Get(i), "Sta"); // Optional
          anim.UpdateNodeColor (wifiStaNodes.Get(i), 0, 0, 255); // Coloração
        }
        anim.EnablePacketMetadata (); // Optional



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
        if(fiveTuple.destinationAddress == "192.168.1.6")
        // if(stats->first < 5)
        {
             std::cout<<"--------------------------------Vazao---------------------------------"<<std::endl;
              std::cout<<"Flow ID: " << stats->first <<"; "<< fiveTuple.sourceAddress <<" -----> "<<fiveTuple.destinationAddress<<std::endl;
              std::cout<<"Vazao: " <<  stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())/1024/1024<<" Mbps"<<std::endl;
              localvazao=  stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())/1024/1024;
              DataSet.Add((double)Simulator::Now().GetSeconds(),(double) localvazao);
              std::cout<<" "<<std::endl;
          }
        }
    
      Simulator::Schedule(Seconds(1), &ThroughputMonitor, fmhelper, flowMon, DataSet);
   //if(flowToXml)
      {
    flowMon->SerializeToXmlFile ("Monitor.xml", true, true);
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
                if(fiveTuple.destinationAddress == "192.168.1.6")
                // if(stats->first < 5)
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
                if(fiveTuple.destinationAddress == "192.168.1.6")
                // if(stats->first < 5)
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
            if(fiveTuple.destinationAddress == "192.168.1.6")
            // if(stats->first < 5)
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

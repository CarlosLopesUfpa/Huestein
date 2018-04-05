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

#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/position-allocator.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/propagation-loss-model.h"
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
#include "ns3/wifi-phy.h"
using namespace ns3;

//Criação das Matrizes
double** create(int rows, int columns){
    double** table = new double*[rows];
    for(int i = 0; i < rows; i++) {
        table[i] = new double[columns]; 
        for(int j = 0; j < columns; j++){ 
        table[i][j] = 0; 
        }// sample set value;    
    }
    return table;
}
NS_LOG_COMPONENT_DEFINE ("Wifi_1");

void avalParam(int nAll, double** Vazao, double** Atraso, double** Loss, double** Energia, double** Jitterav);

int cenario = 1;

int main (int argc, char *argv[]) {

// Simulação 1: Reconhecimento da rede.
//Configurações da rede
    int nRn = 3;
    int nAll = 10; 
    double simTime = 100;
    uint32_t MaxPacketSize = 1024;
    double PacketInterval = 0.15;
    uint16_t grid = 50;
    std::string gr = std::to_string(grid);

    bool constant = true;
    bool random = false;
    
//Variáveis para receber dados do FlowMonitor
    int col = 1;
    double** Vazao = create(nAll*nRn, col);
    double** Atraso = create(nAll*nRn, col);
    double** Loss = create(nAll*nRn, col);
    double** Jitter = create(nAll*nRn, col);
    double atraso1 = 0;
    double atraso2 = 0;
    double** Energia = create(nAll*nRn, col);
    int aux_energy = 0;
      
//Criando Nós da Simulação
      

// Criando Grupo de todos os nós
  NodeContainer wifiAll;
  wifiAll.Create (nAll);

//Setar Mobilidade dos nós
  MobilityHelper mobilitywifiAll;


  mobilitywifiAll.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                            "X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max="+gr+"]"),
                            "Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max="+gr+"]"));

if(constant == true){
//Constante Mobillity
  mobilitywifiAll.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
}

if(random == true){
//Random Mobillity
  mobilitywifiAll.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                           "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=100.0]"),
                           "Bounds", StringValue ("-"+gr+"|"+gr+"|-"+gr+"|"+gr+""));
}
  mobilitywifiAll.Install (wifiAll);

//Configuração da rede
  std::string phyMode ("ErpOfdmRate54Mbps");
  bool verbose = false;

// The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
  if (verbose)
    {
      wifi.EnableLogComponents ();  // Turn on all Wifi logging
    }

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
// set it to zero; otherwise, gain will be added
  wifiPhy.Set ("RxGain", DoubleValue (0) );
// ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

// Criação do modelo de propagação da rede
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel",
                                  "Exponent", DoubleValue (3.0));

  wifiPhy.SetChannel (wifiChannel.Create ());

// Adição do mac
  WifiMacHelper wifiMac;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211g);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  
NetDeviceContainer allDevice;

//AD HOC - NETWORK
 
  wifiMac.SetType ("ns3::AdhocWifiMac");
  allDevice = wifi.Install (wifiPhy, wifiMac, wifiAll);



//Atribuindo energia nos dispositivos
      srand((unsigned)time(0));
      for (int l=0; l<nAll; ++l){
          aux_energy = rand() % 100 + 50; 

          Ptr<BasicEnergySource> energySource = CreateObject<BasicEnergySource>();
          Ptr<SimpleDeviceEnergyModel> energyModel = CreateObject<SimpleDeviceEnergyModel>();

          energySource->SetInitialEnergy (aux_energy);
          energyModel->SetEnergySource (energySource);
          energySource->AppendDeviceEnergyModel (energyModel);
          energyModel->SetCurrentA (20);

          // aggregate energy source to node
          wifiAll.Get(l)->AggregateObject (energySource);
          // energy[l][0] = energySource;
          Energia[l][0] = aux_energy;
        }

//Intalação da pilha TCP/IP nos nós
  InternetStackHelper internet;
  internet.Install (wifiAll);
    
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer allInterface = ipv4.Assign (allDevice);
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

// Criação da aplicação UDP
  uint16_t port = 4000;
  uint32_t rn = 0;
  std::string ipAp[nRn][1];
  uint32_t Rn[nRn][1];
  ApplicationContainer apps;
  
// ERRORRRR

  int vet[nRn][1] = {0, 1, 2};
      

  for(int s = 0; s < nRn; ++s){
    UdpServerHelper server (port);


    server.Install (wifiAll.Get(vet[s][0]));  
    
    if(s != vet[i][0]){
      rn = rand() % nAll;  
      Rn[s][0] = rn;
      server.Install (wifiAll.Get(rn));  
    }


    
    
    apps.Start (Seconds (0.1));
    apps.Stop (Seconds (simTime));
    
  // Set Random IP
    ipAp[s][0] = "192.168.1." + std::to_string(rn + 1);
  //Configuração da aplicação   
    UdpClientHelper client (Ipv4Address (ipAp[s][0].c_str()), port); 
    client.SetAttribute ("MaxPackets", UintegerValue ((uint32_t)(simTime*(1/PacketInterval))));
    client.SetAttribute ("Interval", TimeValue (Seconds (PacketInterval)));
    client.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
  // Intalar Aplicação em todos os nós(Usuários)
    apps = client.Install (wifiAll);
    apps.Start (Seconds (0.1));
    apps.Stop (Seconds (simTime));
}
      
//FLOW-MONITOR
//Intalar FlowMonitor em todos os Nós
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();
  
//Gerar animação
        AnimationInterface anim ("random_master_node.xml"); // Mandatory
        
        for (uint32_t i = 0; i < wifiAll.GetN(); ++i){
          anim.UpdateNodeDescription (wifiAll.Get(i), "Node"); // Optional
          anim.UpdateNodeColor (wifiAll.Get(i), 255, 0, 0); // Coloração
        }
        
        for (int i = 0; i<nRn; ++i){
            anim.UpdateNodeDescription (wifiAll.Get(Rn[i][0]), "Server"); // Optional
            anim.UpdateNodeColor (wifiAll.Get(Rn[i][0]), 255, 255, 255); // Coloração
        }
         
        anim.EnablePacketMetadata (); // Optiona
        anim.EnableIpv4RouteTracking ("routingtable-wireless.xml", Seconds (0), Seconds (5), Seconds (0.25)); //Optional
        anim.EnableWifiMacCounters (Seconds (0), Seconds (10)); //Optional
        anim.EnableWifiPhyCounters (Seconds (0), Seconds (10)); //Optional

  Simulator::Stop(Seconds(simTime));
  Simulator::Run();

//Imprimir resultados da simulação
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

int u = -1;

double sumThroughput = 0;
double sumLoss = 0;
double sumEnergy = 0;
double sumAtraso = 0;
double sumJitter = 0;

double medThroughput = 0;
double medLoss = 0;
double medEnergy = 0;
double medAtraso = 0;
double medJitter = 0;

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i){
     Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
     for(int s = 0; s<nRn; ++s){              
        if (t.destinationAddress == ipAp[s][0].c_str()){
            if(t.destinationAddress != t.sourceAddress){
                u++;
                std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
                std::cout<<"Duration  : "<<(i->second.timeLastRxPacket.GetSeconds()-i->second.timeFirstTxPacket.GetSeconds())<<std::endl;

                std::cout<<"Tx Packets = " << i->second.txPackets<<std::endl;
                std::cout<<"Rx Packets = " << i->second.rxPackets<<std::endl;

                Loss[u][0] = i->second.txPackets - i->second.rxPackets;
                std::cout << "Perda de Pacotes: "<< Loss[u][0]<<std::endl;


                sumLoss = sumLoss + Loss[u][0];

                Vazao[u][0] = i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds()-i->second.timeFirstTxPacket.GetSeconds())/1024/1024;
                std::cout << "Vazão: " << Vazao[u][0] << " Mbps\n";

                sumThroughput = sumThroughput + Vazao[u][0];

                std::cout << "Energia: "<< Energia[u][0] <<std::endl;

                sumEnergy = sumEnergy + Energia[u][0];

                Atraso[u][0] = ((i->second.timeLastRxPacket.GetSeconds()) - (i->second.timeLastTxPacket.GetSeconds()));
                std::cout << "Atraso: "<< Atraso[u][0] <<std::endl;

                sumAtraso = sumAtraso + Atraso[u][0];

                atraso2 = i->second.timeLastRxPacket.GetSeconds()-i->second.timeLastTxPacket.GetSeconds();
                atraso1 = i->second.timeFirstRxPacket.GetSeconds()-i->second.timeFirstTxPacket.GetSeconds();
                Jitter[u][0] = atraso2 - atraso1;
                std::cout << "Jitter: "<< Jitter[u][0] <<std::endl;

                sumJitter = sumJitter + Jitter[u][0];
                
                std::cout << " " <<std::endl;
            }
          } 
      }
  }
// Obter Média da Simulação
   medLoss = sumLoss / (u+1);
   medThroughput = sumThroughput / (u+1);
   medEnergy = sumEnergy / (u+1);
   medAtraso = sumAtraso / (u+1);
   medJitter = sumJitter / (u+1);

  std::cout << " " <<std::endl;
  std::cout << "Média Loss: "<< medLoss <<std::endl;
  std::cout << "Média Vazão: "<< medThroughput <<std::endl;
  std::cout << "Média Energia: "<< medEnergy <<std::endl;
  std::cout << "Média Atraso: "<< medAtraso <<std::endl;
  std::cout << "Média Jitter: "<< medJitter <<std::endl;
  std::cout << " " <<std::endl;

//LÓGICA DE SELEÇÃO
     

  Simulator::Destroy();
  return 0;

}

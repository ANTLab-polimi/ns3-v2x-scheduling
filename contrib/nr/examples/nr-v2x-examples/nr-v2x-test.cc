/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/**
 * \ingroup examples
 * \file cttc-nr-v2x-demo-simple.cc
 * \brief A cozy, simple, NR V2X demo (in a tutorial style)
 *
 * This example describes how to setup an NR sidelink out-of-coverage simulation
 * using the 3GPP channel model from TR 37.885. This example simulates a
 * simple topology consist of 2 UEs, where UE-1 transmits, and UE-2 receives.
 *
 * Have a look at the possible parameters to know what you can configure
 * through the command line.
 *
 * With the default configuration, the example will create a flow that will
 * go through a subband or bandwidth part. For that,
 * specifically, one band with a single CC, and one bandwidth part is used.
 *
 * The example will print on-screen the average Packet Inter-Reception (PIR)
 * type 2 computed as defined in 37.885. Moreover, it saves MAC and PHY layer
 * traces in a sqlite3 database using ns-3 stats module. Moreover, since there
 * is only one transmitter in the scenario, sensing is by default not enabled.
 *
 * \code{.unparsed}
$ ./ns3 run "cttc-nr-v2x-demo-simple --help"
    \endcode
 *
 */

/*
 * Include part. Often, you will have to include the headers for an entire module;
 * do that by including the name of the module you need with the suffix "-module.h".
 */

#include "ns3/core-module.h"
#include "ns3/config-store.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/nr-module.h"
#include "ns3/lte-module.h"
#include "ns3/stats-module.h"
#include "ns3/config-store-module.h"
#include "ns3/log.h"
#include "ns3/ue-mac-pscch-tx-output-stats.h"
#include "ns3/ue-mac-pssch-tx-output-stats.h"
#include "ns3/ue-phy-pscch-rx-output-stats.h"
#include "ns3/ue-phy-pssch-rx-output-stats.h"
#include "ns3/ue-to-ue-pkt-txrx-output-stats.h"
#include "ns3/ue-v2x-scheduling-stats.h"
#include "ns3/antenna-module.h"
#include "ns3/nr-sl-comm-resource-pool.h"
#include <iomanip>
#include <ns3/random-variable-stream.h>
#include <ns3/node-list.h>
#include <ns3/nr-ue-net-device.h>
#include <ns3/lte-ue-net-device.h>

#include "ns3/parameters-config.h"

#include "ns3/ns2-mobility-helper.h"
#include "ns3/sl-stats-helper.h"
#include "ns3/sl-mobility-stats.h"

/*
 * Use, always, the namespace ns3. All the NR classes are inside such namespace.
 */
using namespace ns3;

/*
 * With this line, we will be able to see the logs of the file by enabling the
 * component "CttcNrV2xDemoSimple", in this way:
 *
 * $ export NS_LOG="CttcNrV2xDemoSimple=level_info|prefix_func|prefix_time"
 */
NS_LOG_COMPONENT_DEFINE ("NrV2xTest");

/*
 * Global variables to count TX/RX packets and bytes.
 */

uint32_t rxByteCounter = 0; //!< Global variable to count RX bytes
uint32_t txByteCounter = 0; //!< Global variable to count TX bytes
uint32_t rxPktCounter = 0; //!< Global variable to count RX packets
uint32_t txPktCounter = 0; //!< Global variable to count TX packets

/**
 * \brief Method to listen the packet sink application trace Rx.
 * \param packet The packet
 */
void ReceivePacket (Ptr<const Packet> packet, const Address &)
{
  rxByteCounter += packet->GetSize();
  rxPktCounter++;
}

/**
 * \brief Method to listen the transmitting application trace Tx.
 * \param packet The packet
 */
void TransmitPacket (Ptr<const Packet> packet)
{
  txByteCounter += packet->GetSize();
  txPktCounter++;
}

/*
 * Global variable used to compute PIR
 */
uint64_t pirCounter = 0; //!< counter to count how many time we computed the PIR. It is used to compute average PIR
Time lastPktRxTime; //!< Global variable to store the RX time of a packet
Time pir; //!< Global varible to store PIR value

/**
 * \brief This method listens to the packet sink application trace Rx.
 * \param packet The packet
 * \param from The address of the transmitter
 */
void ComputePir (Ptr<const Packet> packet, const Address &)
{
  if (pirCounter == 0 && lastPktRxTime.GetSeconds () == 0.0)
    {
      //this the first packet, just store the time and get out
      lastPktRxTime = Simulator::Now ();
      return;
    }
  pir = pir + (Simulator::Now () - lastPktRxTime);
  lastPktRxTime = Simulator::Now ();
  pirCounter++;
}

int 
main (int argc, char *argv[])
{
  /*
   * Variables that represent the parameters we will accept as input by the
   * command line. Each of them is initialized with a default value.
   */
  // Scenario parameters (that we will use inside this script):
  uint16_t interUeDistance = 2; //meters
  bool logging = true;


  // Traffic parameters (that we will use inside this script:)
  bool useIPv6 = false; // default IPV4
  uint32_t udpPacketSizeBe = 200;
  double dataRateBe = 160; //16 kilobits per second

  // Simulation parameters.
  Time simTime = Seconds (30);
  //Sidelink bearers activation time
  Time slBearersActivationTime = Seconds (2.0);


  // NR parameters. We will take the input from the command line, and then we
  // will pass them inside the NR module.
  uint16_t numerologyBwpSl = 2;
  double centralFrequencyBandSl = 5.89e9; // band n47  TDD //Here band is analogous to channel
  uint16_t bandwidthBandSl = 400; //Multiple of 100 KHz; 400 = 40 MHz
  double txPower = 23; //dBm

  // Where we will store the output files.

  // modified
  std::string ltePlmnId = "111";
	std::string ricE2TermIpAddress = "10.0.2.10";
	uint16_t ricE2TermPortNumber = 36422;
	uint16_t e2startingPort = 38470;
  std::string simTag = "nr-v2x-test";
	std::string outputDir = "/storage/franci/simulations/ns3-v2x/nr-v2x-mode2/";
  bool e2cuCp = true;
  bool e2du = false;
  bool isXappEnabled = false;
  uint8_t schedulingType = 2;// default is ue Selected (2) and 3 is ORAN SELECTED
  double ueDiscCenterXPosition = 0;
  double ueDiscCenterYPosition = 0;
  uint16_t ueNum = 20; // ue number 

  std::string traceFile = outputDir+"new_mobility.tcl";

  // end modification

  /*
   * From here, we instruct the ns3::CommandLine class of all the input parameters
   * that we may accept as input, as well as their description, and the storage
   * variable.
   */
  CommandLine cmd (__FILE__);

  cmd.AddValue ("interUeDistance",
                "The distance among the UEs in the topology",
                interUeDistance);
  cmd.AddValue ("logging",
                "Enable logging",
                logging);
  cmd.AddValue ("useIPv6",
                "Use IPv6 instead of IPv4",
                useIPv6);
  cmd.AddValue ("packetSizeBe",
                "packet size in bytes to be used by best effort traffic",
                udpPacketSizeBe);
  cmd.AddValue ("dataRateBe",
                "The data rate in kilobits per second for best effort traffic",
                dataRateBe);
  cmd.AddValue ("simTime",
                "Simulation time in seconds",
                simTime);
  cmd.AddValue ("slBearerActivationTime",
                "Sidelik bearer activation time in seconds",
                slBearersActivationTime);
  cmd.AddValue ("numerologyBwpSl",
                "The numerology to be used in sidelink bandwidth part",
                numerologyBwpSl);
  cmd.AddValue ("centralFrequencyBandSl",
                "The central frequency to be used for sidelink band/channel",
                centralFrequencyBandSl);
  cmd.AddValue ("bandwidthBandSl",
                "The system bandwidth to be used for sidelink",
                bandwidthBandSl);
  cmd.AddValue ("txPower",
                "total tx power in dBm",
                txPower);
  cmd.AddValue ("simTag",
                "tag to be appended to output filenames to distinguish simulation campaigns",
                simTag);
  cmd.AddValue ("outputDir",
                "directory where to store simulation results",
                outputDir);
  cmd.AddValue ("ueNumber",
                "Number of ues in the simulation. Default is 50",
                ueNum);
  
  // modified

  cmd.AddValue("ricE2TermIpAddress",
				 "The ip address to reach the termination point of ric",
				 ricE2TermIpAddress);
  cmd.AddValue("ricE2TermPortNumber",
				 "The port number of the E2 termination endpoint",
				 ricE2TermPortNumber);
  cmd.AddValue("ltePlmnId",
				 "Plmn id of the lte coordinator, to distinguish different simulations campaigns",
				 ltePlmnId);
  cmd.AddValue("e2StartingPort",
				 "starting port number for the gnb e2 termination; destination is same",
				 e2startingPort);
  cmd.AddValue("isXappEnabled",
				 "Define if the simulation has the support of Xapp",
				 isXappEnabled);
  cmd.AddValue("ueSchedulingType",
				 "Scheduling type: UE or ORAN",
				 schedulingType);
  cmd.AddValue("ueDiscCenterXPosition",
				 "The x coordinate of the disc center of the ue position",
				 ueDiscCenterXPosition);
  cmd.AddValue("ueDiscCenterYPosition",
				 "The y coordinate of the disc center of the ue position",
				 ueDiscCenterYPosition);
  cmd.AddValue("traceFile",
          "Ns2 movement trace file",
          traceFile);
  // end modification
  
  // Parse the command line
  cmd.Parse (argc, argv);

  // ParametersConfig::EnableTraces();

  if (logging)
    {
      LogLevel logLevel = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_ALL | LOG_DEBUG | LOG_LEVEL_FUNCTION);
      // LogComponentEnable ("UdpClient", logLevel);
      // LogComponentEnable ("UdpServer", logLevel);
      // LogComponentEnable ("LtePdcp", logLevel);
      // LogComponentEnable ("NrSlHelper", logLevel);
      // LogComponentEnable ("NrSlUeRrc", logLevel);
      LogComponentEnable ("NrUeMac", logLevel);
      // LogComponentEnable ("NrUePhy", logLevel);
      // LogComponentEnable ("NrSpectrumPhy", logLevel);
      // LogComponentEnable ("E2Termination", logLevel);
      // LogComponentEnable ("NrGnbNetDevice", logLevel);
      // LogComponentEnable ("Asn1Types", logLevel);
      // LogComponentEnable ("ThreeGppSpectrumPropagationLossModel", logLevel);

      
    }

  double indicationPeriodicity = 0.01; // seconds


  // Config::SetDefault ("ns3::LteEnbNetDevice::ControlFileName", StringValue (controlFilename));
  Config::SetDefault ("ns3::LteEnbNetDevice::E2Periodicity", DoubleValue (indicationPeriodicity));
  Config::SetDefault ("ns3::NrGnbNetDevice::E2Periodicity", DoubleValue (indicationPeriodicity));

  Config::SetDefault ("ns3::NrGnbNetDevice::EnableCuCpReport", BooleanValue (e2cuCp));
  Config::SetDefault ("ns3::NrGnbNetDevice::EnableDuReport", BooleanValue (e2du));

  Config::SetDefault ("ns3::NrHelper::E2TermIp", StringValue (ricE2TermIpAddress));
  Config::SetDefault ("ns3::NrHelper::E2Port", UintegerValue (ricE2TermPortNumber));

  Config::SetDefault ("ns3::NrHelper::E2LocalPort", UintegerValue (e2startingPort));

  Config::SetDefault ("ns3::NrHelper::PlmnId", StringValue (ltePlmnId));
  Config::SetDefault ("ns3::NrGnbNetDevice::PlmnId", StringValue (ltePlmnId));

  Config::SetDefault ("ns3::NrHelper::E2ModeNr", BooleanValue (isXappEnabled));

  Config::SetDefault ("ns3::NrGnbNetDevice::TracesPath",
                      StringValue(outputDir + simTag + "/"));
  Config::SetDefault ("ns3::LteEnbNetDevice::TracesPath",
                      StringValue(outputDir + simTag + "/"));

  NrSlCommResourcePool::SchedulingType schedulingTypeValue = static_cast<NrSlCommResourcePool::SchedulingType>(schedulingType);
  std::cout << "Scheduling type " << schedulingTypeValue << std::endl;
  Config::SetDefault ("ns3::LteUeRrc::V2XSchedulingType", EnumValue (schedulingTypeValue));

  // ParametersConfig::CreateTracesDir(params);

  // Final simulation time is the addition of:
  //simTime + slBearersActivationTime + 10 ms additional delay for UEs to activate the bearers
  Time finalSlBearersActivationTime = slBearersActivationTime + Seconds (0.01);
  Time finalSimTime = simTime + finalSlBearersActivationTime;
  std::cout << "Final Simulation duration " << finalSimTime.GetSeconds () << std::endl;

  /*
   * Check if the frequency is in the allowed range.
   * If you need to add other checks, here is the best position to put them.
   */
  NS_ABORT_IF (centralFrequencyBandSl > 6e9);

  /*
   * Default values for the simulation. We are progressively removing all
   * the instances of SetDefault, but we need it for legacy code (LTE)
   */
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));

  Ns2MobilityHelper ns2 = Ns2MobilityHelper(traceFile);

  NodeContainer ueVoiceContainer;
  ueVoiceContainer.Create (ueNum);

  NodeContainer enbNodes;
  enbNodes.Create (1);

  // MobilityHelper mobility;
  // mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  // Ptr<ListPositionAllocator> positionAllocUe = CreateObject<ListPositionAllocator> ();
  // for (uint16_t i = 0; i < ueNum; i++)
  //   {
  //     positionAllocUe->Add (Vector (interUeDistance * i, 0.0, 1.5));
  //   }
  // mobility.SetPositionAllocator (positionAllocUe);
  // mobility.Install (ueVoiceContainer);

  // double rho = 500;
  // Ptr<UniformDiscPositionAllocator> uePositionAlloc = CreateObject<UniformDiscPositionAllocator> ();
  // uePositionAlloc->SetX(ueDiscCenterXPosition);
  // uePositionAlloc->SetY(ueDiscCenterYPosition);
  //  uePositionAlloc->SetZ(1.5);
  // uePositionAlloc->SetRho (rho);
  // Ptr<UniformRandomVariable> speed = CreateObject<UniformRandomVariable> ();
  // speed->SetAttribute ("Min", DoubleValue (0.5));
  // speed->SetAttribute ("Max", DoubleValue (1.8));
  // mobility.SetMobilityModel ("ns3::RandomWalk2dOutdoorMobilityModel", "Speed",
  //                               PointerValue (speed), "Bounds",
  //                               RectangleValue (Rectangle (ueDiscCenterXPosition-rho, ueDiscCenterXPosition+rho, ueDiscCenterYPosition-rho, ueDiscCenterYPosition+rho)));
  // mobility.SetPositionAllocator (uePositionAlloc);
  // mobility.Install (ueVoiceContainer);
  // install the mobility of nodes
  // TODO: check the mobility of gnb
  ns2.Install();

  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();

  // Put the pointers inside nrHelper
  nrHelper->SetEpcHelper (epcHelper);

  /*
   * Spectrum division. We create one operational band, containing
   * one component carrier, and a single bandwidth part
   * centered at the frequency specified by the input parameters.
   * We will use the StreetCanyon channel modeling.
   */
  BandwidthPartInfoPtrVector allBwps;
  CcBwpCreator ccBwpCreator;
  const uint8_t numCcPerBand = 1;

  /* Create the configuration for the CcBwpHelper. SimpleOperationBandConf
   * creates a single BWP per CC
   */
  CcBwpCreator::SimpleOperationBandConf bandConfSl (centralFrequencyBandSl, bandwidthBandSl, numCcPerBand, BandwidthPartInfo::V2V_Highway);

  // By using the configuration created, it is time to make the operation bands
  OperationBandInfo bandSl = ccBwpCreator.CreateOperationBandContiguousCc (bandConfSl);

  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod",TimeValue (MilliSeconds(100)));
  nrHelper->SetChannelConditionModelAttribute ("UpdatePeriod", TimeValue (MilliSeconds (0)));
  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));

  nrHelper->InitializeOperationBand (&bandSl);
  allBwps = CcBwpCreator::GetAllBwps ({bandSl});

  Packet::EnableChecking ();
  Packet::EnablePrinting ();

  /*
   *  Case (i): Attributes valid for all the nodes
   */
  // Core latency
  epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (0)));

  /*
   * Antennas for all the UEs
   * We are not using beamforming in SL, rather we are using
   * quasi-omnidirectional transmission and reception, which is the default
   * configuration of the beams.
   */
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (1));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (2));
  nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  nrHelper->SetUePhyAttribute ("TxPower", DoubleValue (txPower));

  //NR Sidelink attribute of UE MAC, which are would be common for all the UEs
  nrHelper->SetUeMacAttribute ("EnableSensing", BooleanValue (false));
  nrHelper->SetUeMacAttribute ("T1", UintegerValue (2));
  nrHelper->SetUeMacAttribute ("T2", UintegerValue (33));
  nrHelper->SetUeMacAttribute ("ActivePoolId", UintegerValue (0));
  nrHelper->SetUeMacAttribute ("ReservationPeriod", TimeValue (MilliSeconds(100)));
  nrHelper->SetUeMacAttribute ("NumSidelinkProcess", UintegerValue (4));
  nrHelper->SetUeMacAttribute ("EnableBlindReTx", BooleanValue (true));


  uint8_t bwpIdForGbrMcptt = 0;

  nrHelper->SetBwpManagerTypeId (TypeId::LookupByName ("ns3::NrSlBwpManagerUe"));
  //following parameter has no impact at the moment because:
  //1. No support for PQI based mapping between the application and the LCs
  //2. No scheduler to consider PQI
  //However, till such time all the NR SL examples should use GBR_MC_PUSH_TO_TALK
  //because we hard coded the PQI 65 in UE RRC.
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("GBR_MC_PUSH_TO_TALK", UintegerValue (bwpIdForGbrMcptt));

  std::set<uint8_t> bwpIdContainer;
  bwpIdContainer.insert (bwpIdForGbrMcptt);

  // create single gnb device to get the messages
  double frequencyGnb = 28e9; // central frequency
  double bandwidthGnb = 100e6; //bandwidth
  const uint8_t numCcPerBandGnb = 1;
  enum BandwidthPartInfo::Scenario scenarioEnumGnb = BandwidthPartInfo::UMa;
  CcBwpCreator::SimpleOperationBandConf bandConfGnb (frequencyGnb, bandwidthGnb, numCcPerBandGnb, scenarioEnumGnb);
  OperationBandInfo bandGnb = ccBwpCreator.CreateOperationBandContiguousCc (bandConfGnb);
  //Initialize channel and pathloss, plus other things inside band.
  nrHelper->InitializeOperationBand (&bandGnb);

  BandwidthPartInfoPtrVector allBwpsGnb = CcBwpCreator::GetAllBwps ({bandGnb});

  // position the base stations
  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
  enbPositionAlloc->Add (Vector (0.0, 0.0, 35));
  // enbPositionAlloc->Add (Vector (0.0, 80.0, 35));
  MobilityHelper enbmobility;
  enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbmobility.SetPositionAllocator (enbPositionAlloc);
  enbmobility.Install (enbNodes);

  NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice (enbNodes, allBwpsGnb);

  Config::SetDefault ("ns3::NrGnbPhy::Numerology", UintegerValue (numerologyBwpSl));

  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetTxPower (-10);
  DynamicCast<NrGnbNetDevice> (enbNetDev.Get (0))->UpdateConfig ();

  /*
   * We have configured the attributes we needed. Now, install and get the pointers
   * to the NetDevices, which contains all the NR stack:
   */
  NetDeviceContainer ueVoiceNetDev = nrHelper->InstallUeDevice (ueVoiceContainer, allBwps);

  // When all the configuration is done, explicitly call UpdateConfig ()
  for (auto it = ueVoiceNetDev.Begin (); it != ueVoiceNetDev.End (); ++it)
  {
    // DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    Ptr<NrUeNetDevice> nruedev = DynamicCast<NrUeNetDevice> (*it);
    nruedev->UpdateConfig ();

  }

  /*
   * Configure Sidelink. We create the following helpers needed for the
   * NR Sidelink, i.e., V2X simulation:
   * - NrSlHelper, which will configure the UEs protocol stack to be ready to
   *   perform Sidelink related procedures.
   * - EpcHelper, which takes care of triggering the call to EpcUeNas class
   *   to establish the NR Sidelink bearer (s). We note that, at this stage
   *   just communicate the pointer of already instantiated EpcHelper object,
   *   which is the same pointer communicated to the NrHelper above.
   */
  Ptr<NrSlHelper> nrSlHelper = CreateObject <NrSlHelper> ();
  // Put the pointers inside NrSlHelper
  nrSlHelper->SetEpcHelper (epcHelper);

  /*
   * Set the SL error model and AMC
   * Error model type: ns3::NrEesmCcT1, ns3::NrEesmCcT2, ns3::NrEesmIrT1,
   *                   ns3::NrEesmIrT2, ns3::NrLteMiErrorModel
   * AMC type: NrAmc::ShannonModel or NrAmc::ErrorModel
   */
  std::string errorModel = "ns3::NrEesmIrT1";
  nrSlHelper->SetSlErrorModel (errorModel);
  nrSlHelper->SetUeSlAmcAttribute ("AmcModel", EnumValue (NrAmc::ErrorModel));

  /*
   * Set the SL scheduler attributes
   * In this example we use NrSlUeMacSchedulerSimple scheduler, which uses
   * fix MCS value
   */
  nrSlHelper->SetNrSlSchedulerTypeId (NrSlUeMacSchedulerSimple::GetTypeId());
  nrSlHelper->SetUeSlSchedulerAttribute ("FixNrSlMcs", BooleanValue (true));
  nrSlHelper->SetUeSlSchedulerAttribute ("InitialNrSlMcs", UintegerValue (14));

  nrSlHelper->PrepareUeForSidelink (ueVoiceNetDev, bwpIdContainer);

  //SlResourcePoolNr IE
  LteRrcSap::SlResourcePoolNr slResourcePoolNr;
  //get it from pool factory
  Ptr<NrSlCommPreconfigResourcePoolFactory> ptrFactory = Create<NrSlCommPreconfigResourcePoolFactory> ();
  std::vector <std::bitset<1> > slBitmap = {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1};
  ptrFactory->SetSlTimeResources (slBitmap);
  ptrFactory->SetSlSensingWindow (100); // T0 in ms
  ptrFactory->SetSlSelectionWindow (5);
  ptrFactory->SetSlFreqResourcePscch (10); // PSCCH RBs
  ptrFactory->SetSlSubchannelSize (50);
  ptrFactory->SetSlMaxNumPerReserve (3);
  //Once parameters are configured, we can create the pool
  LteRrcSap::SlResourcePoolNr pool = ptrFactory->CreatePool ();
  slResourcePoolNr = pool;

  //Configure the SlResourcePoolConfigNr IE, which hold a pool and its id
  LteRrcSap::SlResourcePoolConfigNr slresoPoolConfigNr;
  slresoPoolConfigNr.haveSlResourcePoolConfigNr = true;
  //Pool id, ranges from 0 to 15
  uint16_t poolId = 0;
  LteRrcSap::SlResourcePoolIdNr slResourcePoolIdNr;
  slResourcePoolIdNr.id = poolId;
  slresoPoolConfigNr.slResourcePoolId = slResourcePoolIdNr;
  slresoPoolConfigNr.slResourcePool = slResourcePoolNr;

  //Configure the SlBwpPoolConfigCommonNr IE, which hold an array of pools
  LteRrcSap::SlBwpPoolConfigCommonNr slBwpPoolConfigCommonNr;
  //Array for pools, we insert the pool in the array as per its poolId
  slBwpPoolConfigCommonNr.slTxPoolSelectedNormal [slResourcePoolIdNr.id] = slresoPoolConfigNr;

  //Configure the BWP IE
  LteRrcSap::Bwp bwp;
  bwp.numerology = numerologyBwpSl;
  bwp.symbolsPerSlots = 14;
  bwp.rbPerRbg = 1;
  bwp.bandwidth = bandwidthBandSl;

  //Configure the SlBwpGeneric IE
  LteRrcSap::SlBwpGeneric slBwpGeneric;
  slBwpGeneric.bwp = bwp;
  slBwpGeneric.slLengthSymbols = LteRrcSap::GetSlLengthSymbolsEnum (14);
  slBwpGeneric.slStartSymbol = LteRrcSap::GetSlStartSymbolEnum (0);

  //Configure the SlBwpConfigCommonNr IE
  LteRrcSap::SlBwpConfigCommonNr slBwpConfigCommonNr;
  slBwpConfigCommonNr.haveSlBwpGeneric = true;
  slBwpConfigCommonNr.slBwpGeneric = slBwpGeneric;
  slBwpConfigCommonNr.haveSlBwpPoolConfigCommonNr = true;
  slBwpConfigCommonNr.slBwpPoolConfigCommonNr = slBwpPoolConfigCommonNr;

  //Configure the SlFreqConfigCommonNr IE, which hold the array to store
  //the configuration of all Sidelink BWP (s).
  LteRrcSap::SlFreqConfigCommonNr slFreConfigCommonNr;
  //Array for BWPs. Here we will iterate over the BWPs, which
  //we want to use for SL.
  for (const auto &it:bwpIdContainer)
    {
      // it is the BWP id
      slFreConfigCommonNr.slBwpList [it] = slBwpConfigCommonNr;
    }

  //Configure the TddUlDlConfigCommon IE
  LteRrcSap::TddUlDlConfigCommon tddUlDlConfigCommon;
  tddUlDlConfigCommon.tddPattern = "DL|DL|DL|F|UL|UL|UL|UL|UL|UL|";

  //Configure the SlPreconfigGeneralNr IE
  LteRrcSap::SlPreconfigGeneralNr slPreconfigGeneralNr;
  slPreconfigGeneralNr.slTddConfig = tddUlDlConfigCommon;

  //Configure the SlUeSelectedConfig IE
  LteRrcSap::SlUeSelectedConfig slUeSelectedPreConfig;
  slUeSelectedPreConfig.slProbResourceKeep = 0;
  //Configure the SlPsschTxParameters IE
  LteRrcSap::SlPsschTxParameters psschParams;
  psschParams.slMaxTxTransNumPssch = 5;
  //Configure the SlPsschTxConfigList IE
  LteRrcSap::SlPsschTxConfigList pscchTxConfigList;
  pscchTxConfigList.slPsschTxParameters [0] = psschParams;
  slUeSelectedPreConfig.slPsschTxConfigList = pscchTxConfigList;

  /*
   * Finally, configure the SidelinkPreconfigNr This is the main structure
   * that needs to be communicated to NrSlUeRrc class
   */
  LteRrcSap::SidelinkPreconfigNr slPreConfigNr;
  slPreConfigNr.slPreconfigGeneral = slPreconfigGeneralNr;
  slPreConfigNr.slUeSelectedPreConfig = slUeSelectedPreConfig;
  slPreConfigNr.slPreconfigFreqInfoList [0] = slFreConfigCommonNr;

  //Communicate the above pre-configuration to the NrSlHelper
  nrSlHelper->InstallNrSlPreConfiguration (ueVoiceNetDev, slPreConfigNr);

  /****************************** End SL Configuration ***********************/

  /*
   * Fix the random streams
   */
  int64_t stream = 1;
  stream += nrHelper->AssignStreams (ueVoiceNetDev, stream);
  stream += nrSlHelper->AssignStreams (ueVoiceNetDev, stream);

  InternetStackHelper internet;
  internet.Install (ueVoiceContainer);
  stream += internet.AssignStreams (ueVoiceContainer, stream);
  uint32_t dstL2Id = 255;
  Ipv4Address groupAddress4 ("225.0.0.0");     //use multicast address as destination
  Ipv6Address groupAddress6 ("ff0e::1");     //use multicast address as destination
  Address remoteAddress;
  Address localAddress;
  uint16_t port = 8000;
  Ptr<LteSlTft> tft;
  if (!useIPv6)
    {
      Ipv4InterfaceContainer ueIpIface;
      ueIpIface = epcHelper->AssignUeIpv4Address (ueVoiceNetDev);

      // set the default gateway for the UE
      Ipv4StaticRoutingHelper ipv4RoutingHelper;
      for (uint32_t u = 0; u < ueVoiceContainer.GetN (); ++u)
        {
          Ptr<Node> ueNode = ueVoiceContainer.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }
      remoteAddress = InetSocketAddress (groupAddress4, port);
      localAddress = InetSocketAddress (Ipv4Address::GetAny (), port);
      tft = Create<LteSlTft> (LteSlTft::Direction::BIDIRECTIONAL, LteSlTft::CommType::GroupCast, groupAddress4, dstL2Id);
      //Set Sidelink bearers
      nrSlHelper->ActivateNrSlBearer (finalSlBearersActivationTime, ueVoiceNetDev, tft);
    }
  else
    {
      Ipv6InterfaceContainer ueIpIface;
      ueIpIface = epcHelper->AssignUeIpv6Address (ueVoiceNetDev);

      // set the default gateway for the UE
      Ipv6StaticRoutingHelper ipv6RoutingHelper;
      for (uint32_t u = 0; u < ueVoiceContainer.GetN (); ++u)
        {
          Ptr<Node> ueNode = ueVoiceContainer.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv6StaticRouting> ueStaticRouting = ipv6RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv6> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress6 (), 1);
        }
      remoteAddress = Inet6SocketAddress (groupAddress6, port);
      localAddress = Inet6SocketAddress (Ipv6Address::GetAny (), port);
      tft = Create<LteSlTft> (LteSlTft::Direction::BIDIRECTIONAL, LteSlTft::CommType::GroupCast, groupAddress6, dstL2Id);
      //Set Sidelink bearers
      nrSlHelper->ActivateNrSlBearer (finalSlBearersActivationTime, ueVoiceNetDev, tft);
    }

  //Set Application in the UEs
  OnOffHelper sidelinkClient ("ns3::UdpSocketFactory", remoteAddress);
  sidelinkClient.SetAttribute ("EnableSeqTsSizeHeader", BooleanValue (true));
  std::string dataRateBeString  = std::to_string (dataRateBe) + "kb/s";
  std::cout << "Data rate " << DataRate (dataRateBeString) << std::endl;
  sidelinkClient.SetConstantRate (DataRate (dataRateBeString), udpPacketSizeBe);

  // install odd index of ue
  ApplicationContainer clientApps;
  for (uint32_t _ind = 0; _ind<ueVoiceContainer.GetN(); _ind+=2){
    clientApps.Add(sidelinkClient.Install (ueVoiceContainer.Get (_ind)));
  }
  
  // ApplicationContainer clientApps = sidelinkClient.Install (ueVoiceContainer.Get (0));
  //onoff application will send the first packet at :
  //finalSlBearersActivationTime + ((Pkt size in bits) / (Data rate in bits per sec))
  clientApps.Start (finalSlBearersActivationTime);
  clientApps.Stop (finalSimTime);

  //Output app start, stop and duration
  double realAppStart =  finalSlBearersActivationTime.GetSeconds () + ((double)udpPacketSizeBe * 8.0 / (DataRate (dataRateBeString).GetBitRate ()));
  double appStopTime = (finalSimTime).GetSeconds ();

  std::cout << "App start time " << realAppStart << " sec" << std::endl;
  std::cout << "App stop time " << appStopTime << " sec" << std::endl;


  ApplicationContainer serverApps;
  PacketSinkHelper sidelinkSink ("ns3::UdpSocketFactory", localAddress);
  sidelinkSink.SetAttribute ("EnableSeqTsSizeHeader", BooleanValue (true));
  for (uint32_t _ind = 0; _ind<ueVoiceContainer.GetN(); ++_ind){
    serverApps.Add(sidelinkSink.Install (ueVoiceContainer.Get (_ind)));
  }
  // serverApps = sidelinkSink.Install (ueVoiceContainer.Get (ueVoiceContainer.GetN () - 1));
  serverApps.Start (Seconds (2.0));

  // modified
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUePhy/SensingData",
                                   MakeBoundCallback (&NrGnbNetDevice::SciReceptionCallback, DynamicCast<NrGnbNetDevice> (enbNetDev.Get (0))));

  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUeMac/E2PacketDelayReport",
                                   MakeBoundCallback (&NrGnbNetDevice::PacketDelaysInBufferCallback, DynamicCast<NrGnbNetDevice> (enbNetDev.Get (0))));
  
  
  // connect the trace coming from the gnb to the ue mac
  

  for (auto it = ueVoiceNetDev.Begin (); it != ueVoiceNetDev.End (); ++it)
    {
      Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrGnbNetDevice/SlV2XScheduling",
                                   MakeBoundCallback (&NrUeMac::V2xE2SchedulingTraceCallback, DynamicCast<NrUeMac> (DynamicCast<NrUeNetDevice> (*it)->GetMac (0))));
    }

  //Trace receptions; use the following to be robust to node ID changes
  std::ostringstream path;
  path << "/NodeList/" << ueVoiceContainer.Get (1)->GetId () << "/ApplicationList/0/$ns3::PacketSink/Rx";
  Config::ConnectWithoutContext(path.str (), MakeCallback (&ReceivePacket));
  path.str ("");

  path << "/NodeList/" << ueVoiceContainer.Get (1)->GetId () << "/ApplicationList/0/$ns3::PacketSink/Rx";
  Config::ConnectWithoutContext(path.str (), MakeCallback (&ComputePir));
  path.str ("");

  path << "/NodeList/" << ueVoiceContainer.Get (0)->GetId () << "/ApplicationList/0/$ns3::OnOffApplication/Tx";
  Config::ConnectWithoutContext(path.str (), MakeCallback (&TransmitPacket));
  path.str ("");

  //Datebase setup
  std::string exampleName = simTag + "/" + "nr-v2x-simple-demo.db";
  SQLiteOutput db (outputDir + exampleName);

  // modified
  UeV2XScheduling v2xSchedulingxApp;
  v2xSchedulingxApp.SetDb (&db, "v2XSchedulingXapp"); 
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrGnbNetDevice/SlV2XScheduling",
                                   MakeBoundCallback (&SlStatsHelper::NotifyxAppScheduling, &v2xSchedulingxApp, DynamicCast<NrGnbNetDevice> (enbNetDev.Get (0))->GetPhy(0)));

  // Mode 2 scheduling
  UeV2XScheduling v2xSchedulingMode2;
  v2xSchedulingMode2.SetDb (&db, "v2xSchedulingMode2"); 
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUeMac/UeSlV2XScheduling",
                                   MakeBoundCallback (&SlStatsHelper::NotifyUeScheduling, &v2xSchedulingMode2));

  // mobility 
  SlMobilityStats mobilityStats;
  mobilityStats.SetDb (&db, "mobility"); 

  // end modification

  UeMacPscchTxOutputStats pscchStats;
  pscchStats.SetDb (&db, "pscchTxUeMac");
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUeMac/SlPscchScheduling",
                                   MakeBoundCallback (&SlStatsHelper::NotifySlPscchScheduling, &pscchStats));

  UeMacPsschTxOutputStats psschStats;
  psschStats.SetDb (&db, "psschTxUeMac");
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUeMac/SlPsschScheduling",
                                   MakeBoundCallback (&SlStatsHelper::NotifySlPsschScheduling, &psschStats));


  UePhyPscchRxOutputStats pscchPhyStats;
  pscchPhyStats.SetDb (&db, "pscchRxUePhy");
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUePhy/NrSpectrumPhyList/*/RxPscchTraceUe",
                                   MakeBoundCallback (&SlStatsHelper::NotifySlPscchRx, &pscchPhyStats));

  UePhyPsschRxOutputStats psschPhyStats;
  psschPhyStats.SetDb (&db, "psschRxUePhy");
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUePhy/NrSpectrumPhyList/*/RxPsschTraceUe",
                                   MakeBoundCallback (&SlStatsHelper::NotifySlPsschRx, &psschPhyStats));

  UeToUePktTxRxOutputStats pktStats;
  pktStats.SetDb (&db, "pktTxRx");

  if (!useIPv6)
    {
      // Set Tx traces
      for (uint16_t ac = 0; ac < clientApps.GetN (); ac++)
        {
          Ipv4Address localAddrs =  clientApps.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
          std::cout << "Tx address: " << localAddrs << std::endl;
          // clientApps.Get (ac)->TraceConnect ("TxWithSeqTsSize", "tx", MakeBoundCallback (&UePacketTraceDb, &pktStats, ueVoiceContainer.Get (0), localAddrs));
          clientApps.Get (ac)->TraceConnect ("TxWithSeqTsSize", "tx", MakeBoundCallback (&SlStatsHelper::UePacketTraceDb, &pktStats, clientApps.Get (ac)->GetNode (), localAddrs));
        }

      // Set Rx traces
      for (uint16_t ac = 0; ac < serverApps.GetN (); ac++)
        {
          Ipv4Address localAddrs =  serverApps.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
          std::cout << "Rx address: " << localAddrs << std::endl;
          
          // serverApps.Get (ac)->TraceConnect ("RxWithSeqTsSize", "rx", MakeBoundCallback (&UePacketTraceDb, &pktStats, ueVoiceContainer.Get (1), localAddrs));
          serverApps.Get (ac)->TraceConnect ("RxWithSeqTsSize", "rx", MakeBoundCallback (&SlStatsHelper::UePacketTraceDb, &pktStats, serverApps.Get (ac)->GetNode (), localAddrs));
        }
    }
  else
    {
      // Set Tx traces
      for (uint16_t ac = 0; ac < clientApps.GetN (); ac++)
        {
          clientApps.Get (ac)->GetNode ()->GetObject<Ipv6L3Protocol> ()->AddMulticastAddress (groupAddress6);
          Ipv6Address localAddrs =  clientApps.Get (ac)->GetNode ()->GetObject<Ipv6L3Protocol> ()->GetAddress (1,1).GetAddress ();
          std::cout << "Tx address: " << localAddrs << std::endl;
          // clientApps.Get (ac)->TraceConnect ("TxWithSeqTsSize", "tx", MakeBoundCallback (&UePacketTraceDb, &pktStats, ueVoiceContainer.Get (0), localAddrs));
          clientApps.Get (ac)->TraceConnect ("TxWithSeqTsSize", "tx", MakeBoundCallback (&SlStatsHelper::UePacketTraceDb, &pktStats, clientApps.Get (ac)->GetNode (), localAddrs));
        }

      // Set Rx traces
      for (uint16_t ac = 0; ac < serverApps.GetN (); ac++)
        {
          serverApps.Get (ac)->GetNode ()->GetObject<Ipv6L3Protocol> ()->AddMulticastAddress (groupAddress6);
          Ipv6Address localAddrs =  serverApps.Get (ac)->GetNode ()->GetObject<Ipv6L3Protocol> ()->GetAddress (1,1).GetAddress ();
          std::cout << "Rx address: " << localAddrs << std::endl;
          // serverApps.Get (ac)->TraceConnect ("RxWithSeqTsSize", "rx", MakeBoundCallback (&UePacketTraceDb, &pktStats, ueVoiceContainer.Get (ac), localAddrs));
          serverApps.Get (ac)->TraceConnect ("RxWithSeqTsSize", "rx", MakeBoundCallback (&SlStatsHelper::UePacketTraceDb, &pktStats, serverApps.Get (ac)->GetNode (), localAddrs));
        }
    }

  for (auto it = ueVoiceNetDev.Begin (); it != ueVoiceNetDev.End (); ++it)
  {
    Ptr<NrUeNetDevice> dev = DynamicCast<NrUeNetDevice> (*it);

    Ptr<MobilityModel> mm = dev->GetNode()->GetObject<MobilityModel> ();
    mm->TraceConnectWithoutContext("CourseChange", MakeBoundCallback (&SlStatsHelper::CourseChange, &mobilityStats, (*it), "ue"));
  }

  for (auto it = enbNetDev.Begin (); it != enbNetDev.End (); ++it)
  {
    Ptr<NrGnbNetDevice> dev = DynamicCast<NrGnbNetDevice> (*it);

    Ptr<MobilityModel> mm = dev->GetNode()->GetObject<MobilityModel> ();
    mm->TraceConnectWithoutContext("CourseChange", MakeBoundCallback (&SlStatsHelper::CourseChange, &mobilityStats, (*it), "gnb"));
  }

  Simulator::Stop (finalSimTime);
  Simulator::Run ();

  std::cout << "Total Tx bits = " << txByteCounter * 8 << std:: endl;
  std::cout << "Total Tx packets = " << txPktCounter << std:: endl;

  std::cout << "Total Rx bits = " << rxByteCounter * 8 << std:: endl;
  std::cout << "Total Rx packets = " << rxPktCounter << std:: endl;

  std::cout << "Avrg thput = " << (rxByteCounter * 8) / (finalSimTime - Seconds(realAppStart)).GetSeconds () / 1000.0 << " kbps" << std:: endl;

  std::cout << "Average Packet Inter-Reception (PIR) " << pir.GetSeconds () / pirCounter << " sec" << std::endl;

  /*
   * VERY IMPORTANT: Do not forget to empty the database cache, which would
   * dump the data store towards the end of the simulation in to a database.
   */
  pktStats.EmptyCache ();
  pscchStats.EmptyCache ();
  psschStats.EmptyCache ();
  pscchPhyStats.EmptyCache ();
  psschPhyStats.EmptyCache ();
  v2xSchedulingxApp.EmptyCache ();
  v2xSchedulingMode2.EmptyCache();

  mobilityStats.EmptyCache();

  Simulator::Destroy ();
  return 0;
}



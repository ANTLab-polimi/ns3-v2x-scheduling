/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */


#include <vector>
#include <iostream>
#include <numeric>
#include <ns3/nr-gnb-phy.h>
#include <ns3/core-module.h>
#include <ns3/nr-module.h>
#include "sl-stats-helper.h"
// #include <ns3/nr-multi-model-spectrum-channel.h>


namespace ns3 {

//class SinrOutputStats;
//class PowerOutputStats;
//class SlotOutputStats;
//class RbOutputStats;

/* ... */
NS_LOG_COMPONENT_DEFINE("SlStatsHelper");
NS_OBJECT_ENSURE_REGISTERED (SlStatsHelper);

static bool g_rxRxRlcPDUCallbackCalled = false;
static bool g_rxPdcpCallbackCalled = false;

// used at the end of simulation to see the packet arrival time
static std::vector<uint64_t> packetsTime;

// the map of addresses
ContainerAddresses addressesMap;

static const uint32_t totalTxBytes = 45000000; //  45000000; // 000
static uint32_t currentTxBytes = 0;
// Perform series of 1040 byte writes (this is a multiple of 26 since
// we want to detect data splicing in the output stream)
static const uint32_t writeSize = 1024; //  1024;
uint8_t data[writeSize];

int printIteration = 0;

//std::string dir = "results/";
std::string dir = "/home/ugjeci/Desktop/bbr-results/";

std::string statsCheck = "/home/ugjeci/Desktop/bbr-results/StatsCheck/";

uint32_t segmentSize = 524;


SlStatsHelper::SlStatsHelper(void){
	// initializing the parameters

	 // here we empty the log file
	std::string _occupancyFileName = statsCheck + "occupancy.txt";

	std::ofstream _occupancyFile;
	_occupancyFile.open(_occupancyFileName, std::ofstream::out | std::ofstream::trunc);
	_occupancyFile.close();

	std::string _flowFileName = statsCheck + "flowFile.txt";
	std::ofstream _flowFile;
	_flowFile.open(_flowFileName, std::ofstream::out | std::ofstream::trunc);
	_flowFile.close();


	std::string _StatsCheckDir = statsCheck;
//	std::string _StatsCheckDir = "/home/ugjeci/Desktop/bbr-results/StatsCheck/";
	std::ofstream fSlotStat1;
	fSlotStat1.open (_StatsCheckDir+ "file1.txt", std::ofstream::out | std::ofstream::trunc);
	fSlotStat1.close();
	std::ofstream fSlotStat2;
	fSlotStat2.open (_StatsCheckDir+ "file2.txt", std::ofstream::out | std::ofstream::trunc);
	fSlotStat1.close();
}

SlStatsHelper::~SlStatsHelper (void)
{
  NS_LOG_FUNCTION (this);
}

TypeId
SlStatsHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SlStatsHelper")
    .SetParent<Object> ()
    .SetGroupName ("ElephantFlow")
	.AddConstructor<SlStatsHelper> ()
	.AddAttribute ("ResultsStoringPath",
		                   "The path where to store the results"
							"Default value is the local directory",
						   StringValue ("./"),
						   MakeStringAccessor (&SlStatsHelper::SetResultsDirectory),
						   MakeStringChecker ())
	.AddAttribute ("CongestionWindowDirectory",
	                   "The directory name where to store the congestion window traces",
					   StringValue ("cwndTraces/"),
					   MakeStringAccessor (&SlStatsHelper::SetCongestionsWindowDirectory),
					   MakeStringChecker ())
   .AddAttribute ("PcapDirectory",
					   "The directory name where to store the pcap traces",
					   StringValue ("pcap/"),
					   MakeStringAccessor (&SlStatsHelper::SetPcapDirectory),
					   MakeStringChecker ())
   .AddAttribute ("StatsCheckDir",
					   "The directory name where to store the stats check traces",
					   StringValue ("StatsCheck/"),
					   MakeStringAccessor (&SlStatsHelper::SetStatsCheckDirectory),
					   MakeStringChecker ())
//					   .AddTraceSource(name, help, accessor, callback, supportLevel, supportMsg)
					   ;
  return tid;
}

void
SlStatsHelper::SetResultsDirectory(std::string value){
	NS_LOG_FUNCTION (this << value);
//	NS_LOG_UNCOND("Setting result dir");
	// creating the directory if it doesn't exist
	m_dirPath = value;
	createDir(m_dirPath);
}

void
SlStatsHelper::SetCongestionsWindowDirectory (std::string value)
{
  NS_LOG_FUNCTION (this << value);
//  NS_LOG_UNCOND("Congestion wnd");
  m_cwndDir = value;
}

void
SlStatsHelper::SetPcapDirectory (std::string value)
{
	NS_LOG_FUNCTION (this << value);
	m_pcapDir = value;
	// creating directory if it doesn't exist
	createDir(m_dirPath + "/" +  m_pcapDir);
}

void SlStatsHelper::SetStatsCheckDirectory(std::string value){
	NS_LOG_FUNCTION (this << value);
	m_statsCheck = value;
	// creating directory if it doesn't exist
	createDir(m_dirPath +  m_statsCheck);
}

void SlStatsHelper::createDir(std::string dirPath){
	std::string dirToSave = "mkdir -p " + dirPath;
	if (system (dirToSave.c_str ()) == -1){
		NS_LOG_ERROR("Cannot create dir " << dirPath);
	   exit (1);
	}
//	NS_LOG_UNCOND("Directory created...");
}

// Function to trace change in cwnd at n0
void SlStatsHelper::CwndChange (uint32_t oldCwnd, uint32_t newCwnd){
//	std::cout << std::filesystem::current_path() << std::endl;
	std::ofstream fPlotQueue (dir+ "cwndTraces/n0.dat", std::ios::out | std::ios::app); //dir  this->dirPath
	fPlotQueue << Simulator::Now ().GetSeconds () << " " << oldCwnd << " " << newCwnd << std::endl;
	fPlotQueue.close ();
}


void
SlStatsHelper::CourseChange (SlMobilityStats *stats, Ptr<const MobilityModel> mobility)
{
  Vector pos = mobility->GetPosition (); // Get position
  Vector vel = mobility->GetVelocity (); // Get velocity

  stats->SaveSlMobilityStats(pos, vel);
}

void
SlStatsHelper::ReportSinrNr (SlSinrOutputStats *stats, uint16_t cellId, uint16_t rnti,
                           double power, double avgSinr, uint16_t bwpId)
{
//	NS_LOG_UNCOND("saving sinr" << cellId);
  stats->SaveSinr (cellId, rnti, power, avgSinr, bwpId);
}

void
SlStatsHelper::ReportPowerNr (SlPowerOutputStats *stats, const SfnSf & sfnSf,
                            Ptr<const SpectrumValue> txPsd, const Time &t, uint16_t rnti, uint64_t imsi,
                            uint16_t bwpId, uint16_t cellId)
{
  stats->SavePower (sfnSf, txPsd, t, rnti, imsi, bwpId, cellId);
}

void
SlStatsHelper::ReportSlotStatsNr (SlSlotOutputStats *stats, const SfnSf &sfnSf, uint32_t scheduledUe,
                                uint32_t usedReg, uint32_t usedSym,
                                uint32_t availableRb, uint32_t availableSym, uint16_t bwpId,
                                uint16_t cellId)
{
  stats->SaveSlotStats (sfnSf, scheduledUe, usedReg, usedSym,
                        availableRb, availableSym, bwpId, cellId);
}

void 
SlStatsHelper::ReportSlPhySlotStats(SlPhySlotStats *phySlotStats, const SfnSf &sfnSf,
						std::deque<ns3::VarTtiAllocInfo> queue, uint32_t availRb, 
						uint32_t NumRbPerRbg, uint16_t bwpId, uint16_t cellId){

	struct allocDataStruct
	{
		uint16_t rnti = 0;
		uint32_t rntiDataReg = 0;
		uint32_t rntiCtrlReg = 0;
		uint32_t rntiDataSym = 0;
		uint32_t rntiCtrlSym = 0;
		allocDataStruct (uint16_t _rnti, uint32_t _rntiDataReg, uint32_t _rntiDataSym, 
				uint32_t _rntiCtrlReg, uint32_t _rntiCtrlSym): 
		rnti(_rnti), rntiDataReg(_rntiDataReg), rntiCtrlReg(_rntiCtrlReg),
		rntiDataSym(_rntiDataSym), rntiCtrlSym(_rntiCtrlSym){}
	};

	std::vector<allocDataStruct> allocData;
	
	// statistics per rnti
	uint32_t rntiDataReg = 0;
	uint32_t rntiCtrlReg = 0;
	uint32_t rntiDataSym = 0;
	uint32_t rntiCtrlSym = 0;
	int lastSymStart = -1;
  	uint32_t symUsed = 0;

	for (const auto & allocation : queue)
    {
      // modified 
      rntiDataReg = 0;
      rntiCtrlReg = 0;
      rntiDataSym = 0;
      rntiCtrlSym = 0;
      // end modification
      uint32_t rbg = std::count (allocation.m_dci->m_rbgBitmask.begin (),
                                 allocation.m_dci->m_rbgBitmask.end (), 1);

      auto rbgUsed = (rbg * NumRbPerRbg) * allocation.m_dci->m_numSym;
      if (allocation.m_dci->m_type == DciInfoElementTdma::DATA)
        {
          rntiDataReg += rbgUsed;
        }
      else
        {
          rntiCtrlReg += rbgUsed;
        }

      if (lastSymStart != allocation.m_dci->m_symStart)
        {
          symUsed += allocation.m_dci->m_numSym;

          if (allocation.m_dci->m_type == DciInfoElementTdma::DATA)
            {
              rntiDataSym += allocation.m_dci->m_numSym;
            }
          else
            {
              rntiCtrlSym += allocation.m_dci->m_numSym;
            }
        }

      	lastSymStart = allocation.m_dci->m_symStart;

	  	// find rnti in the vector
		if(allocation.m_dci->m_rnti!=0){
			std::vector< allocDataStruct >::iterator it = find_if(allocData.begin (), 
			allocData.end (), [=] (allocDataStruct const& f) { 
				return (f.rnti == allocation.m_dci->m_rnti); 
			});

			if(it!=allocData.end()){
				it->rntiCtrlReg += rntiCtrlReg;
				it->rntiCtrlSym += rntiCtrlSym;
				it->rntiDataReg += rntiDataReg;
				it->rntiDataSym += rntiDataSym;
			}else{
				allocDataStruct _tmp = allocDataStruct(allocation.m_dci->m_rnti,
				rntiDataReg, rntiDataSym, rntiCtrlReg,  rntiCtrlSym);
				allocData.push_back(_tmp);
			}
		}
    }

	// add detailed allocation statistics
	for (const auto & allocDataElement : allocData){
		if(allocDataElement.rnti!=0){
			phySlotStats->SaveSlPhySlotStats(sfnSf, allocDataElement.rnti, 
			allocDataElement.rntiDataSym, allocDataElement.rntiDataReg,
			allocDataElement.rntiCtrlSym, allocDataElement.rntiCtrlReg, 
			availRb, bwpId, cellId);
		}
	}
}

void
SlStatsHelper::ReportSlotCtrlStatsNr (SlSlotCtrlOutputStats *stats, const SfnSf &sfnSf, uint32_t scheduledUe,
                                uint32_t usedReg, uint32_t usedSym,
                                uint32_t availableRb, uint32_t availableSym, uint16_t bwpId,
                                uint16_t cellId){
  stats->SaveSlotStats (sfnSf, scheduledUe, usedReg, usedSym,
                        availableRb, availableSym, bwpId, cellId);
}

void
SlStatsHelper::ReportRbStatsNr (SlRbOutputStats *stats, const SfnSf &sfnSf, uint8_t sym,
                              const std::vector<int> &rbUsed, uint16_t bwpId,
                              uint16_t cellId)
{
  stats->SaveRbStats (sfnSf, sym, rbUsed, bwpId, cellId);
}

void
SlStatsHelper::ReportGnbRxDataNr (SlPowerOutputStats *gnbRxDataStats, const SfnSf &sfnSf,
                                Ptr<const SpectrumValue> rxPsd, const Time &t, uint16_t bwpId,
                                uint16_t cellId)
{
  gnbRxDataStats->SavePower (sfnSf, rxPsd, t, 0, 0, bwpId, cellId);
}

void
SlStatsHelper::ReportFlowRatesNr (SlFlowMonitorOutputStats *flowRatesStats,
					const Ptr<FlowMonitor> &monitor, FlowMonitorHelper &flowmonHelper,
					const std::string &filename)
{
	flowRatesStats->Save(monitor, flowmonHelper, filename);
}

void
SlStatsHelper::ReportULTbSize (SlUplinkTbSizeStats *ulTbSizeStats, uint64_t imsi, uint64_t tbsize)
{
	ulTbSizeStats->SaveUlTbSizeStats(imsi, tbsize);
}
void
SlStatsHelper::ReportDLTbSize (SlDownlinkTbSizeStats *dlTbSizeStats, uint64_t imsi, uint64_t tbsize)
{
	dlTbSizeStats->SaveDlTbSizeStats(imsi, tbsize);
}

void
SlStatsHelper::ReportMacSR(SlMacSRStats *macSRStats, uint8_t bwdId, uint16_t rnti){
	macSRStats->SaveSlMacSRStats(bwdId, rnti);
}

void SlStatsHelper::ReportUeMacBsr(SlMacUeBsrStats *macUeBsrStats, uint16_t frame, uint8_t subframe, uint16_t slot, uint16_t cellId, uint16_t rnti, uint8_t lcid, uint8_t bsrid, 
 uint32_t bufferSize){
	macUeBsrStats->SaveSlMacUeBsrStats(frame, subframe, slot, cellId, rnti, lcid, bsrid, bufferSize);
}

void
SlStatsHelper::ReportMacBsr(SlMacBsrStats *macBsrStats, SfnSf sfnSf, uint16_t cellId, NrMacSchedSapProvider::SchedDlRlcBufferReqParameters schedParams){
	macBsrStats->SaveSlMacBsrStats(sfnSf, cellId, schedParams.m_rnti, schedParams.m_rlcTransmissionQueueSize, 
	schedParams.m_rlcTransmissionQueueHolDelay, schedParams.m_rlcStatusPduSize, schedParams.m_rlcRetransmissionQueueSize,
	schedParams.m_rlcRetransmissionHolDelay, schedParams.m_logicalChannelIdentity);
}

void 
SlStatsHelper::ReportCtrlUlDci(SlCtrlUlDciStats *ctrlUlDciStats, uint16_t cellId, const std::shared_ptr<DciInfoElementTdma> &dci,const SfnSf &sfnsf){

	// std::vector<uint32_t> rbgBitmask;
	// std::transform(dci->m_rbgBitmask.begin(), dci->m_rbgBitmask.end(),
    //            std::back_insert_iterator<std::vector<uint32_t>>(rbgBitmask),
    //            [](auto ptr) { return static_cast<uint32_t>(ptr); });
	// std::string rbgBitmaskString(rbgBitmask.begin(), rbgBitmask.end());

	std::ostringstream oss;
	if (!dci->m_rbgBitmask.empty()){
		std::vector<uint32_t> rbgInt32Vec(dci->m_rbgBitmask.begin(), dci->m_rbgBitmask.end());
		// Convert all but the last element to avoid a trailing ","
		std::copy(rbgInt32Vec.begin(), rbgInt32Vec.end()-1,
			std::ostream_iterator<uint32_t>(oss, ","));
		// Now add the last element with no delimiter
		oss << rbgInt32Vec.back();
	}
	std::string rbgBitmaskString = oss.str();
	
	std::string  dciFormatString;
	std::string ttiTypeString;

	if(dci->m_format == DciInfoElementTdma::DciFormat::DL){
		dciFormatString = "DL";
	}else{
		dciFormatString = "UL";
	}

	if(dci->m_type == DciInfoElementTdma::VarTtiType::SRS){
		ttiTypeString = "SRS";
	}else if(dci->m_type == DciInfoElementTdma::VarTtiType::DATA){
		ttiTypeString = "DATA";
	}else{
		ttiTypeString = "CTRL";
	}


	ctrlUlDciStats->SaveSlCtrlUlDciStats(cellId, dci->m_rnti, sfnsf, 
	dci->m_bwpIndex, dci->m_symStart, dci->m_numSym, dci->m_mcs, dci->m_harqProcess, rbgBitmaskString,
	dciFormatString, ttiTypeString, dci->m_ndi, dci->m_rv, dci->m_tpc);
	
}

void
SlStatsHelper::ReportPacketTrace(SlPacketTraceStats *packetTraceStats, std::string direction, RxPacketTraceParams packetTrace){
	packetTraceStats->SavePacketTrace(direction, packetTrace);
}

void
SlStatsHelper::ReportUeReceivedPss(SlUePssReceivedStats *uePssReceivedStats, const SfnSf &sfnsf, std::vector<uint16_t> userIds, uint64_t imsi, uint16_t generatingCellId, double power, uint16_t nRb, double power_rsrq, uint16_t nRb_rsrq){
	uint16_t cellId = userIds[0];
	uint16_t bwpId = userIds[1];
	uint16_t rnti = userIds[2];
	uePssReceivedStats->SavePss(sfnsf, cellId, bwpId, rnti, imsi, generatingCellId, power, nRb, power_rsrq, nRb_rsrq);
}

void SlStatsHelper::ReportCtrlMessages(SlCtrlMsgsStats *ctrlMsgsStats, std::string layer, std::string entity, SfnSf sfn, 
											uint16_t nodeId, uint16_t rnti,
                                              uint8_t bwpId, Ptr<const NrControlMessage> msg){
	
	std::string msgTypeString;
	if (msg->GetMessageType () == NrControlMessage::DL_CQI){
      msgTypeString = "DL_CQI";
    }
	else if (msg->GetMessageType () == NrControlMessage::SR){
		msgTypeString = "SR";
	}
	else if (msg->GetMessageType () == NrControlMessage::BSR){
		msgTypeString = "BSR";
	}
	else if (msg->GetMessageType () == NrControlMessage::RACH_PREAMBLE){
		msgTypeString = "RACH_PREAMBLE";
	}
	else if (msg->GetMessageType () == NrControlMessage::DL_HARQ){
		msgTypeString = "DL_HARQ";
	}
	else if (msg->GetMessageType () == NrControlMessage::MIB){
		msgTypeString = "MIB";
	}
	else if (msg->GetMessageType () == NrControlMessage::SIB1){
		msgTypeString = "SIB1";
	}
	else if (msg->GetMessageType () == NrControlMessage::RAR){
		msgTypeString = "RAR";
	}
	else if (msg->GetMessageType () == NrControlMessage::DL_DCI){
		msgTypeString = "DL_DCI";
	}
	else if (msg->GetMessageType () == NrControlMessage::UL_DCI){
		msgTypeString = "UL_DCI";
	}
	else{
		msgTypeString = "Other";
	}

	ctrlMsgsStats->SaveCtrlMsgStats(layer, entity, nodeId, rnti, sfn, bwpId, msgTypeString);
}

void
SlStatsHelper::RxPacketTraceUeCallback (Ptr<NrPhyRxTrace> phyStats, std::string path, RxPacketTraceParams params)
{
  std::cout << "\tframe\tsubF\tslot\t1stSym\tsymbol#\tcellId\trnti\ttbSize\tmcs\trv\tSINR(dB)\tcorrupt\tTBler\tbwpId" << std::endl;

  std::cout << "DL\t" << params.m_frameNum
                      << "\t" << (unsigned)params.m_subframeNum
                      << "\t" << (unsigned)params.m_slotNum
                      << "\t" << (unsigned)params.m_symStart
                      << "\t" << (unsigned)params.m_numSym
                      << "\t" << params.m_cellId
                      << "\t" << params.m_rnti
                      << "\t" << params.m_tbSize
                      << "\t" << (unsigned)params.m_mcs
                      << "\t" << (unsigned)params.m_rv
                      << "\t" << 10 * log10 (params.m_sinr)
                      << "\t" << params.m_corrupt
                      << "\t" << params.m_tbler
                      << "\t" << (unsigned)params.m_bwpId << std::endl;

  if (params.m_corrupt)
    {
      NS_LOG_DEBUG ("DL TB error\t" << params.m_frameNum
                                    << "\t" << (unsigned)params.m_subframeNum
                                    << "\t" << (unsigned)params.m_slotNum
                                    << "\t" << (unsigned)params.m_symStart
                                    << "\t" << (unsigned)params.m_numSym
                                    << "\t" << params.m_rnti
                                    << "\t" << params.m_tbSize <<
                    "\t" << (unsigned)params.m_mcs <<
                    "\t" << (unsigned)params.m_rv <<
                    "\t" << params.m_sinr <<
                    "\t" << params.m_tbler <<
                    "\t" << params.m_corrupt <<
                    "\t" << (unsigned)params.m_bwpId);
    }
}


void RxRlcPDUSl (std::string path, uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t rlcDelay){
  std::cout<<"\n\n Data received at RLC layer at:"<<Simulator::Now()<<std::endl;
  std::cout<<"\n rnti:"<<rnti<<std::endl;
  std::cout<<"\n lcid:"<<(unsigned)lcid<<std::endl;
  std::cout<<"\n bytes :"<< bytes<<std::endl;
  std::cout<<"\n delay :"<< rlcDelay<<std::endl;
  g_rxRxRlcPDUCallbackCalled = true;
}

void RxPdcpPDUSl (std::string path, uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t pdcpDelay)
{
  std::cout<<"\n Packet PDCP delay:"<<pdcpDelay<<"\n";
  g_rxPdcpCallbackCalled = true;
}

void ConnectPdcpRlcTracesSl (){
  Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/DataRadioBearerMap/1/LtePdcp/RxPDU",
                      MakeCallback (&RxPdcpPDUSl));
  Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/DataRadioBearerMap/1/LteRlc/RxPDU",
                      MakeCallback (&RxRlcPDUSl));
}

void ConnectUlPdcpRlcTracesSl ()
{
  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/UeMap/*/DataRadioBearerMap/*/LtePdcp/RxPDU",
                      MakeCallback (&RxPdcpPDUSl));

  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/UeMap/*/DataRadioBearerMap/*/LteRlc/RxPDU",
                      MakeCallback (&RxRlcPDUSl));
}

void ReportRlcPacketDrop(Ptr<OutputStreamWrapper> streamRlc, Ptr<const Packet> p){
	std::cout << "Packet size " << p->GetSize() << std::endl;
	*streamRlc->GetStream() << Simulator::Now ().GetSeconds () << " " << p->GetSize() << std::endl;

}

OperationBandInfo configureBandwidth(Ptr<NrHelper> &nrHelper, double centralFrequencyBand, double bandwidthBand){

	
	// best-> gives the highest throughput is UMi_StreetCanyon_LoS
	enum BandwidthPartInfo::Scenario scenarioEnum = BandwidthPartInfo::UMa;
	//UMa_LoS RMa_LoS UMi_StreetCanyon_LoS InH_OfficeOpen_LoS UMa
	// UMi_Buildings -> works only with building info

	/*
	* Spectrum configuration. We create a single operational band and configure the scenario.
	*/
	// BandwidthPartInfoPtrVector allBwps;
	CcBwpCreator ccBwpCreator;
	const uint8_t numCcPerBand = 1;  // in this example we have a single band, and that band is composed of a single component carrier

	/* Create the configuration for the CcBwpHelper. SimpleOperationBandConf creates
	 * a single BWP per CC and a single BWP in CC.
	 *
	 * Hence, the configured spectrum is:
	 *
	 * |---------------Band---------------|
	 * |---------------CC-----------------|
	 * |---------------BWP----------------|
	 */
	// Create the configuration for the CcBwpHelper
	CcBwpCreator::SimpleOperationBandConf bandConf (centralFrequencyBand, bandwidthBand,
			numCcPerBand, scenarioEnum);
	// FDD case
	// bandConf.m_numBwp = 2;

	// Configure scheduler; default is round robin
	// nrHelper->SetSchedulerTypeId(NrMacSchedulerOfdmaRR::GetTypeId());
	nrHelper->SetSchedulerTypeId (NrMacSchedulerTdmaRR::GetTypeId ());
	// nrHelper->SetSchedulerTypeId (NrMacSchedulerTdmaPF::GetTypeId ());
	// nrHelper->SetSchedulerTypeId(NrMacSchedulerOfdmaPF::GetTypeId());
	// nrHelper->SetSchedulerTypeId(NrMacSchedulerOfdmaMR::GetTypeId());

	// By using the configuration created, it is time to make the operation band
	OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);

	// Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod",TimeValue (MilliSeconds(0)));
	// nrHelper->SetSchedulerAttribute ("FixedMcsDl", BooleanValue (true));
	// nrHelper->SetSchedulerAttribute ("StartingMcsDl", UintegerValue(28));
	// nrHelper->SetChannelConditionModelAttribute ("UpdatePeriod", TimeValue (MilliSeconds (0)));
	// nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));

	//Initialize channel and pathloss, plus other things inside band.
	auto bandMask = NrHelper::INIT_PROPAGATION | NrHelper::INIT_CHANNEL | NrHelper::INIT_FADING;
  	// Omit fading from calibration mode

	// bandMask |= NrHelper::INIT_FADING;
    
	nrHelper->InitializeOperationBand (&band, bandMask); // , bandMask 
//	allBwps = CcBwpCreator::GetAllBwps ({band});

//	Packet::EnableChecking ();
//	Packet::EnablePrinting ();

	return band;
}

// Ptr<NrPointToPointEpcHelper> epcHelper
// void setDefaultGateway(NodeContainer nodes, Ptr<NrNewPointToPointEpcHelper> epcHelper, Ipv4StaticRoutingHelper ipv4RoutingHelper){
// 	for (uint32_t u = 0; u < nodes.GetN (); ++u){
// 		Ptr<Node> node = nodes.Get (u);
// 		// Set the default gateway for the UE
// 		Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (node->GetObject<Ipv4> ());
// 		ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
// 	}
// }

/*
* Set UE antenna attributes
 * */
void setUEAntennaAttributes(Ptr<NrHelper> nrHelper){
	nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (2)); // 1
	nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (2)); // 1
}

/*
 * Set UE antenna attributes
 * */
void setGNBAntennaAttributes(Ptr<NrHelper> nrHelper){
	nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (8));
	nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
}

void setGNBPhyAttributes(Ptr<NrHelper> nrHelper, Ptr<NetDevice> gnbNode, const uint16_t numerologyBwp, const double totalTxPower){
	
	Ptr<NrGnbPhy> gnbPhy = nrHelper->GetGnbPhy (gnbNode, 0);
	if(gnbPhy == nullptr){
		// in case gnbphy is nullptr then we do nothing
		return; 
	}
	nrHelper->GetGnbPhy (gnbNode, 0)->SetAttribute("Numerology", UintegerValue (numerologyBwp));
	nrHelper->GetGnbPhy (gnbNode, 0)->SetAttribute("TxPower", DoubleValue (totalTxPower));
}

void setUePhyAttributes(Ptr<NrHelper> nrHelper, Ptr<NetDevice> ueNode, const double totalTxPower, const double noiseFigure, const bool UlPowerControl){
	Ptr<NrUePhy> uePhy = nrHelper->GetUePhy (ueNode, 0);
	if(uePhy == nullptr){
		// in case gnbphy is nullptr then we do nothing
		return; 
	}
	nrHelper->GetUePhy(ueNode, 0)->SetAttribute("NoiseFigure", DoubleValue (noiseFigure));
	nrHelper->GetUePhy(ueNode, 0)->SetAttribute("EnableUplinkPowerControl", BooleanValue (UlPowerControl));
	nrHelper->GetUePhy(ueNode, 0)->SetAttribute("PowerAllocationType", EnumValue (NrSpectrumValueHelper::UNIFORM_POWER_ALLOCATION_BW)); // UNIFORM_POWER_ALLOCATION_BW UNIFORM_POWER_ALLOCATION_USED
	nrHelper->GetUePhy(ueNode, 0)->SetAttribute("TxPower", DoubleValue (totalTxPower));

	nrHelper->GetUePhy(ueNode, 0)->SetAttribute("TbDecodeLatency", TimeValue (MicroSeconds (0)));
}

void setGNBAmcAttributes(Ptr<NrHelper> nrHelper){
	// Both DL and UL AMC will have the same model behind.
	nrHelper->SetGnbDlAmcAttribute ("AmcModel", EnumValue (NrAmc::ShannonModel));
	nrHelper->SetGnbUlAmcAttribute ("AmcModel", EnumValue (NrAmc::ShannonModel));

	/*
	* Adjust the average number of Reference symbols per RB only for LTE case,
	* which is larger than in NR. We assume a value of 4 (could be 3 too).
	*/
	nrHelper->SetGnbDlAmcAttribute ("NumRefScPerRb", UintegerValue (1));
	nrHelper->SetGnbUlAmcAttribute ("NumRefScPerRb", UintegerValue (1));  //FIXME: Might change in LTE
}

Ptr<NrRadioEnvironmentMapHelper> SlStatsHelper::createRem(){
	// configure REM parameters
	  Ptr<NrRadioEnvironmentMapHelper> remHelper = CreateObject<NrRadioEnvironmentMapHelper> ();
	  remHelper->SetMinX (0);
	  remHelper->SetMaxX (30);
	  remHelper->SetResX (1);
	  remHelper->SetMinY (0);
	  remHelper->SetMaxY (30);
	  remHelper->SetResY (1);
	  remHelper->SetSimTag ("rem");
	  remHelper->SetRemMode (NrRadioEnvironmentMapHelper::BEAM_SHAPE);
	  return remHelper;
}

void SendPacket (Ptr<NetDevice> device, Address& addr, uint32_t packetSize)
{
	std::cout<< "Sending the packet from " << device->GetAddress() << " to " << addr << std::endl;
	Ptr<Packet> pkt = Create<Packet> (packetSize);
	Ipv4Header ipv4Header;
    // std::cout << "Port number " + UdpL4Protocol::PROT_NUMBER << std::endl;
	ipv4Header.SetProtocol(UdpL4Protocol::PROT_NUMBER);
	pkt->AddHeader(ipv4Header);
	EpsBearerTag tag (1, 1);
	pkt->AddPacketTag (tag);
	bool isSend = device->Send (pkt, addr, Ipv4L3Protocol::PROT_NUMBER);
	std::cout << "Send status: " << isSend << std::endl;

}

void printNestedMap(ContainerAddresses contAddrStruct){

	for(auto itr1 = contAddrStruct.begin(); itr1 != contAddrStruct.end(); itr1++){
		// Node type (UE, GNB, etc.)
		std::cout << itr1->first << ' '; // Add space to separate entries on the same line
		    // itr1->second represents std::map<int, std::vector<IpV4Address>> node id and addresses.
		    for(auto itr2 = itr1->second.begin (); itr2 != itr1->second.end (); itr2++){
		    	std::cout << itr2->first << ' ';
		        // itr2->second represents std::vector<IpV4Addresses> of node
		        for(auto itr3 = itr2->second.begin(); itr3 != itr2->second.end(); itr3++){
		        	std::cout << *itr3 << ' ';
				}
		    }
		    std::cout << std::endl;
	}
}

int getNodeIdByAddress(ContainerAddresses contAddrStruct, Ipv4Address nodeAddr){
	for(auto itr1 = contAddrStruct.begin(); itr1 != contAddrStruct.end(); itr1++){
		// Node type (UE, GNB, etc.)
		// itr1->second represents std::map<int, std::vector<IpV4Address>> node id and addresses.
		for(auto itr2 = itr1->second.begin (); itr2 != itr1->second.end (); itr2++){
			// itr2->second represents std::vector<IpV4Addresses> of node
			if(std::find(itr2->second.begin(), itr2->second.end(), nodeAddr)!= itr2->second.end()){
				// we found the element, thus we return the node id
				try{
					return (int) itr2->first;
				}catch(...){
					return -1;
				}
			}
		}
	}
	return -1;
}

void SlStatsHelper::BulkApplicationTxPacketSingle(Ptr<const Packet> p){
	std::cout << "Packet size " << p->GetSize() << std::endl;
}

void ReceivedPacket(Ptr<Socket> socket){
	std::cout << "Packet received at time " << Simulator::Now() << std::endl;
	std::cout << "Node ID " << socket->GetNode()->GetId() << std::endl;
}

void GenerateTraffic (Ptr<Socket> socket, uint32_t size)
{
  std::cout << "at=" << Simulator::Now ().GetSeconds () << "s, to be tx bytes=" << size << std::endl;
//  uint32_t left = totalTxBytes - currentTxBytes;
  uint32_t dataOffset = currentTxBytes % writeSize;
  uint32_t toWrite = writeSize - dataOffset;
  toWrite = std::min (toWrite, size);
  toWrite = std::min (toWrite, socket->GetTxAvailable ());
  std::cout<< "Socket available tx " << socket->GetTxAvailable () << std::endl;
  std::cout<< "To write " << toWrite << " Offset " << dataOffset << std::endl;
  int amountSent = socket->Send (&data[dataOffset], toWrite, 0);
  std::cout<< "Amount of data sent " << amountSent << std::endl;
  if ((size > 0) & (amountSent>0))
    {
	  std::cout<< "Data remaining " << size - toWrite << std::endl;
      Simulator::Schedule (MilliSeconds(0) , &GenerateTraffic, socket, size - toWrite);
    }
  else
    {
	  std::cout << "Socket error " << socket->GetErrno() << std::endl;
      socket->Close ();
      return;
    }
  currentTxBytes += amountSent;
}

void StartSinglePacketFlow (Ptr<Socket> localSocket, Ipv4Address servAddress, uint16_t servPort, const uint32_t elephantFlowSize){
	int connectStatus = localSocket->Connect (InetSocketAddress (servAddress, servPort)); //connect
	int amountSent = localSocket->Send (&data[0], 1000, 0);
	std::cout << "Connect status " << connectStatus << std::endl;
	std::cout<< "Amount of data sent " << amountSent << std::endl;
}


void initializeTxBuffer(){
// initialize the tx buffer.
   for(uint32_t i = 0; i < writeSize; ++i){
	   char m = toascii (97 + i % 26);
	   data[i] = m;
	}
}

std::vector<uint8_t> initTxData(std::vector<uint8_t> data){
// initialize the tx buffer.
   for(uint32_t i = 0; i < writeSize; ++i){
	   char m = toascii (97 + i % 26);
	   data[i] = m;
	}
   return data;
}

void createFileHeader(std::string fileName){
	std::ofstream outFile;
	// open a new file or overwrite to existing one
	outFile.open (fileName.c_str (), std::ofstream::out | std::ofstream::trunc);
	// write the header
	std::string divider = ",";
	outFile<< "Iteration" << divider << "Timestamp" << divider << "Protocol" << divider
			<< "Flow number" << divider;
	outFile <<"Source Address" << divider << "Source Port" << divider
			<< "Source Node Type" << divider << "Source  Node Id" << divider;
	outFile << "Destination Address" << divider << "Destination Port" << divider
			<< "Destination Node Type" << divider << " Destination Node Id" << divider;
	outFile << "Tx Packets" << divider << "Tx bytes" << divider
			<< "Rx packets" << divider << "Rx bytes" << divider;
	outFile << "Throughput Mbps Rx" << divider << "Mean delay ms Rx" << divider
			<< "Mean jitter Rx" << divider;
	outFile << "Throughput Mbps Tx" << divider << "Mean delay ms Tx" << divider
			<< "Mean jitter Tx" << divider;
	outFile << "\n";

	outFile.close();
}

void installServerSink(Ptr<Node> node, Ipv4Address ipv4, uint16_t port, std::string socketFactory, ApplicationContainer apps){
//	Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
	Address sinkLocalAddress (InetSocketAddress (ipv4, port));
	PacketSinkHelper sinkHelper (socketFactory, sinkLocalAddress);
	// Take only the first tcp node and use it as a sink
	apps.Add (sinkHelper.Install (node));
}

// Function to install sink application
void installPacketSink (Ptr<Node> node, uint16_t port, std::string socketFactory, ApplicationContainer apps){
	Address packetSinkAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
	PacketSinkHelper sink (socketFactory, packetSinkAddress);
	apps.Add(sink.Install (node));

}


/*
 * connecting the traces to the print statement
 * */
void connectTracesToPrintStatement(bool isUl){
	// connecting the traces to the print statement
	if (isUl)
	{
		std::cout<<"\n Sending data in uplink."<<std::endl;
		Simulator::Schedule(Seconds(0.5), &ConnectUlPdcpRlcTracesSl);
	}
	else
	{
		std::cout<<"\n Sending data in downlink."<<std::endl;
		Simulator::Schedule(Seconds(0.5), &ConnectPdcpRlcTracesSl);
	}
}

void CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd){
//	std::cout << std::filesystem::current_path() << std::endl;
//	NS_LOG_UNCOND("Writing to the cwnd trace file");
//	std::ofstream fPlotQueue ( dir + "cwndTraces/n0.dat", std::ios::out | std::ios::app); //dir  this->dirPath
	*stream->GetStream() << Simulator::Now ().GetSeconds () << " " << oldCwnd << " " << newCwnd << std::endl;
//	fPlotQueue << Simulator::Now ().GetSeconds () << " " << oldCwnd << " " << newCwnd << std::endl;
//	fPlotQueue.close ();
}


 PointToPointHelper createP2PHElper(std::string dataRate, uint64_t delayMicroseconds){
	 // connect a remoteHost to pgw. Setup routing too
	PointToPointHelper p2ph;
	p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
//	p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("2 Gb/s"))); // gbps "2Gb/s" 2000000
//	p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1550));
//	p2ph.SetChannelAttribute ("Delay", TimeValue (MicroSeconds (delayMicroseconds)));
	return p2ph;
}

/*
 * Configure the TCP traffic between clients and server TCP nodes
 * */
void configureTCPTraffic(NodeContainer clientTcpNodes, ApplicationContainer clientApps,
	NodeContainer serverTcpNodes, Ipv4InterfaceContainer  serverTcpIps, uint16_t tcpPort){


	std::cout << "Configure TCP traffic" << std::endl;
	for (uint32_t i = 0; i < clientTcpNodes.GetN (); ++i){
	  for (uint32_t j = 0; j < serverTcpNodes.GetN(); ++j){
		  	AddressValue remoteAddress (InetSocketAddress (serverTcpIps.GetAddress(j), tcpPort));
		  	Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (100)); // arbitrarily
		  //	Address bulkLocalAddress (InetSocketAddress (clientTcpIps.GetAddress(0), otherPort));
		  	BulkSendHelper ftp ("ns3::TcpSocketFactory",  Address ()); // Address () // bulkLocalAddress
		  	ftp.SetAttribute ("Remote", AddressValue(remoteAddress));
		  	ftp.SetAttribute ("SendSize", UintegerValue (100));// arbitrarily
		  	ftp.SetAttribute ("MaxBytes", UintegerValue (2000000));
		  //		serverApps.Add(ftp.Install(serverTcpNodes.Get(0)));
		  	clientApps.Add (ftp.Install (clientTcpNodes.Get(i)));
		}
	}
 }

/*
 * Configure UDP traffic between server and client nodes
 * */
void configureUDPTraffic(NodeContainer clientNodes, ApplicationContainer* clientApps,
		NodeContainer serverNodes, Ipv4InterfaceContainer  serverIps, uint16_t udpPort){
	std::cout << "Configure the UDP traffic" << std::endl;
	for (uint32_t i = 0; i < clientNodes.GetN (); ++i)
	{
	  for (uint32_t j = 0; j < serverNodes.GetN(); ++j)
		{
		  UdpClientHelper dlClient (serverIps.GetAddress (j), udpPort);
		  dlClient.SetAttribute ("MaxPackets", UintegerValue(1000000));
		  dlClient.SetAttribute ("PacketSize", UintegerValue(1000));
		  dlClient.SetAttribute ("Interval", TimeValue (MicroSeconds(100)));
		  clientApps->Add (dlClient.Install (clientNodes.Get(i)));
		}
	}
}

void configureSingleUDPTraffic(NodeContainer clientNodes, ApplicationContainer* clientApps,
		NodeContainer serverNodes, Ipv4InterfaceContainer  serverIps, uint16_t udpPort){
	for (uint32_t j = 0; j < serverNodes.GetN(); ++j)
	{
	  UdpClientHelper dlClient (serverIps.GetAddress (j), udpPort);
	  dlClient.SetAttribute ("MaxPackets", UintegerValue(1000));
	  dlClient.SetAttribute ("PacketSize", UintegerValue(1500));
	  dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds(1)));
	  clientApps->Add (dlClient.Install (clientNodes.Get(0)));
	}
}

std::string getStrFromInt(int number){
	std::ostringstream s;
	s << number;
	if (number < 10){
		return std::string("0") + std::string(s.str());
	}else{ 
		return std::string(s.str());
	}
}

// sl

void
SlStatsHelper::PrintGnuplottableUeListToFile(std::string filename)
{
    std::ofstream outFile;
    outFile.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
    if (!outFile.is_open())
    {
        NS_LOG_ERROR("Can't open file " << filename);
        return;
    }
    for (NodeList::Iterator it = NodeList::Begin(); it != NodeList::End(); ++it)
    {
        Ptr<Node> node = *it;
        int nDevs = node->GetNDevices();
        for (int j = 0; j < nDevs; j++)
        {
            Ptr<LteUeNetDevice> uedev = node->GetDevice(j)->GetObject<LteUeNetDevice>();
            Ptr<NrUeNetDevice> mmuedev = node->GetDevice(j)->GetObject<NrUeNetDevice>();
            if (uedev)
            {
                Vector pos = node->GetObject<MobilityModel>()->GetPosition();
                outFile << "set label \"" << uedev->GetImsi() << "\" at " << pos.x << "," << pos.y
                        << " left font \"Helvetica,8\" textcolor rgb \"black\" front point pt 1 ps "
                           "0.3 lc rgb \"black\" offset 0,0"
                        << std::endl;
            }
            else if (mmuedev)
            {
                Vector pos = node->GetObject<MobilityModel>()->GetPosition();
                outFile << "set label \"" << mmuedev->GetImsi() << "\" at " << pos.x << "," << pos.y
                        << " left font \"Helvetica,8\" textcolor rgb \"black\" front point pt 1 ps "
                           "0.3 lc rgb \"black\" offset 0,0"
                        << std::endl;
            }
        }
    }
}



void 
SlStatsHelper::NotifyxAppScheduling(UeV2XSchedulingXApp* v2xSchedStats, Ptr<NrGnbPhy> gnbPhyPtr, uint64_t ueId, ns3::NrSlGrantInfo nrSlGrantInfo, std::string plmnId){
  v2xSchedStats->Save(ueId, nrSlGrantInfo, plmnId, gnbPhyPtr->GetCurrentSfnSf());
}

void 
SlStatsHelper::NotifySlPscchScheduling (UeMacPscchTxOutputStats *pscchStats, const SlPscchUeMacStatParameters pscchStatsParams)
{
  pscchStats->Save (pscchStatsParams);
}

void 
SlStatsHelper::NotifySlPsschScheduling (UeMacPsschTxOutputStats *psschStats, const SlPsschUeMacStatParameters psschStatsParams)
{
  psschStats->Save (psschStatsParams);
}

void 
SlStatsHelper::NotifySlPscchRx (UePhyPscchRxOutputStats *pscchStats, const SlRxCtrlPacketTraceParams pscchStatsParams)
{
  pscchStats->Save (pscchStatsParams);
}

void 
SlStatsHelper::NotifySlPsschRx (UePhyPsschRxOutputStats *psschStats, const SlRxDataPacketTraceParams psschStatsParams)
{
  psschStats->Save (psschStatsParams);
}


void
SlStatsHelper::UePacketTraceDb (UeToUePktTxRxOutputStats *stats, Ptr<Node> node, const Address &localAddrs,
                 std::string txRx, Ptr<const Packet> p, const Address &srcAddrs,
                 const Address &dstAddrs, const SeqTsSizeHeader &seqTsSizeHeader)
{
  uint32_t nodeId = node->GetId ();
  uint64_t imsi = node->GetDevice (0)->GetObject<NrUeNetDevice> ()->GetImsi ();
  uint32_t seq = seqTsSizeHeader.GetSeq ();
  uint32_t pktSize = p->GetSize () + seqTsSizeHeader.GetSerializedSize ();

  stats->Save (txRx, localAddrs, nodeId, imsi, pktSize, srcAddrs, dstAddrs, seq);
}

}


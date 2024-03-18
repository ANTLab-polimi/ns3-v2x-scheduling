/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "parameters-config.h"
#include <sys/stat.h>
#include <ns3/lte-enb-net-device.h>


namespace ns3 {

// namespace mmwave
// {

NS_LOG_COMPONENT_DEFINE ("ParametersConfig");

NS_OBJECT_ENSURE_REGISTERED (ParametersConfig);

ParametersConfig::ParametersConfig (void){
	NS_LOG_FUNCTION (this);
}


ParametersConfig::~ParametersConfig ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
ParametersConfig::GetTypeId (void)
{
	// we can add attributes as well
  static TypeId tid = TypeId ("ns3::ParametersConfig")
    .SetParent<Object> ()
    .SetGroupName ("ParametersConfig")
    .AddConstructor<ParametersConfig> ()
	;
  return tid;
}

bool
Params::Validate (void) const
{
	NS_ABORT_MSG_IF (simulator != "5GLENA",
	                   "Cannot do the parameters config with the simulator " << simulator);
	return true;
}

void 
ParametersConfig::DisableTraces()
{
	LogLevel logLevel = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_INFO | LOG_DEBUG | LOG_LEVEL_ALL);
	LogComponentDisableAll(logLevel);
}

void 
ParametersConfig::EnableTraces()
{
	LogLevel logLevel = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_INFO | LOG_DEBUG | LOG_LEVEL_ALL | LOG_LEVEL_FUNCTION); //
	// LogComponentEnable ("EpcX2", LOG_LEVEL_ALL);
	// LogComponentEnable("LteEnbRrc", LOG_LEVEL_ALL);
	// LogComponentEnable("LteUeRrc", logLevel);

	// LogComponentEnableAll (logLevel);
//   LogComponentEnable ("RicControlMessage", logLevel);
//   LogComponentEnable ("MmWaveFlexTtiMaxWeightMacScheduler", LOG_LEVEL_ALL);
// 	LogComponentEnable ("MmWaveFlexTtiMaxRateMacScheduler", LOG_LEVEL_ALL);
  
  LogComponentEnable ("E2Termination", logLevel);

//   LogComponentEnable ("LteEnbNetDevice", logLevel);
//   LogComponentEnable ("CttcNrV2xTest", logLevel);
//   LogComponentEnable ("MmWaveEnbPhy", logLevel);
// LogComponentEnable("LteEnbRrc", logLevel);
//   LogComponentEnable ("VideoStreamServerHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("NrHelper", logLevel);
//   LogComponentEnable ("MultiModelSpectrumChannel", LOG_LEVEL_ALL);
//   LogComponentEnable ("NrGnbNetDevice", logLevel);
  LogComponentEnable ("NrUeMac", logLevel);
//   LogComponentEnable ("MmWaveSpectrumPhy", logLevel);

	// LogComponentEnable ("MmWaveUePhy", LOG_LEVEL_ALL);
	// LogComponentEnable ("HexagonalGridScenarioHelper", logLevel);
	// LogComponentEnable ("ElasticFlow", logLevel);

	
	// LogComponentEnable ("MmWaveEnbMac", logLevel);
	// LogComponentEnable ("ThreeGppSpectrumPropagationLossModel", logLevel);

	// LogComponentEnable ("VideoStreamClientApplication", logLevel);
	// LogComponentEnable("PacketSink", logLevel);
	// LogComponentEnable("OnOffApplication", logLevel);
	// LogComponentEnable ("Socket", logLevel);

	// LogComponentEnable ("MmWaveAmc", logLevel);
	// LogComponentEnable ("VideoStreamServerApplication", logLevel);
	// LogComponentEnable ("IdealBeamformingAlgorithm", logLevel);
	// LogComponentEnable ("PhasedArrayModel", logLevel);
	// LogComponentEnable ("PPBPApplication", logLevel);

	// LogComponentEnable ("LteRlcUm", logLevel);
	// LogComponentEnable ("LteRlcAm", logLevel);
	// LogComponentEnable ("LteRlcTm", logLevel);
	// LogComponentEnable ("LteRlcUmLowLat", logLevel);
	
	// LogComponentEnable("LtePdcp", logLevel);
	// LogComponentEnable ("BulkSendApplication", logLevel);

	// LogComponentEnable ("LteInterference", logLevel);
	// LogComponentEnable ("LteChunkProcessor", logLevel);

	// LogComponentEnable ("LteRlc", logLevel);
	// LogComponentEnable("MmWaveBearerStatsConnector", logLevel);
	// LogComponentEnable ("ComponentCarrierUe", logLevel);
	// LogComponentEnable ("SimpleUeComponentCarrierManager", logLevel);

	// LogComponentEnable ("PointToPointNetDevice", logLevel);
	
}

void
ParametersConfig::CreateTracesDir(Params params)
{
	struct stat outputDirInfo;

	if (stat(params.outputDir.c_str(), &outputDirInfo) != 0)
	{
		std::cout << "Cannot access directory. Exiting... " << std::endl;
		exit(1);
	}
	else if (outputDirInfo.st_mode & S_IFDIR)
	{
		// If the main directory exists, we create the other ones if they do not exist
		// std::cout<< "Output dir is valid " << std::endl;
		struct stat simTagDirInfo;
		if (stat((params.outputDir + params.simTag + "/").c_str(), &simTagDirInfo) != 0)
		{
			// std::cout << "Directory does not exists. Creating... " << std::endl;
			const int dir_err = mkdir((params.outputDir + params.simTag).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			if (dir_err == -1)
			{
				// std::cout << "Could not create directory. Exiting... " << std::endl;
				exit(1);
			}
			else
			{
				// check for the pcap directory
				// if this directory exist we go and check the pcap directory if it exists. if not we create it
				struct stat pcapDirInfo;
				if (stat((params.outputDir + params.simTag + "/pcap").c_str(), &pcapDirInfo) != 0)
				{
					// std::cout<<"Directory does not exists. Creating... " << std::endl;
					// Try to create also the pcap directory
					const int pcap_dir_err = mkdir((params.outputDir + params.simTag + "/pcap").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
					if (pcap_dir_err == -1)
					{
						// std::cout << "Could not create directory. Exiting... " << std::endl;
						exit(1);
					}
				}
			}
		}
		else if (simTagDirInfo.st_mode & S_IFDIR)
		{
			// if this directory exist we go and check the pcap directory if it exists. if not we create it
			struct stat pcapDirInfo;
			// std::cout<< "Simulation dir exists. Checking pcap dir" << std::endl;
			if (stat((params.outputDir + params.simTag + "/pcap/").c_str(), &pcapDirInfo) != 0)
			{
				// std::cout<<"Pcap directory does not exists. Creating... " << std::endl;
				// Try to create also the pcap directory
				const int pcap_dir_err = mkdir((params.outputDir + params.simTag + "/pcap").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
				if (pcap_dir_err == -1)
				{
					// std::cout << "Could not create directory. Exiting... " << std::endl;
					exit(1);
				}
			}
			else
			{
				// std::cout<<"Pcap directory exists" << std::endl;
			}
		}
		else
		{
			std::cout << "Cannot access directory. Exiting... " << std::endl;
			exit(1);
		}
	}
	else
	{
		std::cout << "Directory does not exists. Exiting... " << std::endl;
		exit(1);
	}
}

// }
}



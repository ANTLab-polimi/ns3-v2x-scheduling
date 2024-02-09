/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 TELEMATICS LAB, DEE - Politecnico di Bari
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
 * Author: Giuseppe Piro  <g.piro@poliba.it>
 * Author: Marco Miozzo <mmiozzo@cttc.es> : Update to FF API Architecture
 * Author: Nicola Baldo <nbaldo@cttc.es>  : Integrated with new RRC and MAC architecture
 * Author: Danilo Abrignani <danilo.abrignani@unibo.it> : Integrated with new architecture - GSoC 2015 - Carrier Aggregation
 */

#include <ns3/llc-snap-header.h>
#include <ns3/simulator.h>
#include <ns3/callback.h>
#include <ns3/node.h>
#include <ns3/packet.h>
#include <ns3/lte-net-device.h>
#include <ns3/packet-burst.h>
#include <ns3/uinteger.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/pointer.h>
#include <ns3/enum.h>
#include <ns3/lte-amc.h>
#include <ns3/lte-enb-mac.h>
#include <ns3/lte-enb-net-device.h>
#include <ns3/lte-enb-rrc.h>
#include <ns3/lte-ue-net-device.h>
#include <ns3/lte-enb-phy.h>
#include <ns3/ff-mac-scheduler.h>
#include <ns3/lte-handover-algorithm.h>
#include <ns3/lte-anr.h>
#include <ns3/lte-ffr-algorithm.h>
#include <ns3/ipv4-l3-protocol.h>
#include <ns3/ipv6-l3-protocol.h>
#include <ns3/abort.h>
#include <ns3/log.h>
#include <ns3/lte-enb-component-carrier-manager.h>
#include <ns3/object-map.h>
#include <ns3/object-factory.h>

#include <ns3/string.h>

#include "encode_e2apv1.hpp"
#include "e2ap_asn1c_codec.h"

// #include <ns3/control_message_encoder_decoder.h>
#include <control_message_encoder_decoder.h>

#include <ns3/lte-indication-message-helper.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteEnbNetDevice");

NS_OBJECT_ENSURE_REGISTERED ( LteEnbNetDevice);

template <typename A, typename B>
std::pair<B, A>
flip_pair (const std::pair<A, B> &p)
{
  return std::pair<B, A> (p.second, p.first);
}

template <typename A, typename B>
std::multimap<B, A>
flip_map (const std::map<A, B> &src)
{
  std::multimap<B, A> dst;
  std::transform (src.begin (), src.end (), std::inserter (dst, dst.begin ()), flip_pair<A, B>);
  return dst;
}


/**
* KPM Subscription Request callback.
* This function is triggered whenever a RIC Subscription Request for 
* the KPM RAN Function is received.
*
* \param pdu request message
*/
void 
LteEnbNetDevice::KpmSubscriptionCallback (E2AP_PDU_t* sub_req_pdu)
{
  NS_LOG_DEBUG ("\nReceived RIC Subscription Request, cellId = " << m_cellId << "\n");

  E2Termination::RicSubscriptionRequest_rval_s params = m_e2term->ProcessRicSubscriptionRequest (sub_req_pdu);
  NS_LOG_DEBUG ("requestorId " << +params.requestorId << 
                 ", instanceId " << +params.instanceId << 
                 ", ranFuncionId " << +params.ranFuncionId << 
                 ", actionId " << +params.actionId);  
  
  if (!m_isReportingEnabled && !m_forceE2FileLogging)
  {
    
    Simulator::ScheduleWithContext (1, MilliSeconds(10),
                                   &LteEnbNetDevice::BuildAndSendReportMessage, this, params);
    // Simulator::ScheduleWithContext (1, MilliSeconds(10),
    //                                &LteEnbNetDevice::BuildAndSendReportMessageTest, this, params);

    // Simulator::ScheduleWithContext (1, MilliSeconds(10),
    //                                &LteEnbNetDevice::BuildAndSendDetectionReportMessage, this, params);
    
    m_isReportingEnabled = true; 
  }
}

void 
LteEnbNetDevice::ControlMessageReceivedCallback (E2AP_PDU_t* ric_pdu)
{
  NS_LOG_FUNCTION(this);
  // here we receive a control message from the xapp, which we have to decode and actuate upon it
  Ptr<RicControlMessage> controlMessage = Create<RicControlMessage> (ric_pdu);
  // NS_LOG_INFO ("\n Message type received " << controlMessage->m_messageFormatType << "\n");
  NS_LOG_DEBUG ("\n Message received for plmn " << controlMessage->m_plmnString << "\n");
  // xer_fprint(stderr, &asn_DEF_E2AP_PDU, ric_pdu);
  if (controlMessage->m_messageFormatType == RicControlMessage::ControlMessage_PR_handoverMessage_Format){
    // checking if msg is intended for this simulation campaign/plmn
    std::string plmnId = controlMessage->m_plmnString;


    // if(plmnId.compare(m_ltePlmnId)==0){
    //   NS_LOG_DEBUG("Received control message for this simulation branch with plmnid " << plmnId);
    
    //   std::map<long, std::map<long, long>> resultMap = controlMessage->m_allHandoverList;
    //   NS_LOG_DEBUG("The map size " << resultMap.size());
      
    //   // the locig of executing the ric control message

    //   // for (std::map<long, std::map<long, long>>::iterator mapIt = resultMap.begin(); 
    //   //   mapIt!=resultMap.end(); ++mapIt){
    //   //   for(std::map<long,long>::iterator singleUser = mapIt->second.begin(); singleUser!=mapIt->second.end(); ++singleUser){
    //   //     uint64_t ueId = (uint64_t) singleUser->first;
    //   //     uint16_t destinationCellid = (uint16_t) singleUser->second;
    //   //     Simulator::ScheduleWithContext(1, Seconds(0), 
    //   //       &LteEnbNetDevice::AddToDoHandoverTrace, this, (uint16_t)mapIt->first, ueId, destinationCellid);
    //   //   }         
    //   // }
    // }
  }
}

Ptr<E2Termination>
LteEnbNetDevice::GetE2Termination() const
{
  return m_e2term;
}

void
LteEnbNetDevice::SetE2Termination(Ptr<E2Termination> e2term)
{
  m_e2term = e2term;

  NS_LOG_DEBUG("Register E2SM");

  if (!m_forceE2FileLogging)
  {
    Ptr<KpmFunctionDescription> kpmFd = Create<KpmFunctionDescription> ();
    e2term->RegisterKpmCallbackToE2Sm (200, kpmFd,
        std::bind (&LteEnbNetDevice::KpmSubscriptionCallback, this, std::placeholders::_1));

    Ptr<RicControlFunctionDescription> ricCtrlFd = Create<RicControlFunctionDescription> ();
    e2term->RegisterSmCallbackToE2Sm (300, ricCtrlFd,
                                      std::bind (&LteEnbNetDevice::ControlMessageReceivedCallback,
                                                  this, std::placeholders::_1));

    // trigger sending reports
    // test
    // Simulator::ScheduleWithContext (1, Seconds(0),
    //                                 &LteEnbNetDevice::BuildAndSendReportMessageTest, this, E2Termination::RicSubscriptionRequest_rval_s{});
    
    // E2Termination::RicSubscriptionRequest_rval_s params{};
    // BuildAndSendReportMessageTest(params);
  }
}

void
LteEnbNetDevice::BuildAndSendReportMessage(E2Termination::RicSubscriptionRequest_rval_s params)
{
  std::string plmId = m_ltePlmnId;
  std::string gnbId = std::to_string(m_cellId);

  // std::cout << "Node context BuildAndSendReportMessage " << Simulator::GetContext() << std::endl;

  // TODO here we can get something from RRC and onward
  NS_LOG_DEBUG("LteEnbNetDevice " << std::to_string(m_cellId) << " BuildAndSendMessage at time " << Simulator::Now().GetSeconds());

  // if(m_sendCuUp)
  // {
  //   // Create CU-UP
  //   Ptr<KpmIndicationHeader> header = BuildRicIndicationHeader(plmId, gnbId, m_cellId);
  //   Ptr<KpmIndicationMessage> cuUpMsg = BuildRicIndicationMessageCuUp(plmId);
  //   // Send CU-UP only if offline logging is disabled
  //   if (!m_forceE2FileLogging && header != nullptr && cuUpMsg != nullptr)
  //   {
  //     NS_LOG_DEBUG ("Send LTE CU-UP");
  //     E2AP_PDU *pdu_cuup_ue = new E2AP_PDU; 
  //     encoding::generate_e2apv1_indication_request_parameterized(pdu_cuup_ue, 
  //                                                              params.requestorId,
  //                                                              params.instanceId,
  //                                                              params.ranFuncionId,
  //                                                              params.actionId,
  //                                                              1, // TODO sequence number  
  //                                                              (uint8_t*) header->m_buffer, // buffer containing the encoded header
  //                                                              header->m_size, // size of the encoded header
  //                                                              (uint8_t*) cuUpMsg->m_buffer, // buffer containing the encoded message
  //                                                              cuUpMsg->m_size); // size of the encoded message
  //     m_e2term->SendE2Message (pdu_cuup_ue);
  //     delete pdu_cuup_ue;
  //   }
  // }

  // m_sendCuCp = false;
  if(m_sendCuCp)
  {
    for(std::map<uint16_t, std::map<uint64_t, std::map<uint16_t, SinrMcsPair>>>::const_iterator firstOrderIt = m_l3sinrMap.begin();
      firstOrderIt != m_l3sinrMap.end(); ++ firstOrderIt){

      uint16_t generatingCellId = firstOrderIt->first;
      std::string gnbId = std::to_string(generatingCellId);
      // Create CU-CP
      Ptr<KpmIndicationHeader> header = BuildRicIndicationHeader(plmId, gnbId, generatingCellId);
      // Ptr<KpmIndicationMessage> cuCpMsg = BuildRicIndicationMessageCuCp(plmId);
      Ptr<KpmIndicationMessage> cuCpMsg = BuildV2XRicIndicationMessageCucp(plmId, generatingCellId, m_l3sinrMap.size());

      // Send CU-CP only if offline logging is disabled
      if (!m_forceE2FileLogging && header != nullptr && cuCpMsg != nullptr)
      {
        NS_LOG_DEBUG ("Send LTE CU-CP for cell id " << generatingCellId);
        E2AP_PDU *pdu_cucp_ue = new E2AP_PDU; 
        encoding::generate_e2apv1_indication_request_parameterized(pdu_cucp_ue, 
                                                                  params.requestorId,
                                                                  params.instanceId,
                                                                  params.ranFuncionId,
                                                                  params.actionId,
                                                                  1, // TODO sequence number  
                                                                  (uint8_t*) header->m_buffer, // buffer containing the encoded header
                                                                  header->m_size, // size of the encoded header
                                                                  (uint8_t*) cuCpMsg->m_buffer, // buffer containing the encoded message
                                                                  cuCpMsg->m_size); // size of the encoded message  
        m_e2term->SendE2Message (pdu_cucp_ue);
        delete pdu_cucp_ue;
      }
    }

    // reseting used resources vector & l3 sinr map
    // in 10 ms there should be able to refill the l3sinrmap and 
    // used resources as well
    m_l3sinrMap.clear();
    m_ueUsedResourcesMap.clear();
  }
  
  // we need this, but we have to insert the info over the assigned resources
  // if(m_sendDu)
  // {
  //   // Create DU
  //   Ptr<KpmIndicationHeader> header = BuildRicIndicationHeader(plmId, gnbId, m_cellId);
  //   Ptr<KpmIndicationMessage> duMsg = BuildRicIndicationMessageDu(plmId, m_cellId);
  //   // Send DU only if offline logging is disabled
  //   if (!m_forceE2FileLogging && header != nullptr && duMsg != nullptr)
  //   {
  //     NS_LOG_DEBUG ("Send NR DU");
  //     E2AP_PDU *pdu_du_ue = new E2AP_PDU; 
  //     encoding::generate_e2apv1_indication_request_parameterized(pdu_du_ue, 
  //                                                              params.requestorId,
  //                                                              params.instanceId,
  //                                                              params.ranFuncionId,
  //                                                              params.actionId,
  //                                                              1, // TODO sequence number  
  //                                                              (uint8_t*) header->m_buffer, // buffer containing the encoded header
  //                                                              header->m_size, // size of the encoded header
  //                                                              (uint8_t*) duMsg->m_buffer, // buffer containing the encoded message
  //                                                              duMsg->m_size); // size of the encoded message  
  //     NS_LOG_DEBUG("Gnb id " << gnbId);
  //     // DecodeMessage(pdu_du_ue);
  //     m_e2term->SendE2Message (pdu_du_ue);
  //     delete pdu_du_ue;
  //   }
  // }

  if (!m_forceE2FileLogging)
    Simulator::ScheduleWithContext (1, Seconds (m_e2Periodicity),
                                    &LteEnbNetDevice::BuildAndSendReportMessage, this, params);
  else
    Simulator::Schedule (Seconds (m_e2Periodicity), &LteEnbNetDevice::BuildAndSendReportMessage,
                         this, params);
}

Ptr<KpmIndicationHeader>
LteEnbNetDevice::BuildRicIndicationHeader (std::string plmId, std::string gnbId, uint16_t nrCellId)
{
  if (!m_forceE2FileLogging)
    {
      KpmIndicationHeader::KpmRicIndicationHeaderValues headerValues;
      headerValues.m_plmId = plmId;
      headerValues.m_gnbId = gnbId;
      headerValues.m_nrCellId = nrCellId;
      auto time = Simulator::Now ();
      uint64_t timestamp = 0 + (uint64_t) time.GetMilliSeconds ();
      NS_LOG_DEBUG ("NR plmid " << plmId << " gnbId " << gnbId << " nrCellId " << nrCellId);
      NS_LOG_DEBUG ("Timestamp " << timestamp);
      headerValues.m_timestamp = timestamp;
      Ptr<KpmIndicationHeader> header =
          Create<KpmIndicationHeader> (KpmIndicationHeader::GlobalE2nodeType::eNB, headerValues);

      return header;
    }
  else
    {
      return nullptr;
    }
}

Ptr<KpmIndicationMessage>
LteEnbNetDevice::BuildRicIndicationMessageCuUp(std::string plmId)
{
  Ptr<LteIndicationMessageHelper> indicationMessageHelper =
      Create<LteIndicationMessageHelper> (IndicationMessageHelper::IndicationMessageType::CuUp,
                                          m_forceE2FileLogging, m_reducedPmValues);

  // shoulf be the logic of creating this type of msg
  // for the moment we let as it is
  return indicationMessageHelper->CreateIndicationMessage ();
    
}

Ptr<KpmIndicationMessage>
LteEnbNetDevice::BuildV2XRicIndicationMessageCucp(std::string plmId, uint16_t generatingCellId, size_t numOfReports)
{
  NS_LOG_FUNCTION(this);
  Ptr<LteIndicationMessageHelper> indicationMessageHelper =
      Create<LteIndicationMessageHelper> (IndicationMessageHelper::IndicationMessageType::CuCp,
                                             m_forceE2FileLogging, m_reducedPmValues);

  // for(std::map<uint16_t, std::map<uint64_t, std::map<uint16_t, SinrMcsPair>>>::const_iterator firstOrderIt = m_l3sinrMap.begin();
  //   firstOrderIt != m_l3sinrMap.end(); ++ firstOrderIt){
    std::map<uint16_t, std::map<uint64_t, std::map<uint16_t, SinrMcsPair>>>::iterator firstOrderIt = m_l3sinrMap.find(generatingCellId);
    // uint16_t generatingCellId = firstOrderIt->first;
    if(firstOrderIt!= m_l3sinrMap.end()){

      for(std::map<uint64_t, std::map<uint16_t, SinrMcsPair>>::const_iterator secondOrderIt = firstOrderIt->second.begin();
        secondOrderIt != firstOrderIt->second.end(); ++secondOrderIt){

        uint64_t imsi = secondOrderIt->first;
        std::string ueImsiComplete = GetImsiString(imsi);
        Ptr<L3RrcMeasurements> l3RrcMeasurementServing;
        std::map<uint16_t, SinrMcsPair>::const_iterator sameCellSinrIt = secondOrderIt->second.find(generatingCellId);
        std::string servingStr = ",,,";
        if(sameCellSinrIt!= secondOrderIt->second.end()){
          double convertedSinr = L3RrcMeasurements::ThreeGppMapSinr (sameCellSinrIt->second.sinr);
          if (!indicationMessageHelper->IsOffline ())
          {
            
            l3RrcMeasurementServing =
              L3RrcMeasurements::CreateL3RrcUeSpecificSinrMcsServing (generatingCellId, generatingCellId, 
              convertedSinr, sameCellSinrIt->second.mcs);
          }
          servingStr = std::to_string (generatingCellId) + "," +
                             std::to_string (sameCellSinrIt->second.sinr) + "," + 
                             std::to_string ((uint32_t)sameCellSinrIt->second.mcs);
        }
        std::string neighStr;
        Ptr<L3RrcMeasurements> l3RrcMeasurementNeigh;
        if (!indicationMessageHelper->IsOffline ())
        {
          l3RrcMeasurementNeigh = L3RrcMeasurements::CreateL3RrcUeSpecificSinrNeigh ();
        }
        //invert key and value in sortFlipMap, then sort by value
        std::multimap<SinrMcsPair, uint16_t> sortFlipMap = flip_map (secondOrderIt->second);
        uint16_t nNeighbours = E2SM_REPORT_MAX_NEIGH;
        if (m_l3sinrMap[generatingCellId][imsi].size () < nNeighbours)
        {
          nNeighbours = m_l3sinrMap[generatingCellId][imsi].size () - 1;
        }
        int itIndex = 0;
        for (std::map<SinrMcsPair, uint16_t>::iterator it = --sortFlipMap.end ();
          it != --sortFlipMap.begin () && itIndex < nNeighbours; it--)
        {
          uint16_t cellId = it->second;
          if (cellId != generatingCellId)
          {
            if (!indicationMessageHelper->IsOffline ())
            {
              double convertedSinr = L3RrcMeasurements::ThreeGppMapSinr (it->first.sinr);
              uint8_t mcsFromSinr = it->first.mcs;
              l3RrcMeasurementNeigh->AddNeighbourCellMeasurementMcs (cellId, convertedSinr, mcsFromSinr);

              neighStr += "," + std::to_string (cellId) + "," + std::to_string (it->first.sinr) + "," +
                        std::to_string ((uint32_t)mcsFromSinr);
            }
            itIndex++;
          }
        }
        // getting the number of resources
        uint32_t ueUsedResources = 0;
        std::map<uint64_t, std::map<uint16_t, uint32_t>>::iterator allUsedResourcesIt = m_ueUsedResourcesMap.find(imsi);
        if(allUsedResourcesIt!= m_ueUsedResourcesMap.end()){
          std::map<uint16_t, uint32_t>::iterator cellUsedResourcesIt = allUsedResourcesIt->second.find(generatingCellId);
          if(cellUsedResourcesIt!= allUsedResourcesIt->second.end()){
            ueUsedResources+= cellUsedResourcesIt->second;
          }
        }
        std::string ueUsedResourcesStr = std::to_string(ueUsedResources)+",";
        if (!indicationMessageHelper->IsOffline ())
        {
          // indicationMessageHelper->AddCuCpUePmItem (ueImsiComplete, generatingCellId, ueUsedResources,
          //                                           l3RrcMeasurementServing, l3RrcMeasurementNeigh);
          bool isElasticUser = false;
          
          indicationMessageHelper->AddCuCpUePmItem (ueImsiComplete, generatingCellId, ueUsedResources,
                                                    l3RrcMeasurementServing, l3RrcMeasurementNeigh, 
                                                    isElasticUser, 2);
          
          ueUsedResourcesStr += std::to_string(isElasticUser);
        }
        // m_cucpTrace(m_cellId, imsi, servingStr, neighStr, ueUsedResourcesStr);
      }
    
      
  }

  // clear the report vector
  // m_ueUsedResourcesMap.clear();
  // m_l3sinrMap.clear();

  if (!indicationMessageHelper->IsOffline ())
  {
    // Fill CuCp specific fields
    indicationMessageHelper->FillCuCpValues (numOfReports); // Number of Active UEs
  }

  if (m_forceE2FileLogging)
  {
    return nullptr;
  }else{
    return indicationMessageHelper->CreateIndicationMessage ();
  }

  // std::map<uint16_t, uint32_t> allUsedResources;
  // for(std::map<uint64_t, std::map<uint16_t, uint32_t>>::const_iterator mainIt = m_ueUsedResourcesMap.begin();
  // mainIt!= m_ueUsedResourcesMap.end(); ++mainIt){
  //   uint64_t imsi = mainIt->first;
  //   std::string ueImsiComplete = GetImsiString(imsi);
  //   for(std::map<uint16_t, uint32_t>::const_iterator internalIt = mainIt->second.begin(); internalIt != mainIt->second.end();
  //   ++internalIt){
  //     uint16_t cellId = internalIt->first;
  //     // get data from measurements
  //   }
  // }

  // build message

}

Ptr<KpmIndicationMessage>
LteEnbNetDevice::BuildRicIndicationMessageCuCp(std::string plmId)
{
  Ptr<LteIndicationMessageHelper> indicationMessageHelper =
      Create<LteIndicationMessageHelper> (IndicationMessageHelper::IndicationMessageType::CuCp,
                                       m_forceE2FileLogging, m_reducedPmValues);

  // auto ueMap = m_rrc->GetUeMap();
  // auto ueMapSize = ueMap.size ();
  // std::unordered_map<uint64_t, std::string> uePmString {};
  // for (auto ue : ueMap)
  // {
  //   uint64_t imsi = ue.second->GetImsi();
  //   std::string ueImsiComplete = GetImsiString(imsi);
  //   long numDrb = ue.second->GetDrbMap().size();
  //   if (!indicationMessageHelper->IsOffline ())
  //     {
  //       indicationMessageHelper->AddCuCpUePmItem (ueImsiComplete, numDrb, 0);
  //     }
  //   uePmString.insert(std::make_pair(imsi, std::to_string(numDrb) + "," +
  //     std::to_string(0)));
  // }
  // if (!indicationMessageHelper->IsOffline ())
  //   {
  //     indicationMessageHelper->FillCuCpValues (ueMapSize);
  //   }

  if (m_forceE2FileLogging) {
    std::ofstream csv {};
    csv.open (m_cuCpFileName.c_str (),  std::ios_base::app);
    if (!csv.is_open ())
    {
      NS_FATAL_ERROR ("Can't open file " << m_cuCpFileName.c_str ());
    }

    NS_LOG_DEBUG ("m_cuCpFileName open " << m_cuCpFileName);

    // the string is timestamp, ueImsiComplete, numActiveUes, DRB.EstabSucc.5QI.UEID (numDrb), DRB.RelActNbr.5QI.UEID (0)

    uint64_t timestamp = 0 + (uint64_t) Simulator::Now ().GetMilliSeconds ();

    // for (auto ue : ueMap)
    // {
    //   uint64_t imsi = ue.second->GetImsi();
    //   std::string ueImsiComplete = GetImsiString(imsi);
    //   auto uePms = uePmString.find(imsi)->second;
    //   std::string to_print = std::to_string (timestamp) + "," + ueImsiComplete + "," +
    //                          std::to_string (ueMapSize) + "," + uePms + ",,,,,,," +
    //                          "\n";
    //   NS_LOG_DEBUG(to_print);
    //   csv << to_print;
    // }

    csv.close ();
    
    return nullptr;
  }
  else
    {
      return indicationMessageHelper->CreateIndicationMessage ();
    }
}

std::string
LteEnbNetDevice::GetImsiString(uint64_t imsi)
{
  std::string ueImsi = std::to_string(imsi);
  std::string ueImsiComplete {};
  if (ueImsi.length() == 1)
  {
    ueImsiComplete = "0000" + ueImsi;
  }
  else if (ueImsi.length() == 2)
  {
    ueImsiComplete = "000" + ueImsi;
  }
  else
  {
    ueImsiComplete = "00" + ueImsi;
  }
  return ueImsiComplete;
}


TypeId LteEnbNetDevice::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::LteEnbNetDevice")
    .SetParent<LteNetDevice> ()
    .AddConstructor<LteEnbNetDevice> ()
    .AddAttribute ("LteEnbRrc",
                   "The RRC associated to this EnbNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&LteEnbNetDevice::m_rrc),
                   MakePointerChecker <LteEnbRrc> ())
    .AddAttribute ("LteHandoverAlgorithm",
                   "The handover algorithm associated to this EnbNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&LteEnbNetDevice::m_handoverAlgorithm),
                   MakePointerChecker <LteHandoverAlgorithm> ())
    .AddAttribute ("LteAnr",
                   "The automatic neighbour relation function associated to this EnbNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&LteEnbNetDevice::m_anr),
                   MakePointerChecker <LteAnr> ())
    .AddAttribute ("LteFfrAlgorithm",
                   "The FFR algorithm associated to this EnbNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&LteEnbNetDevice::m_ffrAlgorithm),
                   MakePointerChecker <LteFfrAlgorithm> ())
    .AddAttribute ("LteEnbComponentCarrierManager",
                   "The RRC associated to this EnbNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&LteEnbNetDevice::m_componentCarrierManager),
                   MakePointerChecker <LteEnbComponentCarrierManager> ())
    .AddAttribute ("ComponentCarrierMap", "List of component carriers.",
                   ObjectMapValue (),
                   MakeObjectMapAccessor (&LteEnbNetDevice::m_ccMap),
                   MakeObjectMapChecker<ComponentCarrierEnb> ())
    .AddAttribute ("UlBandwidth",
                   "Uplink Transmission Bandwidth Configuration in number of Resource Blocks",
                   UintegerValue (25),
                   MakeUintegerAccessor (&LteEnbNetDevice::SetUlBandwidth, 
                                         &LteEnbNetDevice::GetUlBandwidth),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("DlBandwidth",
                   "Downlink Transmission Bandwidth Configuration in number of Resource Blocks",
                   UintegerValue (25),
                   MakeUintegerAccessor (&LteEnbNetDevice::SetDlBandwidth, 
                                         &LteEnbNetDevice::GetDlBandwidth),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("CellId",
                   "Cell Identifier",
                   UintegerValue (0),
                   MakeUintegerAccessor (&LteEnbNetDevice::m_cellId),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("DlEarfcn",
                   "Downlink E-UTRA Absolute Radio Frequency Channel Number (EARFCN) "
                   "as per 3GPP 36.101 Section 5.7.3. ",
                   UintegerValue (100),
                   MakeUintegerAccessor (&LteEnbNetDevice::m_dlEarfcn),
                   MakeUintegerChecker<uint32_t> (0, 262143))
    .AddAttribute ("UlEarfcn",
                   "Uplink E-UTRA Absolute Radio Frequency Channel Number (EARFCN) "
                   "as per 3GPP 36.101 Section 5.7.3. ",
                   UintegerValue (18100),
                   MakeUintegerAccessor (&LteEnbNetDevice::m_ulEarfcn),
                   MakeUintegerChecker<uint32_t> (0, 262143))
    .AddAttribute ("CsgId",
                   "The Closed Subscriber Group (CSG) identity that this eNodeB belongs to",
                   UintegerValue (0),
                   MakeUintegerAccessor (&LteEnbNetDevice::SetCsgId,
                                         &LteEnbNetDevice::GetCsgId),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("CsgIndication",
                   "If true, only UEs which are members of the CSG (i.e. same CSG ID) "
                   "can gain access to the eNodeB, therefore enforcing closed access mode. "
                   "Otherwise, the eNodeB operates as a non-CSG cell and implements open access mode.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&LteEnbNetDevice::SetCsgIndication,
                                        &LteEnbNetDevice::GetCsgIndication),
                   MakeBooleanChecker ())

    // modified
    .AddAttribute ("E2Termination",
                   "The E2 termination object associated to this node",
                   PointerValue (),
                   MakePointerAccessor (&LteEnbNetDevice::SetE2Termination,
                                        &LteEnbNetDevice::GetE2Termination),
                   MakePointerChecker <E2Termination> ())

    .AddAttribute ("E2Periodicity",
                  "Periodicity of E2 reporting (value in seconds)",
                  DoubleValue (0.01),
                  MakeDoubleAccessor (&LteEnbNetDevice::m_e2Periodicity),
                  MakeDoubleChecker<double> ())
    .AddAttribute ("EnableDuReport",
                   "If true, send DuReport",
                   BooleanValue (false),
                   MakeBooleanAccessor (&LteEnbNetDevice::m_sendDu),
                   MakeBooleanChecker ())
    .AddAttribute ("EnableCuUpReport",
                   "If true, send CuUpReport",
                   BooleanValue (false),
                   MakeBooleanAccessor (&LteEnbNetDevice::m_sendCuUp),
                   MakeBooleanChecker ())
    .AddAttribute ("EnableCuCpReport",
                   "If true, send CuCpReport",
                   BooleanValue (true),
                   MakeBooleanAccessor (&LteEnbNetDevice::m_sendCuCp),
                   MakeBooleanChecker ())
    .AddAttribute ("EnableE2FileLogging",
                   "If true, force E2 indication generation and write E2 fields in csv file",
                   BooleanValue (false),
                   MakeBooleanAccessor (&LteEnbNetDevice::m_forceE2FileLogging),
                   MakeBooleanChecker ())
    .AddAttribute ("ControlFileName",
                   "Filename for the stand alone control mode. The file is deleted after every read."
                   "Format should correspond to the particular use case:\n"
                   "TS: Contains multiple lines with ts, imsi, targetCellId\n",
                   StringValue(""),
                   MakeStringAccessor (&LteEnbNetDevice::m_controlFilename),
                   MakeStringChecker())
    // .AddTraceSource("ReportE2CuCpLte",
    //                 "Report the du message",
    //                 MakeTraceSourceAccessor(&LteEnbNetDevice::m_cucpTrace),
    //                 "ns3::E2CuCp::ReportE2CuCpLteTracedCallback")
    .AddAttribute ("PlmnId",
                  "the plmn id identifying the lte coordinator; shall be used to "
                  " distinguish different simulation campaign involving the same xapp",
                  StringValue("111"),
                  MakeStringAccessor(&LteEnbNetDevice::m_ltePlmnId),
                  MakeStringChecker()
                  )
    .AddAttribute ("TracesPath",
                   "The path where to store the path. ",
                   StringValue ("./"),
                   MakeStringAccessor (&LteEnbNetDevice::m_tracesPath),
                   MakeStringChecker ())
    .AddAttribute ("ReducedPmValues",
                   "If true, send only a subset of pmValues",
                   BooleanValue (false),
                   MakeBooleanAccessor (&LteEnbNetDevice::m_reducedPmValues),
                   MakeBooleanChecker ())
    // end modification
  ;
  return tid;
}

LteEnbNetDevice::LteEnbNetDevice ()
  : m_isConstructed (false),
    m_isConfigured (false),
    m_anr (0),
    m_componentCarrierManager(0),
    // modified
    // m_isReportingEnabled (false),
    // m_forceE2FileLogging (false),
    m_cuUpFileName (),
    m_cuCpFileName (),
    m_ricControlReceivedFilename()
{
  NS_LOG_FUNCTION (this);
}

LteEnbNetDevice::~LteEnbNetDevice (void)
{
  NS_LOG_FUNCTION (this);
}

void
LteEnbNetDevice::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  m_rrc->Dispose ();
  m_rrc = 0;

  m_handoverAlgorithm->Dispose ();
  m_handoverAlgorithm = 0;

  if (m_anr != 0)
    {
      m_anr->Dispose ();
      m_anr = 0;
    }
  m_componentCarrierManager->Dispose();
  m_componentCarrierManager = 0;
  // ComponentCarrierEnb::DoDispose() will call DoDispose
  // of its PHY, MAC, FFR and scheduler instance
  for (uint32_t i = 0; i < m_ccMap.size (); i++)
    {
      m_ccMap.at (i)->Dispose ();
      m_ccMap.at (i) = 0;
    }
   
  LteNetDevice::DoDispose ();
}



Ptr<LteEnbMac>
LteEnbNetDevice::GetMac () const
{
  return GetMac (0);
}

Ptr<LteEnbPhy>
LteEnbNetDevice::GetPhy () const
{
  return GetPhy (0);
}

Ptr<LteEnbMac>
LteEnbNetDevice::GetMac (uint8_t index) const
{
  return DynamicCast<ComponentCarrierEnb> (m_ccMap.at (index))->GetMac ();
}

Ptr<LteEnbPhy>
LteEnbNetDevice::GetPhy(uint8_t index) const
{
  return DynamicCast<ComponentCarrierEnb> (m_ccMap.at (index))->GetPhy ();
}

Ptr<LteEnbRrc>
LteEnbNetDevice::GetRrc () const
{
  return m_rrc;
}

Ptr<LteEnbComponentCarrierManager>
LteEnbNetDevice::GetComponentCarrierManager () const
{
  return  m_componentCarrierManager;
}

uint16_t
LteEnbNetDevice::GetCellId () const
{
  return m_cellId;
}

std::vector<uint16_t>
LteEnbNetDevice::GetCellIds () const
{
  std::vector<uint16_t> cellIds;

  for (auto &it: m_ccMap)
    {
      cellIds.push_back (it.second->GetCellId ());
    }
  return cellIds;
}

bool
LteEnbNetDevice::HasCellId (uint16_t cellId) const
{
  return m_rrc->HasCellId (cellId);
}

uint16_t
LteEnbNetDevice::GetUlBandwidth () const
{
  return m_ulBandwidth;
}

void 
LteEnbNetDevice::SetUlBandwidth (uint16_t bw)
{ 
  NS_LOG_FUNCTION (this << bw);
  switch (bw)
    { 
    case 6:
    case 15:
    case 25:
    case 50:
    case 75:
    case 100:
      m_ulBandwidth = bw;
      break;

    default:
      NS_FATAL_ERROR ("invalid bandwidth value " << bw);
      break;
    }
}

uint16_t
LteEnbNetDevice::GetDlBandwidth () const
{
  return m_dlBandwidth;
}

void 
LteEnbNetDevice::SetDlBandwidth (uint16_t bw)
{
  NS_LOG_FUNCTION (this << uint16_t (bw));
  switch (bw)
    { 
    case 6:
    case 15:
    case 25:
    case 50:
    case 75:
    case 100:
      m_dlBandwidth = bw;
      break;

    default:
      NS_FATAL_ERROR ("invalid bandwidth value " << bw);
      break;
    }
}

uint32_t 
LteEnbNetDevice::GetDlEarfcn () const
{
  return m_dlEarfcn;
}

void 
LteEnbNetDevice::SetDlEarfcn (uint32_t earfcn)
{ 
  NS_LOG_FUNCTION (this << earfcn);
  m_dlEarfcn = earfcn;
}

uint32_t 
LteEnbNetDevice::GetUlEarfcn () const
{
  return m_ulEarfcn;
}

void 
LteEnbNetDevice::SetUlEarfcn (uint32_t earfcn)
{ 
  NS_LOG_FUNCTION (this << earfcn);
  m_ulEarfcn = earfcn;
}

uint32_t
LteEnbNetDevice::GetCsgId () const
{
  return m_csgId;
}

void
LteEnbNetDevice::SetCsgId (uint32_t csgId)
{
  NS_LOG_FUNCTION (this << csgId);
  m_csgId = csgId;
  UpdateConfig (); // propagate the change to RRC level
}

bool
LteEnbNetDevice::GetCsgIndication () const
{
  return m_csgIndication;
}

void
LteEnbNetDevice::SetCsgIndication (bool csgIndication)
{
  NS_LOG_FUNCTION (this << csgIndication);
  m_csgIndication = csgIndication;
  UpdateConfig (); // propagate the change to RRC level
}

std::map < uint8_t, Ptr<ComponentCarrierBaseStation> >
LteEnbNetDevice::GetCcMap () const
{
  return m_ccMap;
}

void
LteEnbNetDevice::SetCcMap (std::map< uint8_t, Ptr<ComponentCarrierBaseStation> > ccm)
{
  NS_ASSERT_MSG (!m_isConfigured, "attempt to set CC map after configuration");
  m_ccMap = ccm;
}

void 
LteEnbNetDevice::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_isConstructed = true;
  UpdateConfig ();
  for (auto it = m_ccMap.begin (); it != m_ccMap.end (); ++it)
    {
       it->second->Initialize ();
    }
  m_rrc->Initialize ();
  m_componentCarrierManager->Initialize();
  m_handoverAlgorithm->Initialize ();

  if (m_anr != 0)
    {
      m_anr->Initialize ();
    }

  m_ffrAlgorithm->Initialize ();
}


bool
LteEnbNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << dest << protocolNumber);
  NS_ABORT_MSG_IF (protocolNumber != Ipv4L3Protocol::PROT_NUMBER
                   && protocolNumber != Ipv6L3Protocol::PROT_NUMBER,
                   "unsupported protocol " << protocolNumber << ", only IPv4 and IPv6 are supported");
  return m_rrc->SendData (packet);
}


void
LteEnbNetDevice::UpdateConfig (void)
{
  NS_LOG_FUNCTION (this);

  if (m_isConstructed)
    {
      if (!m_isConfigured)
        {
          NS_LOG_LOGIC (this << " Configure cell " << m_cellId);
          // we have to make sure that this function is called only once
          NS_ASSERT (!m_ccMap.empty ());
          m_rrc->ConfigureCell (m_ccMap);
          m_isConfigured = true;
        }

      NS_LOG_LOGIC (this << " Updating SIB1 of cell " << m_cellId
                         << " with CSG ID " << m_csgId
                         << " and CSG indication " << m_csgIndication);
      m_rrc->SetCsgId (m_csgId, m_csgIndication);
    }
  else
    {
      /*
       * Lower layers are not ready yet, so do nothing now and expect
       * ``DoInitialize`` to re-invoke this function.
       */
    }
}


} // namespace ns3

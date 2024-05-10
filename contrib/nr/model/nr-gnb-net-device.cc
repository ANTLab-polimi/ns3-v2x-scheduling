/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#include "nr-gnb-net-device.h"
#include <ns3/object-map.h>
#include <ns3/lte-enb-rrc.h>
#include <ns3/abort.h>
#include <ns3/log.h>
#include <ns3/lte-enb-component-carrier-manager.h>
#include <ns3/ipv4-l3-protocol.h>
#include <ns3/ipv6-l3-protocol.h>

#include "bandwidth-part-gnb.h"
#include "nr-gnb-mac.h"
#include "nr-gnb-phy.h"
#include "bwp-manager-gnb.h"

#include <ns3/lte-indication-message-helper.h>
#include <control_message_encoder_decoder.h>
#include "encode_e2apv1.hpp"
#include "e2ap_asn1c_codec.h"

#include <iomanip>
#include <sstream>
#include <string>
#include <time.h>

#include <iostream>
#include <unistd.h>

const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrGnbNetDevice");

NS_OBJECT_ENSURE_REGISTERED ( NrGnbNetDevice);

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
NrGnbNetDevice::KpmSubscriptionCallback (E2AP_PDU_t* sub_req_pdu)
{
  NS_LOG_DEBUG ("\nReceived RIC Subscription Request, cellId = " << m_cellId << "\n");

  E2Termination::RicSubscriptionRequest_rval_s params = m_e2term->ProcessRicSubscriptionRequest (sub_req_pdu);
  NS_LOG_DEBUG ("requestorId " << +params.requestorId << 
                 ", instanceId " << +params.instanceId << 
                 ", ranFuncionId " << +params.ranFuncionId << 
                 ", actionId " << +params.actionId);  

  std::cout << "requestorId " << +params.requestorId << 
                 ", instanceId " << +params.instanceId << 
                 ", ranFuncionId " << +params.ranFuncionId << 
                 ", actionId " << +params.actionId << std::endl; 
  
  if (!m_isReportingEnabled && !m_forceE2FileLogging)
  {
    
    // Simulator::ScheduleWithContext (1, MilliSeconds(10),
    //                                &NrGnbNetDevice::BuildAndSendReportMessage, this, params);
    m_isReportingEnabled = true; 
  }
}

char* converHexToByteLocal(std::string hexString) {

    char * bytes = new char[hexString.length()/2];
    std::stringstream converter;

    for(int i = 0; i < hexString.length(); i+=2)
    {
        converter << std::hex << hexString.substr(i,2);
        int byte;
        converter >> byte;
        bytes[i/2] = byte & 0xFF;
        converter.str(std::string());
        converter.clear();
    }
    // char* bytesPointer= bytes;
    // return bytesPointer;
    return bytes;
}  

void NrGnbNetDevice::test_buffer_msg(){
  NS_LOG_FUNCTION(this << "Testing the decoding");
  const char* nrSlotAlloc = "000000002c0000000100000004e880000000000002be0b02000100000001ff000e00ffffffffffff0800050000003200ffff00000000000000000000";
  
  std::string _e2ap_pduString = std::string(nrSlotAlloc);
  std::cout << "Length of string " << _e2ap_pduString.size() << std::endl;

  char* bytes = converHexToByteLocal(_e2ap_pduString);

  
  int buffer_size = (int)_e2ap_pduString.length()/2;

  uint8_t *e2ApBuffer = (uint8_t *)calloc(1, buffer_size);
  memcpy(e2ApBuffer, bytes, buffer_size);

  NrSlGrantInfo nrSlGrantInfo;

  Buffer bufferSingleAlloc = Buffer();
  bufferSingleAlloc.Deserialize(e2ApBuffer, buffer_size);
  NrSlSlotAlloc singleAlloc = NrSlSlotAlloc();
  Buffer::Iterator bufferSingleAllocIt = bufferSingleAlloc.Begin();
  singleAlloc.DeserializeForE2(bufferSingleAllocIt);
  nrSlGrantInfo.slotAllocations.emplace(singleAlloc);

  NS_LOG_DEBUG("Deserializing the buffer " << singleAlloc.sfn.GetFrame() << " " << +singleAlloc.sfn.GetSubframe());

  NS_LOG_DEBUG("Deserializing the buffer " << singleAlloc.dstL2Id);

}

// void NrGnbNetDevice::test_buffer_msg(){
//   const char* _e2ap_pdu = "40313131010000000100000102003c000000002c0000000100000004584d000000000002be0b02000100000001ff000e00ffffffffffff0300030000003200ffff00000000000000000000003c000000002c0000000100000004e880000000000002be0b02000100000001ff000e00ffffffffffff0800050000003200ffff000000000000000000000101010101010200ff01010100";

//   std::string _e2ap_pduString = std::string(_e2ap_pdu);
//   std::cout << "Length of string " << _e2ap_pduString.size() << std::endl;

//   char* bytes = converHexToByteLocal(_e2ap_pduString);

//   // printf("Get length of string -> %d\n", (int)strlen(bytes));

//   // int buffer_size = (int)strlen(bytes);
//   int buffer_size = (int)_e2ap_pduString.length()/2;

//   // uint8_t *e2ApBuffer = (uint8_t *)calloc(1, (_e2ap_pduString.length()));
//   // memcpy(e2ApBuffer, bytes, (_e2ap_pduString.length()/2));

//   uint8_t *e2ApBuffer = (uint8_t *)calloc(1, buffer_size);
//   memcpy(e2ApBuffer, bytes, buffer_size);

//   RICcontrolRequest_IEs_t* singleRequest = (RICcontrolRequest_IEs_t *)calloc(1, sizeof(RICcontrolRequest_IEs_t));
//   singleRequest->value.present = RICcontrolRequest_IEs__value_PR_RICcontrolMessage;
//   singleRequest->value.choice.RICcontrolMessage.buf = e2ApBuffer;
//   singleRequest->value.choice.RICcontrolMessage.size = buffer_size;
//   singleRequest->id =23;

//   InitiatingMessage_t* initMsg = (InitiatingMessage_t * )calloc(1, sizeof(InitiatingMessage_t));
//   initMsg->value.present = InitiatingMessage__value_PR_RICcontrolRequest;
//   initMsg->criticality = Criticality_ignore;
//   ASN_SEQUENCE_ADD(&(initMsg->value.choice.RICcontrolRequest.protocolIEs.list) , singleRequest);    


//   E2AP_PDU_t* pdu = (E2AP_PDU_t * )calloc(1, sizeof(E2AP_PDU_t));
//   pdu->present = E2AP_PDU_PR_initiatingMessage;
//   pdu->choice.initiatingMessage = initMsg;

//   Ptr<RicControlMessage> controlMessage = Create<RicControlMessage> (pdu);

// }

void 
NrGnbNetDevice::ControlMessageReceivedCallback (E2AP_PDU_t* ric_pdu)
{
  NS_LOG_FUNCTION(this << m_xappRemainingDataSize << m_xappRemainingData);
  // here we receive a control message from the xapp, which we have to decode and actuate upon it
  try{
    Ptr<RicControlMessage> controlMessageMulti = Create<RicControlMessage> (ric_pdu, m_xappRemainingData, m_xappRemainingDataSize);
    
    m_xappRemainingDataSize = controlMessageMulti->m_remaining_bytes;
    for (std::vector<RicControlMessage::SchedulingSingleMsg>::const_iterator controlMessageIt = controlMessageMulti->m_schedulingListMulti.begin();
        controlMessageIt!= controlMessageMulti->m_schedulingListMulti.end(); ++controlMessageIt){
      
      NS_LOG_DEBUG ("\n Message received for plmn " << controlMessageIt->plmnString << " local plmn " << m_ltePlmnId);
      // xer_fprint(stderr, &asn_DEF_E2AP_PDU, ric_pdu);
      if (controlMessageIt->messageFormatType == RicControlMessage::ControlMessage_PR_v2xScheduling_Format){
        // checking if msg is intended for this simulation campaign/plmn
        std::string plmnId = controlMessageIt->plmnString;

        if(plmnId.compare(m_ltePlmnId)==0){
          NS_LOG_DEBUG("Received control message for this simulation branch with plmnid " << plmnId);
          // ue id and the vector with the pointers of single allocation
          //encoding::
          std::map<long, std::map<long, encoding::dest_sched_t>> resultMap = controlMessageIt->scheduleList;
          NS_LOG_DEBUG("The map size " << resultMap.size());

          SfnSf firstSfnsf;
          SfnSf lastSfnsf;

          if (resultMap.size()){

            for (std::map<long, std::map<long, encoding::dest_sched_t>>::iterator sourceUserIt = resultMap.begin(); 
              sourceUserIt!=resultMap.end(); ++sourceUserIt){
              uint64_t sourceId = (uint64_t)sourceUserIt->first;
              NS_LOG_DEBUG("Source id " << sourceId);
              NrSlGrantInfo nrSlGrantInfo = NrSlGrantInfo();

              for (std::map<long, encoding::dest_sched_t>::iterator singleUserIt = sourceUserIt->second.begin(); 
                singleUserIt!=sourceUserIt->second.end(); ++singleUserIt){
                uint64_t ueId = (uint64_t) singleUserIt->first;
                nrSlGrantInfo.cReselCounter = singleUserIt->second.cReselectionCounter;
                nrSlGrantInfo.slResoReselCounter = singleUserIt->second.slResourceReselectionCounter;
                nrSlGrantInfo.prevSlResoReselCounter = singleUserIt->second.prevSlResoReselCounter;
                nrSlGrantInfo.nrSlHarqId = singleUserIt->second.nrSlHarqId;
                nrSlGrantInfo.nSelected = singleUserIt->second.nSelected;
                nrSlGrantInfo.tbTxCounter = singleUserIt->second.tbTxCounter;
                
                for (auto &allocMessageBuffer: singleUserIt->second.singleUserAllocations)  {

                  Buffer bufferSingleAlloc = Buffer(0, false);
                  NS_LOG_DEBUG("Deserialization buffer size " << allocMessageBuffer.length); 
                  bufferSingleAlloc.Deserialize(allocMessageBuffer.buffer, allocMessageBuffer.length);
                  NrSlSlotAlloc singleAlloc = NrSlSlotAlloc();
                  Buffer::Iterator bufferSingleAllocIt = bufferSingleAlloc.Begin();
                  singleAlloc.DeserializeForE2(bufferSingleAllocIt);
                  auto emplaceRes = nrSlGrantInfo.slotAllocations.emplace(singleAlloc);
                  // std::cout << "Trying to insert sfn " << singleAlloc.GetString() << " res: " << emplaceRes.second << std::endl;
                  
                }
                
                if((sourceUserIt == resultMap.begin()) & (singleUserIt == sourceUserIt->second.begin()) & (nrSlGrantInfo.slotAllocations.size()>0)){
                  firstSfnsf = nrSlGrantInfo.slotAllocations.begin()->sfn;
                }
                
                if((sourceUserIt == std::prev(resultMap.end())) & (singleUserIt == std::prev(sourceUserIt->second.end())) & (nrSlGrantInfo.slotAllocations.size()>0)){
                  lastSfnsf = nrSlGrantInfo.slotAllocations.begin()->sfn;
                }
              }
              Simulator::ScheduleWithContext(1, Seconds(0), 
                    &NrGnbNetDevice::SendSchedulerDataToTrace, this, sourceId, nrSlGrantInfo, plmnId, controlMessageMulti->m_lastReport); 
            }
          }else{
            NrSlGrantInfo nrSlGrantInfo;
            Simulator::ScheduleWithContext(1, Seconds(0), 
                    &NrGnbNetDevice::SendSchedulerDataToTrace, this, 0, nrSlGrantInfo, plmnId, true); 
          }
        
          // NS_LOG_DEBUG("First slot " << firstSfnsf << " last slot " << lastSfnsf);
          std::cout << "First slot " << firstSfnsf << " last slot " << lastSfnsf << " curr slot " << GetPhy(0)->GetCurrentSfnSf() << std::endl;
        }
      }
    }
  }
  catch(...){
    std::cout << "Exception occured" << std::endl;
  }
}

Ptr<E2Termination>
NrGnbNetDevice::GetE2Termination() const
{
  return m_e2term;
}

void
NrGnbNetDevice::SetE2Termination(Ptr<E2Termination> e2term)
{
  m_e2term = e2term;

  NS_LOG_DEBUG("Register E2SM");

  if (!m_forceE2FileLogging)
  {
    Ptr<KpmFunctionDescription> kpmFd = Create<KpmFunctionDescription> ();
    e2term->RegisterKpmCallbackToE2Sm (200, kpmFd,
        std::bind (&NrGnbNetDevice::KpmSubscriptionCallback, this, std::placeholders::_1));

    Ptr<RicControlFunctionDescription> ricCtrlFd = Create<RicControlFunctionDescription> ();
    e2term->RegisterSmCallbackToE2Sm (300, ricCtrlFd,
                                      std::bind (&NrGnbNetDevice::ControlMessageReceivedCallback,
                                                  this, std::placeholders::_1));
  }
}

void 
NrGnbNetDevice::SendSchedulerDataToTrace(uint64_t ueId, NrSlGrantInfo nrSlGrantInfo, std::string plmnId, bool isLastReport){
  NS_LOG_FUNCTION(this << ueId << plmnId);
  
  m_e2V2XSchedulingTrace(ueId, nrSlGrantInfo, plmnId);
  if (isLastReport){
    std::cout << currentDateTime() <<  " - Received last report; Unblock simulation" <<std::endl;
    m_pauseSimulation = false;
  }
  
  
}

void
NrGnbNetDevice::PauseSimulation(){
  if (m_pauseSimulation){
    // std::cout << "PauseSimulation at slot " << GetPhy(0)->GetCurrentSfnSf() << std::endl;
    Simulator::Schedule(Seconds(0), &NrGnbNetDevice::PauseSimulation,
                         this);
  }
}

void
NrGnbNetDevice::PrintPdu(E2AP_PDU *pdu_cucp_ue){
  uint8_t *buf;
  // encoding::sctp_buffer_t data;
  unsigned char buffer[32768];
  memset(buffer, 0, 32768); 
  int length = 0;
  // uint8_t* buffer = (uint8_t*) calloc(1, 32768);
  // int length = encoding::e2ap_asn1c_encode_pdu_test(pdu_cucp_ue, &buf);
  auto er = asn_encode_to_buffer(nullptr, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2AP_PDU, pdu_cucp_ue, buf, 32768);
  length = (int)er.encoded;
  NS_LOG_DEBUG("Encoded successfully " <<  length);
  memcpy(buffer, buf, std::min(length, 32768));
  NS_LOG_DEBUG("Decoding message"); 
  // for(int i=0; i<length; ++i){
  //   std::cout << std::hex << (int)buffer[i];
  //     // std::cout << (int)buffer[i] << " ";
  // }
  // std::cout << std::endl;

  // std::stringstream ss;
  // ss << std::hex;
  // for( int i = 0 ; i < length; ++i ){
  //    std::cout << length << " ";
  //     ss << std::setw(2) << std::setfill('0') << (int)buffer[i];
  // }
  // std::cout << ss.str() << std::endl;

  static const char characters[] = "0123456789abcdef";
  // Zeroes out the buffer unnecessarily, can't be avoided for std::string.
  std::string ret(length * 2, 0);
  // Hack... Against the rules but avoids copying the whole buffer.
  auto bufHex = const_cast<char *>(ret.data());
  
  // for (const auto &oneInputByte : buffer){
  // *buf++ = characters[oneInputByte >> 4];
  //   *buf++ = characters[oneInputByte & 0x0F];
  // }
  for(int i=0; i<length; ++i){
    *bufHex++ = characters[buffer[i] >> 4];
    *bufHex++ = characters[buffer[i] & 0x0F];
  }

  std::cout << "REturn " << ret << std::endl;

  

  E2AP_PDU_t *pdu = nullptr;
  // E2AP_PDU_t *pdu = (E2AP_PDU_t *) calloc (1, sizeof (E2AP_PDU_t));
  asn_transfer_syntax syntax = ATS_ALIGNED_BASIC_PER;
  auto rval = asn_decode(nullptr, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2AP_PDU, (void **) &pdu,
                    buffer, length);
  NS_LOG_DEBUG("Message created, test decode " << (rval.code == RC_OK));
  // xer_fprint(stdout, &asn_DEF_E2AP_PDU,  (void*) pdu);  
  InitiatingMessage_t *initmsg = (InitiatingMessage_t *) pdu->choice.initiatingMessage;
  RICindication_t *ricIndication = (RICindication_t *) &initmsg->value.choice.RICindication;
  // xer_fprint(stdout, &asn_DEF_InitiatingMessage,  (void*) initmsg);  
  // xer_fprint(stdout, &asn_DEF_RICindication,  (void*) ricIndication);  
  size_t count = ricIndication->protocolIEs.list.count; 
  for (size_t i = 0; i < count; i++) 
  {
    
      RICindication_IEs *ie = ricIndication->protocolIEs.list.array [i];
      // NS_LOG_DEBUG("the index " << i << " count " << count << " pointer " << ie);
      // NS_LOG_DEBUG("the value " << ie->value);
      // NS_LOG_DEBUG("the present " << ie->value.present);
      switch (ie->value.present) {
        case RICindication_IEs__value_PR_RICindicationMessage: 
        {
          E2SM_KPM_IndicationMessage_t *format = (E2SM_KPM_IndicationMessage_t *) calloc (
                                                  1, sizeof (E2SM_KPM_IndicationMessage_t));
          ASN_STRUCT_RESET(asn_DEF_E2SM_KPM_IndicationMessage, format);
          asn_decode (nullptr, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2SM_KPM_IndicationMessage,
                      (void **) &format, ie->value.choice.RICindicationMessage.buf,
                      ie->value.choice.RICindicationMessage.size);
          NS_LOG_INFO ("Printing xml format");
          xer_fprint(stdout, &asn_DEF_E2SM_KPM_IndicationMessage, format);
        }
        case RICindication_IEs__value_PR_RICindicationHeader:  
        {
          E2SM_KPM_IndicationHeader_t *format = (E2SM_KPM_IndicationHeader_t *) calloc (
                                                1, sizeof (E2SM_KPM_IndicationHeader_t));
          ASN_STRUCT_RESET(asn_DEF_E2SM_KPM_IndicationHeader, format);
          asn_decode (nullptr, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2SM_KPM_IndicationHeader,
                      (void **) &format, ie->value.choice.RICindicationHeader.buf,
                      ie->value.choice.RICindicationHeader.size);
          // NS_LOG_INFO ("Printing header xml format");
          // xer_fprint(stdout, &asn_DEF_E2SM_KPM_IndicationHeader, format);
        }
      }
  }
}

void
NrGnbNetDevice::SendMessage(E2Termination::RicSubscriptionRequest_rval_s params, uint16_t generatingCellId,
                          Ptr<KpmIndicationHeader> header,
                          Ptr<KpmIndicationMessage> cuCpMsg)
{
  NS_LOG_FUNCTION(this);
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

    // test instead of sending decode to xml format to see if it is correct
    // xer_fprint(stdout, &asn_DEF_E2AP_PDU,  (void*) pdu_cucp_ue);  
    // PrintPdu(pdu_cucp_ue);
    
    std::cout << "Sending msg at slot " << GetPhy(0)->GetCurrentSfnSf() << std::endl;
    m_e2term->SendE2Message (pdu_cucp_ue);
    delete pdu_cucp_ue;
  }
}

void
NrGnbNetDevice::BuildAndSendReportMessage(E2Termination::RicSubscriptionRequest_rval_s params)
{
  std::string plmId = m_ltePlmnId;
  std::string gnbId = std::to_string(m_cellId);

  // TODO here we can get something from RRC and onward
  NS_LOG_DEBUG("NrGnbNetDevice " << std::to_string(m_cellId) << " BuildAndSendMessage at time " << Simulator::Now().GetSeconds());

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
    // uint16_t generatingCellId = firstOrderIt->first;
    uint16_t generatingCellId = 1;
    std::string gnbId = std::to_string(generatingCellId);
    // Create CU-CP

    uint32_t e2SciDataMapSize = m_e2SciDataMap.size();
    // we have an entry for each user
    std::map<uint16_t, std::map<uint16_t, ns3::PacketDelayStruct>> e2PacketDelaysInBufferFilterred;
    std::copy_if(m_e2PacketDelaysInBuffer.begin(), m_e2PacketDelaysInBuffer.end(),
              std::inserter(e2PacketDelaysInBufferFilterred, e2PacketDelaysInBufferFilterred.begin()),
              [](std::pair<uint16_t, std::map<uint16_t, ns3::PacketDelayStruct>> a) { return !a.second.empty(); }
              );
    // uint32_t packetDelayTxBufferSize = m_e2PacketDelaysInBuffer.size();
    // NS_LOG_DEBUG("Size of first " << m_e2PacketDelaysInBuffer.size() 
    //             <<" and filtered map " << e2PacketDelaysInBufferFilterred.size());
    uint32_t packetDelayTxBufferSize = e2PacketDelaysInBufferFilterred.size();
    // uint32_t packetDelayTxBufferSize = 2;
    uint32_t genericCount = 0;
    uint32_t ueDataAccumulateOrder = 1;
    NS_LOG_DEBUG("The size of packet delay " << packetDelayTxBufferSize);
    std::cout << "The size of packet delay " << packetDelayTxBufferSize << std::endl;
    // for (uint32_t genericCount = 0; genericCount<e2SciDataMapSize; genericCount+=ueDataAccumulateOrder){
    for (uint32_t genericCount = 0; genericCount<packetDelayTxBufferSize; genericCount+=ueDataAccumulateOrder){
      Ptr<KpmIndicationHeader> header = BuildRicIndicationHeader(plmId, gnbId, generatingCellId);
      // Ptr<KpmIndicationMessage> cuCpMsg = BuildV2XRicIndicationMessageCucp( plmId, generatingCellId, ueDataAccumulateOrder,
      //                                                                       std::next(m_e2SciDataMap.begin(), genericCount),
      //                                                                       m_e2SciDataMap.end(),
      //                                                                       e2SciDataMapSize);
      Ptr<KpmIndicationMessage> cuCpMsg = BuildV2XRicIndicationMessageCucp( plmId, generatingCellId, ueDataAccumulateOrder,
                                                                            std::next(e2PacketDelaysInBufferFilterred.begin(), genericCount),
                                                                            e2PacketDelaysInBufferFilterred.end(),
                                                                            packetDelayTxBufferSize);
      
      // NS_LOG_DEBUG("The pointer of ind message " << (cuCpMsg == nullptr)<< " " << cuCpMsg );

      // Send CU-CP only if offline logging is disabled
      
      SendMessage(params, generatingCellId, header, cuCpMsg);
      usleep(200000);
      // Simulator::ScheduleWithContext (1, NanoSeconds (1),
      //                               &NrGnbNetDevice::SendMessage, this, params, generatingCellId, header, cuCpMsg);
    }
    m_e2SciDataMap.clear();
    m_e2PacketDelaysInBuffer.clear();
    if (packetDelayTxBufferSize>0){
      // in case we have transmitted a buffer report, it means we have to expect the scheduling
      
      std::cout << currentDateTime() <<  " Blocking after sent data" << std::endl;
      m_pauseSimulation = true;
      // Simulator::Schedule(Seconds(0), &NrGnbNetDevice::PauseSimulation,
      //                    this);
      // Simulator::ScheduleWithContext(1, Seconds(0), 
      //                       &NrGnbNetDevice::PauseSimulation, this);
      Simulator::Schedule (Seconds(0), 
                            &NrGnbNetDevice::PauseSimulation, this);
    }
  
  }

  if (!m_forceE2FileLogging)
    Simulator::ScheduleWithContext (1, Seconds (m_e2Periodicity),
                                    &NrGnbNetDevice::BuildAndSendReportMessage, this, params);
  else
    Simulator::Schedule (Seconds (m_e2Periodicity), &NrGnbNetDevice::BuildAndSendReportMessage,
                         this, params);
}

Ptr<KpmIndicationHeader>
NrGnbNetDevice::BuildRicIndicationHeader (std::string plmId, std::string gnbId, uint16_t nrCellId)
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
NrGnbNetDevice::BuildRicIndicationMessageCuUp(std::string plmId)
{
  Ptr<LteIndicationMessageHelper> indicationMessageHelper =
      Create<LteIndicationMessageHelper> (IndicationMessageHelper::IndicationMessageType::CuUp,
                                          m_forceE2FileLogging, m_reducedPmValues);

  // shoulf be the logic of creating this type of msg
  // for the moment we let as it is
  return indicationMessageHelper->CreateIndicationMessage ();
    
}
Ptr<KpmIndicationMessage>
NrGnbNetDevice::BuildV2XRicIndicationMessageCucp(std::string plmId, uint16_t generatingCellId, uint32_t numSingleReports, 
                                        std::map<uint16_t, std::map<uint16_t, PacketDelayStruct>>::iterator itBegin,
                                        std::map<uint16_t, std::map<uint16_t, PacketDelayStruct>>::iterator itEnd, 
                                        size_t numOfReports)
{
  NS_LOG_FUNCTION(this << plmId << numSingleReports << numOfReports);
  ////////////////**** This function sends numSingleReports reports in  single e2 RIC message
  Ptr<LteIndicationMessageHelper> indicationMessageHelper =
      Create<LteIndicationMessageHelper> (IndicationMessageHelper::IndicationMessageType::CuCp,
                                             m_forceE2FileLogging, m_reducedPmValues);
  // for(std::map<uint64_t, std::vector<E2SciData>>::const_iterator e2SciDateMapIt = m_e2SciDataMap.begin();
  //       e2SciDateMapIt != m_e2SciDataMap.end(); ++e2SciDateMapIt){
  uint32_t count = 0;

  std::map<uint16_t, std::map<uint16_t, DestBufferSize>> bufferPacketDelays = AggregateData(itBegin, itEnd, numSingleReports);
  // iterate the packet delay in tx buffer and inert it in the vector
  // Ptr<V2XAllUsersBufferPacketDelays> v2XAllUsersBufferPacketDelays = Create<V2XAllUsersBufferPacketDelays> ();
  
  // NS_LOG_DEBUG("Entering the buffer pakcet delay data with size " << bufferPacketDelays.size());
  // adding the delay map
  // std::string _all_buffer_str = plmId + ";" + std::to_string(generatingCellId) + ";" + "[";
  for (std::map<uint16_t, std::map<uint16_t, DestBufferSize>>::iterator userAllBufferPacketDelaysIt = bufferPacketDelays.begin(); 
      userAllBufferPacketDelaysIt != bufferPacketDelays.end(); ++userAllBufferPacketDelaysIt)
  {
    // NS_LOG_DEBUG("Source id " << userAllBufferPacketDelaysIt->first);
    Ptr<V2XUserBufferPacketDelays> v2xUserBufferPacketDelays = Create<V2XUserBufferPacketDelays> ((long)userAllBufferPacketDelaysIt->first);
    // iterate over the dest nodes
    std::string _source_ue_id_str = "[" + std::to_string(userAllBufferPacketDelaysIt->first) + "," + "[";
    for (std::map<uint16_t, DestBufferSize>::iterator userBufferPacketDelaysIt = userAllBufferPacketDelaysIt->second.begin(); 
      userBufferPacketDelaysIt != userAllBufferPacketDelaysIt->second.end(); ++userBufferPacketDelaysIt)
    {
      
      // reservation period from the original data vector
      uint64_t reservationPeriod = m_e2PacketDelaysInBuffer[userAllBufferPacketDelaysIt->first][userBufferPacketDelaysIt->first].reservationPeriod;
      
      Ptr<V2XBufferPacketDelayIntervalsWrap> v2xSingleDestBufferPacketDelays = Create<V2XBufferPacketDelayIntervalsWrap> ((long)userBufferPacketDelaysIt->first);
      
      // NS_LOG_DEBUG("Source id " << userAllBufferPacketDelaysIt->first << " dest id " << userBufferPacketDelaysIt->first
      //             <<" size " << userBufferPacketDelaysIt->second.packetIntervalBufferSize.size());
      // std::string _str_obj = "";
      std::string _dest_ue_id_str = std::to_string(userBufferPacketDelaysIt->first) + "," + "[";
      for (std::map<uint32_t, BufferSize>::iterator itervalsIt = userBufferPacketDelaysIt->second.packetIntervalBufferSize.begin(); 
          itervalsIt!=userBufferPacketDelaysIt->second.packetIntervalBufferSize.end(); ++itervalsIt){
        // create the interval object and add to the list of buffer intervals
        uint32_t intervalLowerLimit = itervalsIt->first*m_packetDelayGroupingGranularity;
        uint32_t intervalUpperLimit = itervalsIt->first*m_packetDelayGroupingGranularity+m_packetDelayGroupingGranularity;
        uint32_t packetNumberInInterval = itervalsIt->second.num_packets;
        uint32_t bufferSize = itervalsIt->second.buffer_size;

        // NS_LOG_DEBUG("lower limit " << intervalLowerLimit << " upper limit " << intervalUpperLimit
        //             << " pack numb " << packetNumberInInterval << " buff size " << bufferSize);

        Ptr<V2XPacketDelayIntervalWrap> v2xPacketDelayInterval = Create<V2XPacketDelayIntervalWrap> ((long)intervalLowerLimit,
                                                                                      (long)intervalUpperLimit, 
                                                                                      (long)packetNumberInInterval, (long)reservationPeriod,
                                                                                      (long) bufferSize);
        // xer_fprint(stdout, &asn_DEF_V2XPacketDelayInterval,  (void*) v2xPacketDelayInterval->GetPointer());  
        v2xSingleDestBufferPacketDelays->AddPacketInterval(v2xPacketDelayInterval->GetPointer());
        std::string _str_obj = "["+ std::to_string(intervalLowerLimit) + "," + std::to_string(intervalUpperLimit) + "," + std::to_string(packetNumberInInterval) + \
                               "," + std::to_string(reservationPeriod)  + "," + std::to_string(bufferSize) + "]";
        if (itervalsIt == userBufferPacketDelaysIt->second.packetIntervalBufferSize.begin()){
          _dest_ue_id_str += _str_obj;
        }else{
          _dest_ue_id_str +=  ("," + _str_obj);
        }
      }
      _dest_ue_id_str += "]";

      if (userBufferPacketDelaysIt == userAllBufferPacketDelaysIt->second.begin()){
          _source_ue_id_str += _dest_ue_id_str;
        }else{
          _source_ue_id_str +=  ("|" + _dest_ue_id_str);
        }
      
      // harq buffer size
      // for (std::map<uint8_t, uint32_t>::iterator harqBufferIt = userBufferPacketDelaysIt->second.harqIdBufferSize.begin();
      //     harqBufferIt != userBufferPacketDelaysIt->second.harqIdBufferSize.end(); ++ harqBufferIt){
      //   Ptr<V2XHarqBufferSizeWrap> v2XHarqBufferSizeWrap = Create<V2XHarqBufferSizeWrap> ((long) harqBufferIt->first, 
      //                                                               (long) 0, 
      //                                                               (long) harqBufferIt->second);
      //   v2xSingleDestBufferPacketDelays->AddHarqBufferSize(v2XHarqBufferSizeWrap->GetPointer());
      // }
      v2xUserBufferPacketDelays->AddBufferPacketDelays(v2xSingleDestBufferPacketDelays->GetPointer());
      // xer_fprint(stdout, &asn_DEF_V2XBufferPacketDelays,  (void*) v2xSingleDestBufferPacketDelays->GetPointer());
    }
    _source_ue_id_str += "]";

    // v2XAllUsersBufferPacketDelays->AddUserBufferPacketDelays(v2xUserBufferPacketDelays->GetPointer());
    // xer_fprint(stdout, &asn_DEF_V2XAllBufferPacketDelays,  (void*) v2xUserBufferPacketDelays->GetPointer());
    // add sci messages received for the same user in the same structure

    // Ptr<V2XUserReceivedSciMessages> v2xUserReceivedSciMessages;
    Ptr<V2XUserReceivedSciMessages> v2xUserReceivedSciMessages = Create<V2XUserReceivedSciMessages> ();
    std::map<uint16_t, std::vector<E2SciData>>::iterator e2SciDateMapIt = m_e2SciDataMap.find(userAllBufferPacketDelaysIt->first);
    std::string _e2SciDataStr = "[";
    if(e2SciDateMapIt != m_e2SciDataMap.end()){
      // uint16_t localRnti = e2SciDateMapIt->first;
      
      // iterate the sci message vector of a user and insert in the sequence
      // v2xUserReceivedSciMessages = Create<V2XUserReceivedSciMessages> ();
      
      for (std::vector<NrGnbNetDevice::E2SciData>::iterator  e2SciData =  e2SciDateMapIt->second.begin(); e2SciData!= e2SciDateMapIt->second.end(); ++e2SciData){
      // for (auto &e2SciData: e2SciDateMapIt->second){
        uint16_t rnti = e2SciData->rnti;
        // here we serialize the stuct into buffer and inser in the indication msg
        // NS_LOG_DEBUG("Creating header buffer");
        uint32_t headerSize = e2SciData->sciF1Header.GetSerializedSizeForE2();
        // NS_LOG_DEBUG("Creating sci header size " << headerSize);
        Buffer bufferSciHeader = Buffer();
        bufferSciHeader.AddAtStart(headerSize);
        // Buffer::Iterator bufferIterator = bufferSciHeader.Begin();
        e2SciData->sciF1Header.SerializeForE2(bufferSciHeader.Begin());
        uint32_t extraSizeHeader = 30;
        uint8_t *bufferSciHeaderBuffer = (uint8_t *) calloc (1, headerSize+extraSizeHeader);
        auto serRes = bufferSciHeader.Serialize(bufferSciHeaderBuffer, headerSize+extraSizeHeader);
        
        extraSizeHeader = bufferSciHeader.GetSerializedSize() - bufferSciHeader.GetSize();
        // // check for decentralization
        uint8_t *bufferHeader = (uint8_t *)calloc(1, headerSize+extraSizeHeader+4);
        memcpy(bufferHeader, bufferSciHeaderBuffer, headerSize+extraSizeHeader+4);
        // Buffer bufferSciHeaderTest = Buffer();
        // bufferSciHeaderTest.Deserialize(bufferSciHeaderBuffer, headerSize+extraSize+4);
        // NrSlSciF1aHeader testHeader = NrSlSciF1aHeader();
        // Buffer::Iterator bufferSciHeaderTestIt = bufferSciHeaderTest.Begin();
        // testHeader.DeserializeForE2(bufferSciHeaderTestIt);
        // NS_LOG_DEBUG("Serialization and deserealization is the same " << (uint32_t)(testHeader == e2SciData.sciF1Header));
        
        uint32_t tagSize = e2SciData->sciTag.GetSerializedSizeForE2();
        Buffer bufferSciTag = Buffer();
        bufferSciTag.AddAtStart(tagSize);
        Buffer::Iterator bufferSciTagIterator = bufferSciTag.Begin();
        e2SciData->sciTag.SerializeForE2(bufferSciTagIterator);
        uint32_t extraSizeTag = 30;
        uint8_t *bufferSciTagBuffer = (uint8_t *) calloc (1, tagSize+extraSizeTag);
        bufferSciTag.Serialize(bufferSciTagBuffer, tagSize+extraSizeTag);

        extraSizeTag = bufferSciTag.GetSerializedSize() - bufferSciTag.GetSize();
        // // check for decentralization
        uint8_t *bufferTag = (uint8_t *)calloc(1, tagSize+extraSizeTag+4);
        memcpy(bufferTag, bufferSciTagBuffer, tagSize+extraSizeTag+4);
        // Buffer bufferSciTagTest = Buffer();
        // bufferSciTagTest.Deserialize(bufferSciTagBuffer, tagSize+extraSize+4);
        // NrSlMacPduTag testTag = NrSlMacPduTag();
        // Buffer::Iterator bufferSciTagTestIt = bufferSciTagTest.Begin();
        // testTag.DeserializeForE2(bufferSciTagTestIt);
        // NS_LOG_DEBUG("Serialization and deserealization of tag is the same " << (uint32_t)(testTag == e2SciData.sciTag));
        v2xUserReceivedSciMessages->addReceivedSciMessage(bufferHeader, headerSize+extraSizeHeader+4, 
                                                  bufferTag, tagSize+extraSizeTag+4, e2SciData->rsrpdBm);
        // free(bufferSciTagBuffer);
        // free(bufferSciHeaderBuffer);

        std::string _e2SciDataStrSingle = "[" + std::to_string(rnti) + "," + e2SciData->sciF1Header.GetString() + "," + e2SciData->sciTag.GetString() + "]";
      
        if(e2SciData ==  e2SciDateMapIt->second.begin()){
          _e2SciDataStr += _e2SciDataStrSingle;
        }else{
          _e2SciDataStr += ("," + _e2SciDataStrSingle);
        }
      }
      
      // _source_ue_id_str+= "," + _e2SciDataStr;
      // xer_fprint(stdout, &asn_DEF_V2XSciMessageItemList,  (void*) v2xUserReceivedSciMessages->GetPointer());
    }
    _e2SciDataStr += "]";
    // else{
    //   // Add empty sci data
    //    _source_ue_id_str+= ",[]";
    // }

    // adding the two pointers to the structure
    Ptr<V2XSingleReportWrap> v2XSingleReport = Create<V2XSingleReportWrap> (0 , 0); // position
    // NS_LOG_DEBUG("Pointer of packet buffer delays " << v2xUserBufferPacketDelays);
    v2XSingleReport->AddUserBufferPacketDelays(v2xUserBufferPacketDelays->GetPointer());
    // NS_LOG_DEBUG("Pointer of sci messages " << v2xUserReceivedSciMessages);
    if (v2xUserReceivedSciMessages!=nullptr){
      v2XSingleReport->AddUserReceivedSciMessageList(v2xUserReceivedSciMessages->GetPointer());
    }
    // v2XSingleReport->AddUserBufferAndSciMessageList(v2xUserBufferPacketDelays->GetPointer(), 
    //                                                   v2xUserReceivedSciMessages->GetPointer());

    // get position of user
    double x = 0;
    double y = 0;
    std::map<uint16_t, NodePosition>::iterator e2PosIt = m_e2NodePositions.find(userAllBufferPacketDelaysIt->first);

    if (e2PosIt!= m_e2NodePositions.end()){
      x = e2PosIt->second.x;
      y = e2PosIt->second.y;
    }

    // _source_ue_id_str+= "," + std::to_string(x) + "," + std::to_string(y) + "]";
    _source_ue_id_str += "]";

    // if (userAllBufferPacketDelaysIt == bufferPacketDelays.begin()){
    //   _all_buffer_str += _source_ue_id_str;
    // }else{
    //   _all_buffer_str +=  ("," + _source_ue_id_str);
    // }

    // std::cout << " Sci " << _e2SciDataStr << std::endl;

    // adding the report to the pm containers and the sfnsf
    std::string ueImsiComplete = GetImsiString((uint64_t)userAllBufferPacketDelaysIt->first);
    uint64_t timestamp =  (uint64_t) Simulator::Now ().GetMilliSeconds ();
    // NS_LOG_DEBUG("Preparing data for imsi " << ueImsiComplete);
    indicationMessageHelper->AddCuCpUePmItem (ueImsiComplete, generatingCellId, v2XSingleReport, x, y, 
                                              GetPhy(0)->GetCurrentSfnSf().GetFrame(),
                                              GetPhy(0)->GetCurrentSfnSf().GetSubframe(),
                                              GetPhy(0)->GetCurrentSfnSf().GetSlot(),
                                              timestamp);

    
    // xer_fprint (stdout, &asn_DEF_V2X_Single_User_Report, (void*)v2XSingleReport->GetPointer());

    if (!m_forceE2FileLogging){
      // file to write
      std::ofstream csv {};
      csv.open (m_ricControlReceivedFilename.c_str (),  std::ios_base::app);
      if (!csv.is_open ())
      {
        NS_FATAL_ERROR ("Can't open file " << m_ricControlReceivedFilename.c_str ());
      }

      csv << plmId << ";" << std::to_string(generatingCellId) << ";" << ueImsiComplete << ";" << _source_ue_id_str << ";" << _e2SciDataStr;
      csv << ";" << std::to_string(GetPhy(0)->GetCurrentSfnSf().GetFrame()) << ";" << std::to_string(GetPhy(0)->GetCurrentSfnSf().GetSubframe()) ;
      csv << ";" << std::to_string(GetPhy(0)->GetCurrentSfnSf().GetSlot()) ;
      csv << ";" << std::to_string(x) << ";" << std::to_string(y) << ";" << std::to_string(timestamp);
      csv << "\n";
      csv.close();
    }
  }

  // _all_buffer_str +="]";
  

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
}


Ptr<KpmIndicationMessage>
NrGnbNetDevice::BuildMillicarRicIndicationMessageCucp(std::string plmId, uint16_t generatingCellId, size_t numOfReports)
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
}

Ptr<KpmIndicationMessage>
NrGnbNetDevice::BuildRicIndicationMessageCuCp(std::string plmId)
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

    // uint64_t timestamp = 0 + (uint64_t) Simulator::Now ().GetMilliSeconds ();

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
NrGnbNetDevice::GetImsiString(uint64_t imsi)
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


TypeId
NrGnbNetDevice::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::NrGnbNetDevice").SetParent<NrNetDevice> ()
    .AddConstructor<NrGnbNetDevice> ()
    .AddAttribute ("LteEnbComponentCarrierManager",
                   "The component carrier manager associated to this EnbNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&NrGnbNetDevice::m_componentCarrierManager),
                   MakePointerChecker <LteEnbComponentCarrierManager> ())
    .AddAttribute ("BandwidthPartMap", "List of Bandwidth Part container.",
                   ObjectMapValue (),
                   MakeObjectMapAccessor (&NrGnbNetDevice::m_ccMap),
                   MakeObjectMapChecker<BandwidthPartGnb> ())
    .AddAttribute ("LteEnbRrc", "The RRC layer associated with the ENB", PointerValue (),
                   MakePointerAccessor (&NrGnbNetDevice::m_rrc),
                   MakePointerChecker<LteEnbRrc> ())
    // modified
    .AddAttribute ("E2Termination",
                   "The E2 termination object associated to this node",
                   PointerValue (),
                   MakePointerAccessor (&NrGnbNetDevice::SetE2Termination,
                                        &NrGnbNetDevice::GetE2Termination),
                   MakePointerChecker <E2Termination> ())

    .AddAttribute ("E2Periodicity",
                  "Periodicity of E2 reporting (value in seconds)",
                  DoubleValue (0.01),
                  MakeDoubleAccessor (&NrGnbNetDevice::m_e2Periodicity),
                  MakeDoubleChecker<double> ())
    .AddAttribute ("EnableDuReport",
                   "If true, send DuReport",
                   BooleanValue (false),
                   MakeBooleanAccessor (&NrGnbNetDevice::m_sendDu),
                   MakeBooleanChecker ())
    .AddAttribute ("EnableCuUpReport",
                   "If true, send CuUpReport",
                   BooleanValue (false),
                   MakeBooleanAccessor (&NrGnbNetDevice::m_sendCuUp),
                   MakeBooleanChecker ())
    .AddAttribute ("EnableCuCpReport",
                   "If true, send CuCpReport",
                   BooleanValue (true),
                   MakeBooleanAccessor (&NrGnbNetDevice::m_sendCuCp),
                   MakeBooleanChecker ())
    .AddAttribute ("EnableE2FileLogging",
                   "If true, force E2 indication generation and write E2 fields in csv file",
                   BooleanValue (false),
                   MakeBooleanAccessor (&NrGnbNetDevice::m_forceE2FileLogging),
                   MakeBooleanChecker ())
    .AddAttribute ("ControlFileName",
                   "Filename for the stand alone control mode. The file is deleted after every read."
                   "Format should correspond to the particular use case:\n"
                   "TS: Contains multiple lines with ts, imsi, targetCellId\n",
                   StringValue(""),
                   MakeStringAccessor (&NrGnbNetDevice::m_controlFilename),
                   MakeStringChecker())
    // .AddTraceSource("ReportE2CuCpLte",
    //                 "Report the du message",
    //                 MakeTraceSourceAccessor(&NrGnbNetDevice::m_cucpTrace),
    //                 "ns3::E2CuCp::ReportE2CuCpLteTracedCallback")
    .AddAttribute ("PlmnId",
                  "the plmn id identifying the lte coordinator; shall be used to "
                  " distinguish different simulation campaign involving the same xapp",
                  StringValue("111"),
                  MakeStringAccessor(&NrGnbNetDevice::m_ltePlmnId),
                  MakeStringChecker()
                  )
    .AddAttribute ("TracesPath",
                   "The path where to store the path. ",
                   StringValue ("./"),
                   MakeStringAccessor (&NrGnbNetDevice::m_tracesPath),
                   MakeStringChecker ())
    .AddAttribute ("ReducedPmValues",
                   "If true, send only a subset of pmValues",
                   BooleanValue (false),
                   MakeBooleanAccessor (&NrGnbNetDevice::m_reducedPmValues),
                   MakeBooleanChecker ())
    .AddAttribute("TxDelayGroupGranularityMs", 
                  "The granularity of grouping delays of packet "
                  "in tx buffer in milliseconds",
                  UintegerValue (10),
                  MakeUintegerAccessor (&NrGnbNetDevice::m_packetDelayGroupingGranularity),
                  MakeUintegerChecker<uint32_t> (1))
    .AddTraceSource ("SlV2XScheduling",
                     "The traces which shall be activated when gnb receives scheduling from the xApp"
                     "This shall be captured by the ues at the end and treated as control message",
                     MakeTraceSourceAccessor (&NrGnbNetDevice::m_e2V2XSchedulingTrace), 
                     "ns3::NrGnbNetDevice::SlV2XSchedulingTracedCallback")
    // end modification
    ;
  return tid;
}

NrGnbNetDevice::NrGnbNetDevice ()
  : m_cellId (0),
  m_forceE2FileLogging (false),
  m_isReportingEnabled (false)
{
  NS_LOG_FUNCTION (this);
  m_xappRemainingData = (uint8_t*) calloc (10000, sizeof(uint8_t));
}

NrGnbNetDevice::~NrGnbNetDevice ()
{
  NS_LOG_FUNCTION (this);
}

Ptr<NrMacScheduler>
NrGnbNetDevice::GetScheduler(uint8_t index) const
{
  NS_LOG_FUNCTION (this);
  return m_ccMap.at (index)->GetScheduler ();
}

void
NrGnbNetDevice::SetCcMap (const std::map< uint8_t, Ptr<BandwidthPartGnb> > &ccm)
{
  NS_ABORT_IF (m_ccMap.size () > 0);
  m_ccMap = ccm;
}

uint32_t
NrGnbNetDevice::GetCcMapSize() const
{
  return static_cast<uint32_t> (m_ccMap.size ());
}

void
NrGnbNetDevice::RouteIngoingCtrlMsgs (const std::list<Ptr<NrControlMessage> > &msgList,
                                          uint8_t sourceBwpId)
{
  NS_LOG_FUNCTION (this);

  for (const auto & msg : msgList)
    {
      uint8_t bwpId = DynamicCast<BwpManagerGnb> (m_componentCarrierManager)->RouteIngoingCtrlMsgs (msg, sourceBwpId);
      m_ccMap.at (bwpId)->GetPhy ()->PhyCtrlMessagesReceived (msg);
    }
}

void
NrGnbNetDevice::RouteOutgoingCtrlMsgs (const std::list<Ptr<NrControlMessage> > &msgList,
                                           uint8_t sourceBwpId)
{
  // NS_LOG_FUNCTION (this);

  for (const auto & msg : msgList)
    {
      uint8_t bwpId = DynamicCast<BwpManagerGnb> (m_componentCarrierManager)->RouteOutgoingCtrlMsg (msg, sourceBwpId);
      NS_ASSERT_MSG (m_ccMap.size () > bwpId, "Returned bwp " << +bwpId << " is not present. Check your configuration");
      NS_ASSERT_MSG (m_ccMap.at (bwpId)->GetPhy ()->HasDlSlot (),
                     "Returned bwp " << +bwpId << " has no DL slot, so the message can't go out. Check your configuration");
      m_ccMap.at (bwpId)->GetPhy ()->EncodeCtrlMsg (msg);
    }
}

void
NrGnbNetDevice::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_rrc->Initialize ();

  NrNetDevice::DoInitialize ();

  // test_buffer_msg();
}

void
NrGnbNetDevice::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  m_rrc->Dispose ();
  m_rrc = nullptr;
  for (const auto &it: m_ccMap)
    {
      it.second->Dispose ();
    }
  m_ccMap.clear ();
  m_componentCarrierManager->Dispose ();
  m_componentCarrierManager = nullptr;
  NrNetDevice::DoDispose ();
}

Ptr<NrGnbMac>
NrGnbNetDevice::GetMac (uint8_t index) const
{
  return m_ccMap.at (index)->GetMac ();
}

Ptr<NrGnbPhy>
NrGnbNetDevice::GetPhy (uint8_t index) const
{
  // NS_LOG_FUNCTION (this);
  return m_ccMap.at (index)->GetPhy ();
}

Ptr<BwpManagerGnb>
NrGnbNetDevice::GetBwpManager () const
{
  return DynamicCast<BwpManagerGnb> (m_componentCarrierManager);
}

uint16_t
NrGnbNetDevice::GetCellId () const
{
  NS_LOG_FUNCTION (this);
  return m_cellId;
}

std::vector<uint16_t>
NrGnbNetDevice::GetCellIds () const
{
  std::vector<uint16_t> cellIds;

  for (auto &it: m_ccMap)
    {
      cellIds.push_back (it.second->GetCellId ());
    }
  return cellIds;
}


void
NrGnbNetDevice::SetCellId (uint16_t cellId)
{
  NS_LOG_FUNCTION (this);
  m_cellId = cellId;
}

uint16_t
NrGnbNetDevice::GetBwpId (uint8_t index) const
{
  NS_LOG_FUNCTION (this);
  return m_ccMap.at(index)->GetCellId ();
}

uint16_t
NrGnbNetDevice::GetEarfcn (uint8_t index) const
{
  NS_LOG_FUNCTION (this);
  return m_ccMap.at (index)->GetDlEarfcn (); //Ul or Dl doesn't matter, they are the same

}

void
NrGnbNetDevice::SetRrc (Ptr<LteEnbRrc> rrc)
{
  m_rrc = rrc;
}

Ptr<LteEnbRrc>
NrGnbNetDevice::GetRrc (void)
{
  return m_rrc;
}

bool
NrGnbNetDevice::DoSend (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << dest << protocolNumber);
  NS_ABORT_MSG_IF (protocolNumber != Ipv4L3Protocol::PROT_NUMBER
                   && protocolNumber != Ipv6L3Protocol::PROT_NUMBER,
                   "unsupported protocol " << protocolNumber << ", only IPv4 and IPv6 are supported");
  return m_rrc->SendData (packet);
}

void
NrGnbNetDevice::UpdateConfig (void)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (!m_ccMap.empty ());

  std::map < uint8_t, Ptr<ComponentCarrierBaseStation> > ccPhyConfMap;
  for (auto i:m_ccMap)
    {
      Ptr<ComponentCarrierBaseStation> c = i.second;
      ccPhyConfMap.insert (std::pair<uint8_t, Ptr<ComponentCarrierBaseStation> > (i.first,c));
    }

  m_rrc->ConfigureCell (ccPhyConfMap);

  // test the e2term 

  bool testE2Term = false;

  if (testE2Term){
    // for (uint64_t _imsi = 0; _imsi<10; ++_imsi){
    //   m_ueUsedResourcesMap[_imsi][1] = 10;
    //   m_l3sinrMap[1][_imsi][1].mcs = 10;
    //   m_l3sinrMap[1][_imsi][1].sinr = 100;
    // }
    for (uint16_t _rnti = 0; _rnti<4; ++_rnti){
      NrSlSciF1aHeader nrSlSciF1aHeader = NrSlSciF1aHeader();
      nrSlSciF1aHeader.SetSlMaxNumPerReserve(3);
      nrSlSciF1aHeader.SetGapReTx1(12);
      nrSlSciF1aHeader.SetGapReTx2(11);
      nrSlSciF1aHeader.SetLengthSubChannel(1);
      nrSlSciF1aHeader.SetMcs(1);
      nrSlSciF1aHeader.SetIndexStartSbChReTx1(1);
      nrSlSciF1aHeader.SetIndexStartSbChReTx2(10);
      nrSlSciF1aHeader.SetPriority(1);
      nrSlSciF1aHeader.SetIndexStartSubChannel(7);
      nrSlSciF1aHeader.SetSciStage2Format(0);
      nrSlSciF1aHeader.SetSlResourceReservePeriod(12);
      nrSlSciF1aHeader.SetTotalSubChannels(10);
      
      NrSlMacPduTag nrSlMacPduTag = NrSlMacPduTag(1, SfnSf(1,1,1,1), 1,1,1,1);
      // m_e2SciDataMap[_imsi].push_back(E2SciData(_imsi, _imsi, nrSlSciF1aHeader, nrSlMacPduTag));
      m_e2SciDataMap[_rnti].push_back(E2SciData( (uint64_t)_rnti, _rnti, nrSlSciF1aHeader, nrSlMacPduTag, 10, 10, 10));
    }

    // testing 
    for (uint16_t localRnti = 1; localRnti<6;++localRnti){
      for (uint16_t destRnti = localRnti; destRnti<6;++destRnti){
        // inserting to the vector
        for (uint64_t waiting = 10; waiting<60; waiting+=10){
          m_e2PacketDelaysInBuffer[localRnti][destRnti].m_waitingSincePacket.push_back(waiting);
          m_e2PacketDelaysInBuffer[localRnti][destRnti].m_sequenceNumberPacket.push_back((uint16_t)(waiting/10));
          m_e2PacketDelaysInBuffer[localRnti][destRnti].m_packetSize.push_back(512);
        }
        m_e2PacketDelaysInBuffer[localRnti][destRnti].bufferSizeInBytes = 1500;
        m_e2PacketDelaysInBuffer[localRnti][destRnti].packetNumberInBuffer = 5;
        m_e2PacketDelaysInBuffer[localRnti][destRnti].reservationPeriod = 100;
      }
    }

    E2Termination::RicSubscriptionRequest_rval_s params{};
    Simulator::ScheduleWithContext (1, MilliSeconds(100),
                                    &NrGnbNetDevice::BuildAndSendReportMessage, this, params);
  }

  testE2Term = true;

  if ((m_e2term!=nullptr)&(testE2Term)){
    E2Termination::RicSubscriptionRequest_rval_s params{24, 0, 200, 1};
    Simulator::ScheduleWithContext (1, Seconds(2),
                                    &NrGnbNetDevice::BuildAndSendReportMessage, this, params);
  }


  if(m_e2term!=nullptr){
    NS_LOG_DEBUG("E2sim start in cell " << m_cellId 
      << " force CSV logging " << m_forceE2FileLogging);

    if (!m_forceE2FileLogging)
      {
        // received control messages
        m_ricControlReceivedFilename = m_tracesPath + "ric-control-messages-" + std::to_string(m_cellId) + ".txt";
        std::ofstream csv {};
        csv.open (m_ricControlReceivedFilename.c_str ());
        csv << "Plmn;CellId;SourceImsi;data;sciMessages;Frame;Subframe;Slot;PositionX;PositionY;Ns3Timestamp\n";
        csv.close();
        Simulator::Schedule (MicroSeconds (0), &E2Termination::Start, m_e2term);
      }
  }


}

void 
NrGnbNetDevice::SciReceptionCallback (Ptr<NrGnbNetDevice> gnbNetDevicePtr, //std::string path,
                                  uint64_t imsi, uint16_t rnti,
                                 NrSlSciF1aHeader sciF1Header, NrSlMacPduTag sciTag,
                                 double rsrpdBm, double x, double y){
  gnbNetDevicePtr->SciE2Relay(imsi, rnti, sciF1Header, sciTag, rsrpdBm, x, y);

}
void 
NrGnbNetDevice::SciE2Relay(uint64_t imsi, uint16_t rnti,
                                 NrSlSciF1aHeader sciF1Header, NrSlMacPduTag sciTag,
                                 double rsrpdBm, double x, double y){
  // we store the data we receive in a vector
  // NS_LOG_FUNCTION (this<< imsi << rnti) ;
  // NS_LOG_FUNCTION (this<< imsi << rnti << sciF1Header << sciTag) ;
  m_e2SciDataMap[rnti].push_back(E2SciData(imsi, rnti, sciF1Header, sciTag, rsrpdBm, x, y));
}

void 
NrGnbNetDevice::PacketDelaysInBufferCallback (Ptr<NrGnbNetDevice> gnbNetDevicePtr, //std::string path,
                                              uint16_t rnti, std::map<uint16_t, 
                                              PacketDelayStruct> packetDelaysInBuffer,
                                              double xPosition, double yPosition){
  gnbNetDevicePtr->PacketDelaysInBuffer(rnti, packetDelaysInBuffer, xPosition, yPosition);

}
void 
NrGnbNetDevice::PacketDelaysInBuffer(uint16_t rnti, std::map<uint16_t, 
                                        PacketDelayStruct> packetDelaysInBuffer,
                                        double xPosition, double yPosition){
  // NS_LOG_FUNCTION (this);

  // m_e2PacketDelaysInBuffer[rnti] = packetDelaysInBuffer;
  // adding the new report
  for(std::map<uint16_t, PacketDelayStruct>::iterator mapIt = packetDelaysInBuffer.begin();
      mapIt!=packetDelaysInBuffer.end(); ++mapIt){
        m_e2PacketDelaysInBuffer[rnti][mapIt->first] = mapIt->second;
  }

  m_e2NodePositions[rnti].x = xPosition;
  m_e2NodePositions[rnti].y = yPosition;
}

// std::map<uint16_t, std::map<uint16_t, AggregatePacketDelayReportStruct>>
std::map<uint16_t, std::map<uint16_t, NrGnbNetDevice::DestBufferSize>>
NrGnbNetDevice::AggregateData(std::map<uint16_t, std::map<uint16_t, PacketDelayStruct>>::iterator itBegin,
                                        std::map<uint16_t, std::map<uint16_t, PacketDelayStruct>>::iterator itEnd, 
                                        size_t numOfReports){
  NS_LOG_FUNCTION (this << numOfReports);
  // groupby data for the xapp
  // for each source-dest pair we want to send a report of aggregate delays of packets
  // eg. source: 1, dest: 2 Agg data 0-10ms: 10 packets; 10-20ms 20 packets
  // std::map<uint16_t, std::map<uint16_t, AggregatePacketDelayReportStruct>> mValues;
  std::map<uint16_t, std::map<uint16_t, DestBufferSize>> mValues;
  uint32_t count = 0;
  for(std::map<uint16_t, std::map<uint16_t, PacketDelayStruct>>::iterator userBufferIt = itBegin;
    (count<numOfReports)&&(userBufferIt!=itEnd); ++userBufferIt, ++count){
      // NS_LOG_DEBUG("Aggregate S " << userBufferIt->first);
    for(std::map<uint16_t, PacketDelayStruct>::iterator packetDelaysInBufferIt = userBufferIt->second.begin();
      packetDelaysInBufferIt!=userBufferIt->second.end(); ++packetDelaysInBufferIt){
      // for destination we make the agreggation of data of delays
      // we iterate over the delays present in the vector and apply the grouping function
      // for (uint64_t packetDelay: packetDelaysInBufferIt->second.m_waitingSincePacket){
      //   uint32_t packetDelayGroupingGranularityIndex = (uint32_t)(packetDelay/ m_packetDelayGroupingGranularity);
      //   // if (packetDelayGroupingGranularityIndex>=0){
      //     ++mValues[userBufferIt->first][packetDelaysInBufferIt->first][packetDelayGroupingGranularityIndex].num_packets;
      //   // }
      // }
      // for (uint64_t bufferSize: packetDelaysInBufferIt->second.bufferSizeInBytes){
      //   mValues[userBufferIt->first][packetDelaysInBufferIt->first][packetDelayGroupingGranularityIndex].buffer_size +=bufferSize;
      // }
      // NS_LOG_DEBUG("Agg d " << packetDelaysInBufferIt->first);
      mValues[userBufferIt->first][packetDelaysInBufferIt->first].harqIdBufferSize = packetDelaysInBufferIt->second.harqIdBufferSizeMap;

      uint32_t _packetDelayGroupingGranularity = m_packetDelayGroupingGranularity;
      
      if (packetDelaysInBufferIt->second.m_waitingSincePacket.size()>0){
        _packetDelayGroupingGranularity = (uint32_t) *std::max_element(packetDelaysInBufferIt->second.m_waitingSincePacket.begin(), 
                                                            packetDelaysInBufferIt->second.m_waitingSincePacket.end());
      }
      
      uint32_t _packetDelayGroupingGranularityInd = 0;
      std::map<uint32_t, BufferSize> num_packets_agg_value;
      do
      {
        num_packets_agg_value.clear();
        for (uint32_t _indPacket=0; _indPacket<packetDelaysInBufferIt->second.m_waitingSincePacket.size(); ++_indPacket){
          uint32_t packetDelayGroupingGranularityIndex = (uint32_t)(packetDelaysInBufferIt->second.m_waitingSincePacket[_indPacket]/ _packetDelayGroupingGranularity);
          ++num_packets_agg_value[packetDelayGroupingGranularityIndex].num_packets;
        }
        // increase the periodicity in case we re-iterate
        ++_packetDelayGroupingGranularityInd;
        _packetDelayGroupingGranularity = m_acceptablePeriodicity[_packetDelayGroupingGranularityInd];
      } while ((num_packets_agg_value.size()>4)&(_packetDelayGroupingGranularityInd<m_acceptablePeriodicity.size()));
    
      

      // NS_LOG_DEBUG("The size packet size vector " << packetDelaysInBufferIt->second.m_packetSize.size());
      for (uint32_t _indPacket=0; _indPacket<packetDelaysInBufferIt->second.m_waitingSincePacket.size(); ++_indPacket){
        uint32_t packetDelayGroupingGranularityIndex = (uint32_t)(packetDelaysInBufferIt->second.m_waitingSincePacket[_indPacket]/ _packetDelayGroupingGranularity);
        ++mValues[userBufferIt->first][packetDelaysInBufferIt->first].packetIntervalBufferSize[packetDelayGroupingGranularityIndex].num_packets;
        mValues[userBufferIt->first][packetDelaysInBufferIt->first].packetIntervalBufferSize[packetDelayGroupingGranularityIndex].buffer_size += packetDelaysInBufferIt->second.m_packetSize[_indPacket];
      }
      
    }
  }
  return mValues;
  // afterwards we 
}

}

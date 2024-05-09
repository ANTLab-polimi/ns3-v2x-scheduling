/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2022 Northeastern University
 * Copyright (c) 2022 Sapienza, University of Rome
 * Copyright (c) 2022 University of Padova
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
 * Author: Andrea Lacava <thecave003@gmail.com>
 *		   Tommaso Zugno <tommasozugno@gmail.com>
 *		   Michele Polese <michele.polese@gmail.com>
 */
 
#include <ns3/ric-control-message.h>
#include <ns3/asn1c-types.h>
#include <ns3/log.h>
#include <bitset>

#include "v2x-scheduling-plmn.h"
#include "V2X-Scheduling-All-Users.h" 
#include "all_handovers_plmn.h"
#include "all_handovers.h"

void print_buffer(uint8_t* m_data, uint32_t start, uint32_t length){

    static const char characters[] = "0123456789abcdef";
    std::string ret(length * 2, 0);
    auto bufHex = const_cast<char *>(ret.data());
    for(uint32_t i=start; i<length; ++i){
        *bufHex++ = characters[m_data[i] >> 4];
        *bufHex++ = characters[m_data[i] & 0x0F];
    }
    std::cout << "REturn " << ret << std::endl;
}


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RicControlMessage");


RicControlMessage::RicControlMessage (E2AP_PDU_t* pdu)
{
  DecodeRicControlMessage (pdu);
//   NS_LOG_INFO ("End of RicControlMessage::RicControlMessage()");
}

RicControlMessage::RicControlMessage (E2AP_PDU_t* pdu, uint8_t* remainingDataPtr, uint32_t remainingDataSize)
{
    NS_LOG_FUNCTION(this << pdu << remainingDataPtr << remainingDataSize);
    m_remaining_bytes = remainingDataSize;
    m_gndBufferPtr = remainingDataPtr;

    m_data = (uint8_t*) calloc(10000, sizeof(uint8_t));
    // copy remaining data to local buffer
    std::memcpy(m_data, remainingDataPtr, remainingDataSize);
    DecodeRicControlMessage (pdu);
}

RicControlMessage::~RicControlMessage ()
{

}

void  
RicControlMessage::DecodeRicControlMessage(E2AP_PDU_t* pdu)
{
    InitiatingMessage_t* mess = pdu->choice.initiatingMessage;
    auto *request = (RICcontrolRequest_t *) &mess->value.choice.RICcontrolRequest;
    // NS_LOG_INFO (xer_fprint(stderr, &asn_DEF_RICcontrolRequest, request));

    size_t count = request->protocolIEs.list.count; 
    if (count <= 0) {
        NS_LOG_ERROR("[E2SM] received empty list");
        return;
    }

    for (size_t i = 0; i < count; i++) 
    {
        RICcontrolRequest_IEs_t *ie = request->protocolIEs.list.array [i];
        switch (ie->value.present) {
            case RICcontrolRequest_IEs__value_PR_RICrequestID: {
                NS_LOG_DEBUG("[E2SM] RICcontrolRequest_IEs__value_PR_RICrequestID");
                m_ricRequestId = ie->value.choice.RICrequestID;
                switch (m_ricRequestId.ricRequestorID) {
                    case 1001: {
                        NS_LOG_DEBUG("TS xApp message");
                        m_requestType = ControlMessageRequestIdType::TS;
                        break;
                    }
                    case 1002: {
                        NS_LOG_DEBUG("QoS xApp message");
                        m_requestType = ControlMessageRequestIdType::QoS;
                        break;
                    }
                }
                break;
            }
            case RICcontrolRequest_IEs__value_PR_RANfunctionID: {
                m_ranFunctionId = ie->value.choice.RANfunctionID;

                NS_LOG_DEBUG("[E2SM] RICcontrolRequest_IEs__value_PR_RANfunctionID");
                break;
            }
            case RICcontrolRequest_IEs__value_PR_RICcallProcessID: {
                m_ricCallProcessId = ie->value.choice.RICcallProcessID;
                NS_LOG_DEBUG("[E2SM] RICcontrolRequest_IEs__value_PR_RICcallProcessID");
                break;
            }
            case RICcontrolRequest_IEs__value_PR_RICcontrolHeader: {
                NS_LOG_DEBUG("[E2SM] RICcontrolRequest_IEs__value_PR_RICcontrolHeader");
                // xer_fprint(stderr, &asn_DEF_RICcontrolHeader, &ie->value.choice.RICcontrolHeader);

                auto *e2smControlHeader = (E2SM_RC_ControlHeader_t *) calloc(1,
                                                                             sizeof(E2SM_RC_ControlHeader_t));
                ASN_STRUCT_RESET(asn_DEF_E2SM_RC_ControlHeader, e2smControlHeader);
                asn_decode (nullptr, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2SM_RC_ControlHeader,
                            (void **) &e2smControlHeader, ie->value.choice.RICcontrolHeader.buf,
                            ie->value.choice.RICcontrolHeader.size);

                // NS_LOG_INFO (xer_fprint (stderr, &asn_DEF_E2SM_RC_ControlHeader, e2smControlHeader));
                if (e2smControlHeader->present == E2SM_RC_ControlHeader_PR_controlHeader_Format1) {
                    m_e2SmRcControlHeaderFormat1 = e2smControlHeader->choice.controlHeader_Format1;
                    //m_e2SmRcControlHeaderFormat1->ric_ControlAction_ID;
                    //m_e2SmRcControlHeaderFormat1->ric_ControlStyle_Type;
                    //m_e2SmRcControlHeaderFormat1->ueId;
                } else {
                    NS_LOG_DEBUG("[E2SM] Error in checking format of E2SM Control Header");
                }
                break;
            }
            case RICcontrolRequest_IEs__value_PR_RICcontrolMessage: {
                NS_LOG_DEBUG("[E2SM] RICcontrolRequest_IEs__value_PR_RICcontrolMessage");
                // xer_fprint(stderr, &asn_DEF_RICcontrolMessage, &ie->value.choice.RICcontrolMessage);
                // the bytes consumed due to multiple ric messages
                uint32_t consumed = 0;
                uint32_t msg_size = (uint32_t)ie->value.choice.RICcontrolMessage.size;

                NS_LOG_DEBUG("RICcontrolMessage size " << msg_size);

                uint8_t* ricControlMessageBuf;
                if (m_remaining_bytes == 0){
                    ricControlMessageBuf = ie->value.choice.RICcontrolMessage.buf;
                }else{
                    // we have to add the buffer remaining from prior messages
                    // start coping from the index where last buffer ended to have continuous data
                    std::memcpy(m_data + m_remaining_bytes, ie->value.choice.RICcontrolMessage.buf, ie->value.choice.RICcontrolMessage.size);
                    ricControlMessageBuf = m_data;
                    msg_size = (uint32_t)ie->value.choice.RICcontrolMessage.size + m_remaining_bytes;

                    // print_buffer(ricControlMessageBuf, 0, msg_size);
                }
                 
                bool incomplete_msg = false;
                do{
                    asn_dec_rval_t dec_ret;

                    auto *e2SmControlMessage = (E2SM_RC_ControlMessage_t *) calloc(1,
                                                                                sizeof(E2SM_RC_ControlMessage_t));
                    ASN_STRUCT_RESET(asn_DEF_E2SM_RC_ControlMessage, e2SmControlMessage);

                    dec_ret = asn_decode (nullptr, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2SM_RC_ControlMessage,
                                (void **) &e2SmControlMessage, ricControlMessageBuf + consumed,
                                msg_size);

                    NS_LOG_DEBUG("RICcontrolMessage bytes consumed " << consumed
                                << " remaining " << m_remaining_bytes
                                << " msg size " << msg_size); 
                    
                    NS_LOG_INFO (xer_fprint(stderr, &asn_DEF_E2SM_RC_ControlMessage, e2SmControlMessage));

                    if ((consumed + dec_ret.consumed) > msg_size){
                        // might happen that due to buffer redudency we decode 
                        // message from remainings of previous buffer, thus we 
                        // ignore this message and wait for the next message to come
                        // update the gnb buffer before breaking from the while loop
                        std::memcpy(m_gndBufferPtr, ricControlMessageBuf+consumed, m_remaining_bytes);
                        break;
                    }

                    consumed += dec_ret.consumed;

                    m_remaining_bytes = msg_size - consumed;

                    if (dec_ret.code == RC_OK){
                        NS_LOG_DEBUG("check the eq " << ((consumed < msg_size) && (!incomplete_msg)));

                        // NS_LOG_INFO (xer_fprint(stderr, &asn_DEF_E2SM_RC_ControlMessage, e2SmControlMessage));

                        if (e2SmControlMessage->present == E2SM_RC_ControlMessage_PR_controlMessage_Format1)
                        {
                            // NS_LOG_DEBUG ("[E2SM] E2SM_RC_ControlMessage_PR_controlMessage_Format1");
                            m_messageFormatType = ControlMessage_PR_controlMessage_Format1;

                            E2SM_RC_ControlMessage_Format1_t *e2SmRcControlMessageFormat1 =
                                e2SmControlMessage->choice.controlMessage_Format1;
                            m_valuesExtracted =
                                ExtractRANParametersFromControlMessage (e2SmRcControlMessageFormat1);
                            if (m_requestType == ControlMessageRequestIdType::TS)
                            {
                                // Get and parse the secondaty cell id according to 3GPP TS 38.473, Section 9.2.2.1
                                for (RANParameterItem item : m_valuesExtracted)
                                {
                                    if (item.m_valueType == RANParameterItem::ValueType::OctectString)
                                    {
                                        // First 3 digits are the PLMNID (always 111), last digit is CellId
                                        std::string cgi = item.m_valueStr->DecodeContent ();
                                        // NS_LOG_INFO ("Decoded CGI value is: " << cgi);
                                        m_secondaryCellId = cgi.back();
                                    }
                                }         
                            }
                        } 
                        // modified 
                        else if(e2SmControlMessage->present == E2SM_RC_ControlMessage_PR_handoverMessage_Format){
                            NS_LOG_DEBUG("[E2SM] Handover msg received");
                            m_messageFormatType = ControlMessage_PR_handoverMessage_Format;
                            // AllHandoversList_t *handoverMessage_Format = 
                            //     e2SmControlMessage->choice.handoverMessage_Format;
                            AllHandoversListPlmn_t *handoverMessagePlmn_Format = 
                                e2SmControlMessage->choice.handoverMessage_Format;

                            uint32_t plmnIdSize = handoverMessagePlmn_Format->plmn_id.size;
                            // NS_LOG_DEBUG("Size " << (uint32_t)plmnIdSize);
                            // char buf[plmnIdSize];
                            // uint8_t *buf = (uint8_t *)calloc(1,plmnIdSize);
                            // memcpy(buf, handoverMessagePlmn_Format->plmn_id.buf, plmnIdSize);

                            m_plmnString.assign((const char *)handoverMessagePlmn_Format->plmn_id.buf, plmnIdSize);
                            // std::string plmnString (&buf[0]);
                            // m_plmnString = plmnString;
                            // NS_LOG_DEBUG("Ric control " << m_plmnString << " ");
                            
                            AllHandoversList_t *handoverMessage_Format = handoverMessagePlmn_Format->allHandoversList;
                            m_allHandoverList = encoding::extract_handover_map(handoverMessage_Format);
                        }
                        
                        else if(e2SmControlMessage->present == E2SM_RC_ControlMessage_PR_v2xSchedulingMessage_Format){
                            // the ric control message regarding the scheduling of users
                            // NS_LOG_DEBUG ("[E2SM] E2SM_RC_ControlMessage_PR_v2xSchedulingMessage_Format");
                            m_messageFormatType = ControlMessage_PR_v2xScheduling_Format;
                            V2X_Scheduling_All_UsersPlmn_t* v2xSchedulingPlmnMessage_Format = 
                            e2SmControlMessage->choice.v2xSchedulingMessage_Format;

                            uint32_t plmnIdSize = v2xSchedulingPlmnMessage_Format->plmn_id.size;
                            m_plmnString.assign((const char *)v2xSchedulingPlmnMessage_Format->plmn_id.buf, plmnIdSize);

                            

                            V2X_Scheduling_All_Users_t *v2XSchedulingMessage_Format = v2xSchedulingPlmnMessage_Format->v2XSchedulingAllUsersList;
                            
                            NS_LOG_DEBUG("MEssage plmnd " << m_plmnString<< " count " << (uint32_t) v2XSchedulingMessage_Format->list.count);
                            if (v2XSchedulingMessage_Format->list.count == 0){
                                // it means this is the last sent report,
                                // we set continue simulation to true
                                m_lastReport = true;
                            }
                            // NS_LOG_DEBUG ("[E2SM] Extract scheduling map");
                            m_schedulingList = encoding::extract_scheduling_map(v2XSchedulingMessage_Format);

                            SchedulingSingleMsg singleMsg;
                            singleMsg.scheduleList = m_schedulingList;
                            singleMsg.plmnString = m_plmnString;
                            singleMsg.messageFormatType = ControlMessage_PR_v2xScheduling_Format;
                            m_schedulingListMulti.push_back(singleMsg);
                        }
                        // end modification
                        else
                        {
                            NS_LOG_DEBUG("[E2SM] Error in checking format of E2SM Control Message");
                        }
                        // break;
                    }else{
                        // we store the rest in a local buffer for the next coming message
                        // m_remaining_bytes = msg_size - consumed;
                        // update gnb buffer for the next coming message
                        std::memcpy(m_gndBufferPtr, ricControlMessageBuf+consumed, m_remaining_bytes);
                        incomplete_msg = true;
                        // print_buffer(ricControlMessageBuf, consumed, m_remaining_bytes);
                        NS_LOG_DEBUG ("RICcontrolMessage cannot decode msg: consumed " << consumed 
                                    << " remaining " << m_remaining_bytes << " msg size " << msg_size);
                        // print_buffer(m_gndBufferPtr, 0, m_remaining_bytes);
                    }
                }while((consumed<msg_size) && (!incomplete_msg));
            }
            case RICcontrolRequest_IEs__value_PR_RICcontrolAckRequest: {
                NS_LOG_DEBUG("[E2SM] RICcontrolRequest_IEs__value_PR_RICcontrolAckRequest");

                switch (ie->value.choice.RICcontrolAckRequest) {
                    case RICcontrolAckRequest_noAck: {
                        NS_LOG_DEBUG("[E2SM] RIC Control ack value: NO ACK");
                        break;
                    }
                    case RICcontrolAckRequest_ack: {
                        NS_LOG_DEBUG("[E2SM] RIC Control ack value: ACK");
                        break;
                    }
                    case RICcontrolAckRequest_nAck: {
                        NS_LOG_DEBUG("[E2SM] RIC Control ack value: NACK");
                        break;
                    }
                    default: {
                        NS_LOG_DEBUG("[E2SM] RIC Control ack value unknown");
                        break;
                    }
                }
                break;
            }
            case RICcontrolRequest_IEs__value_PR_NOTHING: {
                NS_LOG_DEBUG("[E2SM] RICcontrolRequest_IEs__value_PR_NOTHING");
                NS_LOG_DEBUG("[E2SM] Nothing");
                break;
            }
            default: {
                NS_LOG_DEBUG("[E2SM] RIC Control value unknown");
                break;
            }
        }
    }
}

std::string
RicControlMessage::GetSecondaryCellIdHO ()
{
  return m_secondaryCellId;
}

std::vector<RANParameterItem>
RicControlMessage::ExtractRANParametersFromControlMessage (
    E2SM_RC_ControlMessage_Format1_t *e2SmRcControlMessageFormat1)
{
  std::vector<RANParameterItem> ranParameterList;
  int count = e2SmRcControlMessageFormat1->ranParameters_List->list.count;
  for (int i = 0; i < count; i++)
    {
      RANParameter_Item_t *ranParameterItem =
          e2SmRcControlMessageFormat1->ranParameters_List->list.array[i];
      for (RANParameterItem extractedParameter :
           RANParameterItem::ExtractRANParametersFromRANParameter (ranParameterItem))
        {
          ranParameterList.push_back (extractedParameter);
        }
    }

  return ranParameterList;
}

} // namespace ns3

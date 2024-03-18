#include "ns3/core-module.h"
#include "ns3/oran-interface.h"

// extern "C" {
  // #include "OCUCP-PF-Container.h"
  #include "asn_application.h"
// #include "E2SM-KPM-IndicationMessage.h"
// #include "FQIPERSlicesPerPlmnListItem.h"
// #include "E2SM-KPM-RANfunction-Description.h"
// #include "E2SM-KPM-IndicationHeader-Format1.h"
// #include "E2SM-KPM-IndicationHeader.h"
// #include "Timestamp.h"
#include "E2AP-PDU.h"
#include "ProtocolIE-SingleContainer.h"
#include "InitiatingMessage.h"
#include "E2SM-RC-ControlMessage-Format1.h"


#include "control_message_encoder_decoder.h"
#include "all_handovers.h"
#include "all_handovers_plmn.h"
// }

#include "e2sim.hpp"

using namespace ns3;

int 
main (int argc, char *argv[])
{

    std::vector<long> ue_id_vec {1,2,3,4,5};
    std::vector<long> start_position_vec {1,2,3,1,2};
    std::vector<long> optimized_vec {2,1,4,3,5};
    // for(int _ind = 0; _ind<(int)size; ++_ind){
    //     ue_id_vec[_ind] = (ue_id[_ind]);
    //     start_position_vec[_ind] = (start_position[_ind]);
    //     optimized_vec[_ind] = (optimized[_ind]);
    // }

    std::set<long> sourceCellIdSet;
    for (long x: start_position_vec){
        sourceCellIdSet.insert(x);
    }

    std::string plmn("111");
    std::cout<< "Plmn " << plmn << std::endl;

    AllHandoversListPlmn_t* allHandoversListPlmn = (AllHandoversListPlmn_t *) calloc(1, sizeof(AllHandoversListPlmn_t));

    AllHandoversList_t* allHandoversList = (AllHandoversList_t *) calloc(1, sizeof(AllHandoversList_t));
    allHandoversListPlmn->plmn_id.buf = (uint8_t *) calloc (1, 3);
    allHandoversListPlmn->plmn_id.size = 3;
    memcpy (allHandoversListPlmn->plmn_id.buf, plmn.c_str (), 3);

    allHandoversListPlmn->allHandoversList = allHandoversList;

    for (long sourceCellId: sourceCellIdSet){
        CellHandoverList_t* cellHandovers = (CellHandoverList_t *) calloc(1, sizeof(CellHandoverList_t));
        cellHandovers->sourceCellId = sourceCellId;
        // find items in the starting vec from the set
        std::vector<int> indices = encoding::findItems(start_position_vec, sourceCellId);
        std::list<CellHandoverItem_t*> handoverItems;
        for (int index : indices){
            long _ue_ind = ue_id_vec.at(index);
            long _dst_cell_id = optimized_vec.at(index);
            CellHandoverItem_t* control_message = encoding::create_handover_item(_ue_ind, _dst_cell_id);
            handoverItems.push_back(control_message);
        }
        CellHandoverItemList_t* cellHandoverList = encoding::create_handover_item_list(handoverItems);
        cellHandovers->cellHandoverItemList = cellHandoverList;
        ASN_SEQUENCE_ADD(&allHandoversList->list, cellHandovers);
    }

    E2SM_RC_ControlMessage_t* rcControlMessage = (E2SM_RC_ControlMessage_t *) calloc(1, sizeof(E2SM_RC_ControlMessage_t));
    rcControlMessage->present = E2SM_RC_ControlMessage_PR_handoverMessage_Format;
    // rcControlMessage->choice.handoverMessage_Format = allHandoversList;
    rcControlMessage->choice.handoverMessage_Format = allHandoversListPlmn;

    // uint8_t* buf;
    uint32_t _MAX_SCTP_BUFFER_CTRL = 1024;
    unsigned char buf[_MAX_SCTP_BUFFER_CTRL];
    memset(buf, 0, _MAX_SCTP_BUFFER_CTRL);
    size_t len = 0;

    auto er = asn_encode_to_buffer(nullptr, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2SM_RC_ControlMessage, rcControlMessage, buf, _MAX_SCTP_BUFFER_CTRL);
    len = er.encoded;
    // int len = aper_encode_to_new_buffer(&asn_DEF_E2SM_RC_ControlMessage, 0, rcControlMessage, (void **)&buf);

    RICcontrolRequest_IEs_t* singleRequest = (RICcontrolRequest_IEs_t *)calloc(1, sizeof(RICcontrolRequest_IEs_t));
    singleRequest->value.present = RICcontrolRequest_IEs__value_PR_RICcontrolMessage;
    singleRequest->value.choice.RICcontrolMessage.buf = buf;
    singleRequest->value.choice.RICcontrolMessage.size = len;

    // RICcontrolRequest_t * ric_control_request = (RICcontrolRequest_t * )calloc(1, sizeof(RICcontrolRequest_t));
    // ASN_SEQUENCE_ADD(&(ric_control_request->protocolIEs.list), singleRequest);    

    InitiatingMessage_t* initMsg = (InitiatingMessage_t * )calloc(1, sizeof(InitiatingMessage_t));
    initMsg->value.present = InitiatingMessage__value_PR_RICcontrolRequest;
    ASN_SEQUENCE_ADD(&(initMsg->value.choice.RICcontrolRequest.protocolIEs.list) , singleRequest);    


    E2AP_PDU_t* pdu = (E2AP_PDU_t * )calloc(1, sizeof(E2AP_PDU_t));
    pdu->present = E2AP_PDU_PR_initiatingMessage;
    pdu->choice.initiatingMessage = initMsg;
    
    // NS_LOG_INFO ();
    // 
    xer_fprint(stderr, &asn_DEF_E2AP_PDU, pdu);

    xer_fprint(stderr, &asn_DEF_E2SM_RC_ControlMessage, rcControlMessage);

    // reverse the encoding of e2sm control msg

    unsigned char new_buf[len];
    memcpy(new_buf, buf, len);

    std::cout << "Len og buff " << len << std::endl;
    
    E2SM_RC_ControlMessage_t* rcNewControlMessage = (E2SM_RC_ControlMessage_t *) calloc(1,
                                                                             sizeof(E2SM_RC_ControlMessage_t));
    ASN_STRUCT_RESET(asn_DEF_E2SM_RC_ControlMessage, rcNewControlMessage);
    asn_dec_rval_t dec_ret = asn_decode(nullptr, ATS_ALIGNED_BASIC_PER, &asn_DEF_E2SM_RC_ControlMessage, (void **) &rcNewControlMessage, new_buf, len);
    xer_fprint(stderr, &asn_DEF_E2SM_RC_ControlMessage, rcNewControlMessage);
    // if (dec_ret.code != RC_OK) {
    //     std::cout << "Error happening" << std::endl;
    // } else {
    //     xer_fprint(stderr, &asn_DEF_E2SM_RC_ControlMessage, rcNewControlMessage);
    // }

}
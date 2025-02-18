/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "E2SM-KPM-RC"
 * 	found in "e2sm-kpm-rc.asn"
 * 	`asn1c -fcompound-names -fno-include-deps -findirect-choice -pdu=auto -gen-PER -gen-OER -no-gen-example -D .`
 */

#include "handover_item.h"

#include <NativeInteger.h>
// #include "constr_TYPE.h"
// #include <asn1c-types.h>
#include <per_support.h>
namespace ns3{
namespace ric_control {

asn_TYPE_member_t asn_MBR_CellHandoverItem_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct CellHandoverItem, ueId),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		+1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{0,0,0}, 
		// { &asn_OER_memb_CellHandoverItem_constr_2, &asn_PER_memb_CellHandoverItem_constr_2, 0 },
		0, 0, /* No default value */
		"ueId"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct CellHandoverItem, destinationCellId),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		+1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{0,0,0},
		// { &asn_OER_memb_CellHandoverItem_constr_3, &asn_PER_memb_CellHandoverItem_constr_3, 0 },
		0, 0, /* No default value */
		"destinationCellId"
		},
};

static const ber_tlv_tag_t asn_DEF_CellHandoverItem_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};

static const asn_TYPE_tag2member_t asn_MAP_CellHandoverItem_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* ueID */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* destCellID */
};

asn_SEQUENCE_specifics_t asn_SPC_CellHandoverItem_specs_1 = {
	sizeof(struct CellHandoverItem),
	offsetof(struct CellHandoverItem, _asn_ctx),
	asn_MAP_CellHandoverItem_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0,	/* Optional elements (not needed) */
	2,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_CellHandoverItem= {
	"CellHandoverItem",
	"CellHandoverItem",
	&asn_OP_SEQUENCE,
	asn_DEF_CellHandoverItem_tags_1,
	sizeof(asn_DEF_CellHandoverItem_tags_1)
		/sizeof(asn_DEF_CellHandoverItem_tags_1[0]), /* 1 */
	asn_DEF_CellHandoverItem_tags_1,	/* Same as above */
	sizeof(asn_DEF_CellHandoverItem_tags_1)
		/sizeof(asn_DEF_CellHandoverItem_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_CellHandoverItem_1,
	2,	/* Elements count */
	&asn_SPC_CellHandoverItem_specs_1	/* Additional specs */
};
}
}
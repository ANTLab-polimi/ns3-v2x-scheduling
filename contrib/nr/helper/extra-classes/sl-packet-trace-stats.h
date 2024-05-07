/*
 * ef-packet-trace.h
 *
 *  Created on: Apr 5, 2022
 *      Author: ugjeci
 */

#ifndef SL_CONTRIB_HELPER_PACKET_TRACE_STATS_H_
#define SL_CONTRIB_HELPER_PACKET_TRACE_STATS_H_

#include <inttypes.h>
#include <vector>

#include <ns3/sqlite-output.h>
//#include <ns3/sfnsf.h>
//#include <ns3/nstime.h>
#include <ns3/nr-phy-mac-common.h>

namespace ns3 {
/**
 * \brief Class to collect and store the packet trace stats obtained from a simulation
 *
 * The class is meant to store in a database the values from UE or GNB during
 * a simulation. The class contains a cache, that after some time is written
 * to the disk.
 *
 * \see SetDb
 * \see SavePacketTrace
 * \see EmptyCache
 */
class SlPacketTraceStats
{
public:
  /**
   * \brief Constructor
   */
	SlPacketTraceStats ();

  /**
   * \brief Install the output dabase.
   * \param db database pointer
   * \param tableName name of the table where the values will be stored
   *
   * The db pointer must be valid through all the lifespan of the class. The
   * method creates, if not exists, a table for storing the values. The table
   * will contain the following columns:
   *
   * - "Frame INTEGER NOT NULL, "
   * - "SubFrame INTEGER NOT NULL,"
   * - "Slot INTEGER NOT NULL,"
   * - "Rnti INTEGER NOT NULL,"
   * - "Imsi INTEGER NOT NULL,"
   * - "BwpId INTEGER NOT NULL,"
   * - "CellId INTEGER NOT NULL,"
   * - "txPsdSum DOUBLE NOT NULL,"
   * - "Seed INTEGER NOT NULL,"
   * - "Run INTEGER NOT NULL);"
   *
   * Please note that this method, if the db already contains a table with
   * the same name, also clean existing values that has the same
   * Seed/Run pair.
   */
  void SetDb (SQLiteOutput *db, const std::string& tableName = "packetTrace", uint32_t writeSize = 100000);

  /**
   * \brief Store power values
   * \param sfnSf Slot number
   * \param txPsd TxPsd
   * \param t Time for the transmission
   * \param rnti RNTI
   * \param imsi IMSI
   * \param bwpId BWP ID
   * \param cellId cell ID
   *
   * Please note that the values in txPsd will be summed before storing.
   */
  void SavePacketTrace (std::string direction, RxPacketTraceParams params);

  /**
   * \brief Force the cache write to disk, emptying the cache itself.
   */
  void EmptyCache ();

private:

  static void
  DeleteWhere (SQLiteOutput* p, uint32_t seed, uint32_t run, const std::string &table);

  void WriteCache ();

  struct SlPacketTraceCache
    {
  	std::string direction;
//	  uint32_t direction;
	  RxPacketTraceParams packetTrace;
      Time timeInstance;
    };

  SQLiteOutput *m_db;                           //!< DB pointer
  std::vector<SlPacketTraceCache> m_packetTraceCache;   //!< Result cache
  std::string m_tableName;                      //!< Table name
  uint32_t m_writeSize;
};

}



#endif /* CONTRIB_ELEPHANT_FLOW_HELPER_EF_PACKET_TRACE_STATS_H_ */

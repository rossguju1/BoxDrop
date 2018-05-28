/*

 CS60 - DropBox Project

 tracker.h

*/

#ifndef TRACKER_H
#define TRACKER_H


void *tracker_hand_shake();


void *monitor_peers();

void tracker_recieve(tracker_table_t *table, seg_t *segmentBuffer);

void tracker_send(tracker_table_t *table, );

int connect_to_peer();

#endif //TRACKER_H
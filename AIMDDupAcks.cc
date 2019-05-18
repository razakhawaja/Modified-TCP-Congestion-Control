#include <iostream>
#include <inttypes.h>
#include <cmath>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;


double my_window_size = 25.0;
unsigned int ssthresh = 1000000;
bool slow_start_phase = true;
unsigned int succ_timeouts = 0;
double succ_timeouts_thresh = my_window_size / 4.0;
double my_timeout = 100;
double est_rtt = 100;
double dev_rtt = 1;
uint64_t last_seq_num = 0;
int dup_ack_count = 0;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size()
{
  /* Default: fixed window size of 100 outstanding datagrams */
  //unsigned int the_window_size = 50;

	double new_window_size = my_window_size;
	if (new_window_size < 1.0)
	{
		new_window_size = 1.0;
	}

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << new_window_size << endl;
  }

  return new_window_size;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp,
                                    /* in milliseconds */
				    const bool after_timeout
				    /* datagram was sent because of a timeout */ )
{
  /* Default: take no action */

  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << " (timeout = " << after_timeout << ")\n";
  }


}

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
  /* Default: take no action */

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }

   //printf("%" PRIu64 "\n", sequence_number_acked);
  uint64_t rtt = timestamp_ack_received - send_timestamp_acked;

  if (sequence_number_acked == last_seq_num)
  {
    cout << "HIT" << endl;
    dup_ack_count++;
  }
  else
  {
    last_seq_num = sequence_number_acked;
    dup_ack_count = 0;
  }

  if(rtt > timeout_ms())
  {
  		ssthresh = my_window_size * 0.75;
  		my_window_size = my_window_size / 2.0;
  		slow_start_phase = true;

  }
  else
  {
  	if (slow_start_phase)
  	{
  		my_window_size += 2;
  		if (my_window_size >= ssthresh)
  		{
  			slow_start_phase = false;
  			my_window_size = 3 * my_window_size / 4.0;
  			if (my_window_size < 1.0) 
  			{
  				my_window_size = 1.0;
  			}
  		}
  	}
  	else
  	{
      if (dup_ack_count >= 3)
      {
        my_window_size = my_window_size * 0.75;
        dup_ack_count = 0;
      }
      else
      {
        my_window_size = my_window_size + (2.0/my_window_size);
      }
  		
  	}
  	
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms()
{
  return (unsigned int) my_timeout; /* timeout of one second */
}

#include <iostream>
#include <inttypes.h>
#include <cmath>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

const double alpha = 0.25;
const double rho = 0.20;
const double dev_factor = 0.25;
const double desired_delay = 60;
const double slot_length = 30;

double my_window_size = 12.0;
double my_timeout = 100;
double ack_count = 0;
uint64_t last_interval_start = 0;
double estimated_throughput = 0;
double dev_throughput = 0;

bool slow_start = false;
double ssthresh = 100000;

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

  return (unsigned int) new_window_size;
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

  uint64_t rtt = timestamp_ack_received - send_timestamp_acked;

  if (rtt > 2.0 * timeout_ms())
  {
    ssthresh = my_window_size;
    my_window_size = my_window_size * 0.65;
    if (my_window_size < 1.0)
    {
      my_window_size = 1.0;
    }
    slow_start = true;
  }

  if (slow_start)
  {
    my_window_size += 2.0;
    if (my_window_size >= ssthresh)
    {
      my_window_size = my_window_size * 0.75;
      slow_start = false;
    }
  }

  if ((timestamp_ack_received - last_interval_start) >= slot_length)
  {

  	double observed_throughput = ack_count / slot_length;


  	dev_throughput = (1 - rho) * dev_throughput + (rho)*fabs(observed_throughput - estimated_throughput);
  	estimated_throughput = (1-alpha) * estimated_throughput + (alpha) * observed_throughput;


  	ack_count = 0;
  	last_interval_start = timestamp_ack_received;


    if (!slow_start)
    {
      my_window_size = (estimated_throughput - dev_factor * dev_throughput) * desired_delay;
    }

  }

   ack_count++;

}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms()
{
  return (unsigned int) my_timeout; /* timeout of one second */
}

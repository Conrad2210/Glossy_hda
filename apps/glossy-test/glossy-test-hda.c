/*
 * Copyright (c) 2011, ETH Zurich.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Author: Federico Ferrari <ferrari@tik.ee.ethz.ch>
 *
 */

/**
 *
 * \mainpage
 *           These files document the source code of Glossy, a flooding architecture for wireless sensor networks,
 *           implemented in <a href="http://www.sics.se/contiki/">Contiki</a> based on Tmote Sky sensor nodes.
 *
 *           Glossy was published at ACM/IEEE IPSN '11 in the paper titled
 *           <a href="ftp://ftp.tik.ee.ethz.ch/pub/people/ferrarif/FZTS2011.pdf">
 *           Efficient network flooding and time synchronization with Glossy</a>,
 *           which also received the best paper award.
 *
 *           This documentation is divided into three main parts:
 *           \li \ref glossy-test "Simple application for testing Glossy":
 *           Example of a simple application that periodically floods a packet and prints related statistics.
 *           \li \ref glossy_interface "Glossy API":
 *           API provided by Glossy for an application that wants to use it.
 *           \li \ref glossy_internal "Glossy internal functions":
 *           Functions used internally by Glossy during a flood.
 *
 *           A complete overview of the documentation structure is available <a href="modules.html">here</a>.
 *
 * \author
 *           <a href="http://www.tik.ee.ethz.ch/~ferrarif">Federico Ferrari</a> <ferrari@tik.ee.ethz.ch>
 *
 */

/**
 * \defgroup glossy-test Simple application for testing Glossy
 *
 *           This application runs Glossy periodically to flood a packet from one node (initiator)
 *           to the other nodes (receivers) and prints flooding-related statistics.
 *
 *           The application schedules Glossy periodically with a fixed period \link GLOSSY_PERIOD \endlink.
 *
 *           The duration of each Glossy phase is given by \link GLOSSY_DURATION \endlink.
 *
 *           During each Glossy phase, the maximum number of transmissions in Glossy (N)
 *           is set to \link N_TX \endlink.
 *
 *           The initiator of the floods is the node having nodeId \link INITIATOR_NODE_ID \endlink.
 *
 *           The packet to be flooded has the format specified by data structure \link glossy_data_struct \endlink.
 *
 *           Receivers synchronize by computing the reference time during each Glossy phase.
 *
 *           To synchronize fast, at startup receivers run Glossy with a significantly shorter period
 *           (\link GLOSSY_INIT_PERIOD \endlink) and longer duration (\link GLOSSY_INIT_DURATION \endlink).
 *
 *           Receivers exit the bootstrapping phase when they have computed the reference time for
 *           \link GLOSSY_BOOTSTRAP_PERIODS \endlink consecutive Glossy phases.
 *
 * @{
 */

/**
 * \file
 *         A simple example of an application that uses Glossy, source file.
 *
 * \author
 *         Federico Ferrari <ferrari@tik.ee.ethz.ch>
 */

#include "glossy-test-hda.h"

/**
 * \defgroup glossy-test-variables Application variables
 * @{
 */

/**
 * \defgroup glossy-test-variables-sched-sync Scheduling and synchronization variables
 * @{
 */

glossy_data_struct glossy_data; /**< \brief Flooding data. */
static struct rtimer rt; /**< \brief Rtimer used to schedule Glossy. */
static struct etimer et_traffic,et_traffic_period;
static struct pt pt; /**< \brief Protothread used to schedule Glossy. */
static rtimer_clock_t t_ref_l_old = 0; /**< \brief Reference time computed from the Glossy
 phase before the last one. \sa get_t_ref_l */
static uint8_t skew_estimated = 0; /**< \brief Not zero if the clock skew over a period of length
 \link GLOSSY_PERIOD \endlink has already been estimated. */
static uint8_t sync_missed = 0; /**< \brief Current number of consecutive phases without
 synchronization (reference time not computed). */
static rtimer_clock_t t_start = 0; /**< \brief Starting time (low-frequency clock)
 of the last Glossy phase. */
static int period_skew = 0; /**< \brief Current estimation of clock skew over a period
 of length \link GLOSSY_PERIOD \endlink. */

static rtimer_clock_t t_start_queue = 0;

glossy_data_struct random_traffic_data;

/** @} */

/**
 * \defgroup glossy-test-variables-stats Statistics variables
 * @{
 */

static unsigned long packets_received = 0; /**< \brief Current number of received packets. */
static unsigned long packets_missed = 0; /**< \brief Current number of missed packets. */
static unsigned long latency = 0; /**< \brief Latency of last Glossy phase, in us. */
static unsigned long sum_latency = 0; /**< \brief Current sum of latencies, in ticks of low-frequency
 clock (used to compute average). */

struct queueCDT myQueue;

unsigned int traffic_period = 4;
unsigned int k = 0;
int count = 0;
int K = 4;
int seg_no = 0;

/** @} */
/** @} */

/**
 * \defgroup glossy-test-processes Application processes and functions
 * @{
 */

/**
 * \defgroup glossy-test-print-stats Print statistics information
 * @{
 */

PROCESS(glossy_test, "Glossy test");
PROCESS(glossy_print_stats_process, "Glossy print stats");
PROCESS(queue_init, "Glossy init queue");
PROCESS(random_traffic_process, "Glossy init queue");
PROCESS(set_traffic_period, "Glossy init queue");

AUTOSTART_PROCESSES(&glossy_test);



////////////////////////////////////////
///		QUEUE FUNCTIONS//////////////////////
/////////////////////////////////////

void QueueInit(queueCDT* queueADT)
{
  int i = 0;
  queueADT->rear = 0;
  queueADT->front = 0;
  queueADT->count = 0;
  for(int i = 0; i < MAX_QUEUE_SIZE; i++)
	  queueADT->glossy_data[i].seq_no = -1;
}

 void Enqueue(queueCDT* queueADT, glossy_data_struct data){

   queueADT->glossy_data[queueADT->rear] = data;
   if(queueADT->count + 1 < MAX_QUEUE_SIZE)
	   LqueueADT->count++;

   if(queueADT->rear + 1 > MAX_QUEUE_SIZE)
	   queueADT->rear = 0;
   else
	   queueADT->rear++;
 }

 void Dequeue(queueCDT* queueADT){
   queueADT->count--;
 }


 glossy_data_struct getGlossyData(queueCDT* queueADT){
	 glossy_data_struct returnData;


	returnData = queueADT->glossy_data[queueADT->front];
	if(queueADT->count > 0){
		queueADT->count--;
		queueADT->front++;	}

   return returnData;
 }

int get_count(queueCDT* queueADT)
{
return queueADT->count;
}


///////////////////////////////////////////////////////
/////			GLOSSY CODE			//////////////////
//////////////////////////////////////////////////////


PROCESS_THREAD(glossy_print_stats_process, ev, data)
{
	PROCESS_BEGIN()
		;

		while (1)
		{
			PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
			// Print statistics only if Glossy is not still bootstrapping.
			if (!GLOSSY_IS_BOOTSTRAPPING())
			{
				if (get_rx_cnt())
				{	// Packet received at least once.
					// Increment number of successfully received packets.
					packets_received++;
					// Compute latency during last Glossy phase.
					rtimer_clock_t lat = get_t_first_rx_l() - get_t_ref_l();
					// Add last latency to sum of latencies.
					sum_latency += lat;
					// Convert latency to microseconds.
					latency = (unsigned long) (lat) * 1e6 / RTIMER_SECOND;
					// Print information about last packet and related latency.
					printf("Glossy received %u time%s: seq_no %lu, latency %lu.%03lu ms\n", get_rx_cnt(), (get_rx_cnt() > 1) ? "s" : "",
							glossy_data.seq_no, latency / 1000, latency % 1000);
				} else
				{	// Packet not received.
					// Increment number of missed packets.
					packets_missed++;
					// Print failed reception.
					printf("Glossy NOT received\n");
				}
#if GLOSSY_DEBUG
//			printf("skew %ld ppm\n", (long)(period_skew * 1e6) / GLOSSY_PERIOD);
				printf("high_T_irq %u, rx_timeout %u, bad_length %u, bad_header %u, bad_crc %u\n", high_T_irq, rx_timeout, bad_length, bad_header,
						bad_crc);
#endif /* GLOSSY_DEBUG */
				// Compute current average reliability.
				unsigned long avg_rel = packets_received * 1e5 / (packets_received + packets_missed);
				// Print information about average reliability.
				printf("average reliability %3lu.%03lu %% ", avg_rel / 1000, avg_rel % 1000);
				printf("(missed %lu out of %lu packets)\n", packets_missed, packets_received + packets_missed);
#if ENERGEST_CONF_ON
				// Compute average radio-on time, in microseconds.
				unsigned long avg_radio_on = (unsigned long) GLOSSY_PERIOD * 1e6 / RTIMER_SECOND
						* (energest_type_time(ENERGEST_TYPE_LISTEN) + energest_type_time(ENERGEST_TYPE_TRANSMIT))
						/ (energest_type_time(ENERGEST_TYPE_CPU) + energest_type_time(ENERGEST_TYPE_LPM));
				// Print information about average radio-on time.
				printf("average radio-on time %lu.%03lu ms\n", avg_radio_on / 1000, avg_radio_on % 1000);
#endif /* ENERGEST_CONF_ON */
				// Compute average latency, in microseconds.
				unsigned long avg_latency = sum_latency * 1e6 / (RTIMER_SECOND * packets_received);
				// Print information about average latency.
				printf("average latency %lu.%03lu ms\n", avg_latency / 1000, avg_latency % 1000);
			}
		}

	PROCESS_END();
}

PROCESS_THREAD(queue_init, ev, data)
{

PROCESS_BEGIN()
	;
	printf("[create queue]: create queue\n");
	if (IS_INITIATOR())
	{
		QueueInit(&myQueue);

	}

PROCESS_END();
}
PROCESS_THREAD(set_traffic_period, ev, data)
{

PROCESS_BEGIN()
	;
etimer_set(&et_traffic_period, CLOCK_SECOND * 10);
	printf("[set_traffic_period]: set_traffic_period\n");
	if (IS_INITIATOR())
	{
		while(1)
		{
			PROCESS_YIELD();

			if(ev == PROCESS_EVENT_TIMER)
			{

				//set traffic periodto random intervall between 0 and 8


				//reset trraffic period    after random intervall between 5s and 15s
				// x is in [0,1[
				   traffic_period = (uint) (rand() % 9);
				   if(traffic_period < 0){

					   traffic_period  = traffic_period * -1;
					   printf("[Traffic Period]: negativ  %u \n",traffic_period);
				   }
				   else
					   printf("[Traffic Period]: positiv  %u \n",traffic_period);

				   int x = rand() % 10;
				   if(x < 0)
					   x  = x * -1;

				   x = x + 5;
				   printf("[Traffic Period]: period will change again in %u s\n",x);
				   etimer_set(&et_traffic_period, CLOCK_SECOND * x);
			}
		}

	}

PROCESS_END();
}

PROCESS_THREAD(random_traffic_process, ev, data)
{
PROCESS_BEGIN()
;
etimer_set(&et_traffic, CLOCK_SECOND);

while (1)
{

	PROCESS_YIELD()
	;
	if ( IS_INITIATOR())

	{

		if (ev == PROCESS_EVENT_TIMER)
		{
			random_traffic_data.seq_no = seg_no++;
			printf("[QUEUE]: Add data to Queue with Seq: %d\n", random_traffic_data.seq_no);
			printf("[QUEUE]: Current traffic period is: %d\n\n", traffic_period);
			Enqueue(&myQueue, random_traffic_data);

			switch (traffic_period)
			{
				case 0:
					etimer_set(&et_traffic, CLOCK_SECOND);
					break;
				case 1:
					etimer_set(&et_traffic, CLOCK_SECOND * 0.9);
					break;
				case 2:
					etimer_set(&et_traffic, CLOCK_SECOND * 0.8);
					k++;
					break;
				case 3:
					etimer_set(&et_traffic, CLOCK_SECOND * 0.7);
					break;
				case 4:
					etimer_set(&et_traffic, CLOCK_SECOND * 0.6);
					break;
				case 5:
					etimer_set(&et_traffic, CLOCK_SECOND * 0.5);
					break;
				case 6:
					etimer_set(&et_traffic, CLOCK_SECOND * 0.4);
					break;
				case 7:
					etimer_set(&et_traffic, CLOCK_SECOND * 0.3);
					break;
				case 8:
					etimer_set(&et_traffic, CLOCK_SECOND * 0.2);
					break;
				case 9:
					etimer_set(&et_traffic, CLOCK_SECOND * 0.15);
					break;

				default:
					etimer_set(&et_traffic, CLOCK_SECOND * 0.4);
					break;
			}
		}

	}

PROCESS_END()
;
}
}

/** @} */

/**
 * \defgroup glossy-test-skew Clock skew estimation
 * @{
 */

static inline void estimate_period_skew(void)
{
// Estimate clock skew over a period only if the reference time has been updated.
if (GLOSSY_IS_SYNCED())
{
					// Estimate clock skew based on previous reference time and the Glossy period.
period_skew = get_t_ref_l() - (t_ref_l_old + (rtimer_clock_t)GLOSSY_PERIOD);
					// Update old reference time with the newer one.
t_ref_l_old = get_t_ref_l();
					// If Glossy is still bootstrapping, count the number of consecutive updates of the reference time.
if (GLOSSY_IS_BOOTSTRAPPING())
{
// Increment number of consecutive updates of the reference time.
skew_estimated++;
// Check if Glossy has exited from bootstrapping.
if (!GLOSSY_IS_BOOTSTRAPPING())
{
	// Glossy has exited from bootstrapping.
	leds_off(LEDS_RED);
	// Initialize Energest values.
	energest_init();
#if GLOSSY_DEBUG
	high_T_irq = 0;
	bad_crc = 0;
	bad_length = 0;
	bad_header = 0;
#endif /* GLOSSY_DEBUG */

}
}
}
}

/** @} */

void packet_queue(struct rtimer *t, void *ptr)
{

if ( IS_INITIATOR())

{

while (1)
{

}
}
}

/**
 * \defgroup glossy-test-scheduler Periodic scheduling
 * @{
 */

char glossy_scheduler(struct rtimer *t, void *ptr)
{
PT_BEGIN(&pt)
;

if (IS_INITIATOR())
{	// Glossy initiator.

while (1)
{

	random_traffic_data = getGlossyData(&myQueue);
	count = get_count(&myQueue);
//
//	printf("[SCHEDULER]: Get data from Queue with Seq: %d\n", random_traffic_data.seq_no);
	//printf("[SCHEDULER]: Data count: %d\n\n", count);

	if (count <= 10 && count >= 0)
	{
		K = 2;
		random_traffic_data.set_period = 2;
	} else if (count > 10 && count <= 30)
	{
		K = 4;
		random_traffic_data.set_period = 4;
	} else
	{
		K = 8;
		random_traffic_data.set_period = 8;
	}

	// Increment sequence number.
	// Glossy phase.
	leds_on(LEDS_GREEN);
	rtimer_clock_t t_stop = RTIMER_TIME(t) + GLOSSY_DURATION;
	// Start Glossy.
	glossy_start((uint8_t *) &random_traffic_data, DATA_LEN, GLOSSY_INITIATOR, GLOSSY_SYNC,
	N_TX,
	APPLICATION_HEADER, t_stop, (rtimer_callback_t) glossy_scheduler, t, ptr);
	// Store time at which Glossy has started.
	t_start = RTIMER_TIME(t);
	// Yield the protothread. It will be resumed when Glossy terminates.
	PT_YIELD(&pt);

	// Off phase.
	leds_off(LEDS_GREEN);
	// Stop Glossy.
	glossy_stop();

	Dequeue(&myQueue);

	if (!GLOSSY_IS_BOOTSTRAPPING())
	{
		// Glossy has already successfully bootstrapped.
		if (!GLOSSY_IS_SYNCED())
		{
			// The reference time was not updated: increment reference time by GLOSSY_PERIOD.
			set_t_ref_l(GLOSSY_REFERENCE_TIME + GLOSSY_PERIOD);
			set_t_ref_l_updated(1);
		}
	}
	// Schedule begin of next Glossy phase based on GLOSSY_PERIOD.

	rtimer_set(t, t_start + (GLOSSY_PERIOD / K), 1, (rtimer_callback_t) glossy_scheduler, ptr);
	// Estimate the clock skew over the last period.
	estimate_period_skew();
	// Poll the process that prints statistics (will be activated later by Contiki).
	process_poll(&glossy_print_stats_process);
	// Yield the protothread.
	PT_YIELD(&pt);
}
} else
{	// Glossy receiver.
int L = 4;
while (1)
{
	// Glossy phase.
	leds_on(LEDS_GREEN);
	rtimer_clock_t t_stop;
	if (GLOSSY_IS_BOOTSTRAPPING())
	{
		// Glossy is still bootstrapping:
		// Schedule end of Glossy phase based on GLOSSY_INIT_DURATION.
		t_stop = RTIMER_TIME(t) + GLOSSY_INIT_DURATION;
	} else
	{
		// Glossy has already successfully bootstrapped:
		// Schedule end of Glossy phase based on GLOSSY_DURATION.
		t_stop = RTIMER_TIME(t) + GLOSSY_DURATION;
	}

	// Start Glossy.
	glossy_start((uint8_t *) &glossy_data, DATA_LEN, GLOSSY_RECEIVER, GLOSSY_SYNC, N_TX,
	APPLICATION_HEADER, t_stop, (rtimer_callback_t) glossy_scheduler, t, ptr);

	//data1=&glossy_data;
	// Yield the protothread. It will be resumed when Glossy terminates.
	PT_YIELD(&pt);
	// Off phase.
	leds_off(LEDS_GREEN);
	// Stop Glossy.
	glossy_stop();

	if (glossy_data.set_period == 2)
	{
		L = 2;
	} else if (glossy_data.set_period == 4)
	{
		L = 4;
	} else
	{
		L = 8;
	}

	if (GLOSSY_IS_BOOTSTRAPPING())
	{
		// Glossy is still bootstrapping.
		if (!GLOSSY_IS_SYNCED())
		{
			// The reference time was not updated: reset skew_estimated to zero.
			skew_estimated = 0;
		}
	} else
	{
		// Glossy has already successfully bootstrapped.
		if (!GLOSSY_IS_SYNCED())
		{
			// The reference time was not updated:
			// increment reference time by GLOSSY_PERIOD + period_skew.
			set_t_ref_l(
			GLOSSY_REFERENCE_TIME + GLOSSY_PERIOD + period_skew);
			set_t_ref_l_updated(1);
			// Increment sync_missed.
			sync_missed++;
		} else
		{
			// The reference time was not updated: reset sync_missed to zero.
			sync_missed = 0;
		}
	}
	// Estimate the clock skew over the last period.
	estimate_period_skew();
	if (GLOSSY_IS_BOOTSTRAPPING())
	{
		// Glossy is still bootstrapping.
		if (skew_estimated == 0)
		{
			// The reference time was not updated:
			// Schedule begin of next Glossy phase based on last begin and GLOSSY_INIT_PERIOD.
			rtimer_set(t, RTIMER_TIME(t) + GLOSSY_INIT_PERIOD, 1, (rtimer_callback_t) glossy_scheduler, ptr);
		} else
		{
			// The reference time was updated:
			// Schedule begin of next Glossy phase based on reference time and GLOSSY_INIT_PERIOD.
			rtimer_set(t,
			GLOSSY_REFERENCE_TIME + GLOSSY_PERIOD - GLOSSY_INIT_GUARD_TIME, 1, (rtimer_callback_t) glossy_scheduler, ptr);
		}
	} else
	{
		// Glossy has already successfully bootstrapped:
		// Schedule begin of next Glossy phase based on reference time and GLOSSY_PERIOD.
		rtimer_set(t,
		GLOSSY_REFERENCE_TIME + (GLOSSY_PERIOD / L) + period_skew - GLOSSY_GUARD_TIME * (1 + sync_missed), 1, (rtimer_callback_t) glossy_scheduler,
				ptr);
	}
	// Poll the process that prints statistics (will be activated later by Contiki).
	process_poll(&glossy_print_stats_process);
	// Yield the protothread.
	PT_YIELD(&pt);
}
}

PT_END(&pt);
}

/** @} */

/**
 * \defgroup glossy-test-init Initialization
 * @{
 */

PROCESS_THREAD(glossy_test, ev, data)
{
PROCESS_BEGIN()
;

leds_on(LEDS_RED);
// Initialize Glossy data.
glossy_data.seq_no = 0;
// Start print stats processes.
process_start(&glossy_print_stats_process, NULL);
// Start Glossy busy-waiting process.
process_start(&glossy_process, NULL);
process_start(&queue_init, NULL);
process_start(&set_traffic_period, NULL);
process_start(&random_traffic_process, NULL);
rtimer_set(&rt, RTIMER_NOW() + RTIMER_SECOND * 2, 1, (rtimer_callback_t) glossy_scheduler, NULL);

PROCESS_END();
}

/** @} */
/** @} */
/** @} */

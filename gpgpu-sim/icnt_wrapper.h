// Copyright (c) 2009-2011, Tor M. Aamodt, Wilson W.L. Fung, Ali Bakhoda
// The University of British Columbia
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this
// list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.
// Neither the name of The University of British Columbia nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef ICNT_WRAPPER_H
#define ICNT_WRAPPER_H

#include <stdio.h>

// functional interface to the interconnect

typedef void (*icnt_create_p)(unsigned n_shader,  unsigned n_mem);
typedef void (*icnt_init_p)( );
typedef bool (*icnt_has_buffer_p)(unsigned input, unsigned int size);
typedef void (*icnt_push_p)(unsigned input, unsigned output, void* data, unsigned int size);
typedef void* (*icnt_pop_p)(unsigned output);
typedef void (*icnt_transfer_p)( );
typedef bool (*icnt_busy_p)( );
typedef void (*icnt_drain_p)( );
typedef void (*icnt_display_stats_p)( );
typedef void (*icnt_display_overall_stats_p)( );
typedef void (*icnt_display_state_p)(FILE* fp);
typedef unsigned (*icnt_get_flit_size_p)();

extern icnt_create_p     icnt_create;
extern icnt_init_p       icnt_init;
extern icnt_has_buffer_p icnt_has_buffer;
extern icnt_push_p       icnt_push;
extern icnt_pop_p        icnt_pop;
extern icnt_transfer_p   icnt_transfer;
extern icnt_busy_p       icnt_busy;
extern icnt_drain_p      icnt_drain;
extern icnt_display_stats_p icnt_display_stats;
extern icnt_display_overall_stats_p icnt_display_overall_stats;
extern icnt_display_state_p icnt_display_state;
extern icnt_get_flit_size_p icnt_get_flit_size;
extern int g_network_mode;

enum network_mode {
   INTERSIM = 1,
   N_NETWORK_MODE
};

void icnt_wrapper_init();
void icnt_reg_options( class OptionParser * opp );


// added by kh(10/30/14)
enum perf_packet_type {
   SM_TO_ICNT = 1,
   ICNT_TO_MC,
   MC_TO_ICNT,
   ICNT_TO_SM
};

typedef void (*perf_icnt_push_p)(unsigned input, unsigned output, void* data, unsigned int size, perf_packet_type type, unsigned long long cycle, unsigned long long cycle2);
typedef void* (*perf_icnt_pop_p)(unsigned output, perf_packet_type type, unsigned long long cycle, unsigned long long cycle2);

extern perf_icnt_push_p       perf_icnt_push;
extern perf_icnt_pop_p        perf_icnt_pop;
///

//added by kh(120815)
typedef void* (*icnt_top_for_buffering_p)(unsigned output);
extern icnt_top_for_buffering_p		icnt_top_for_buffering;

//added by kh(042516)
typedef unsigned (*icnt_get_buffer_occupancy_p)(unsigned input);
extern icnt_get_buffer_occupancy_p icnt_get_buffer_occupancy;
///

#endif

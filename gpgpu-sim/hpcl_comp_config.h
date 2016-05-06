/*
 * hpcl_comp_config.h
 *
 *  Created on: Jan 4, 2016
 *      Author: mumichang
 */

#ifndef HPCL_COMP_CONFIG_H_
#define HPCL_COMP_CONFIG_H_

struct hpcl_comp_config {

  hpcl_comp_config()
  {
    hpcl_comp_en = 0;
    hpcl_comp_algo = GLOBAL_PRIVATE;
    hpcl_comp_buffer_size = 1;
    hpcl_comp_word_size = 2;

    //added by kh(041216)
    hpcl_comp_noc_type = MESH;
    ///

    //added by kh(042416)
    hpcl_inter_rep_comp_en = 0;
    ///

    //added by kh(042516)
    hpcl_req_coal_en = 0;
    hpcl_req_coal_buffer_size = 1;
    hpcl_req_coal_lazy_inj_en = 0;
    ///
  }

  void init()
  {
  }

  void reg_options(class OptionParser * opp)
  {
    option_parser_register(opp, "-hpcl_comp_en", OPT_UINT32, &hpcl_comp_en,
			     "0: disable compression, 1: enable compression",
			     "0");

    option_parser_register(opp, "-hpcl_comp_algo", OPT_UINT32, &hpcl_comp_algo,
			   "0: global_table(private), 1: ",
			   "0");

    option_parser_register(opp, "-hpcl_comp_buffer_size", OPT_UINT32, &hpcl_comp_buffer_size,
  			   "1: default, ",
  			   "1");

    option_parser_register(opp, "-hpcl_comp_word_size", OPT_UINT32, &hpcl_comp_word_size,
  			   "2: default, ",
  			   "2");

    //added by kh(041216)
    option_parser_register(opp, "-hpcl_comp_noc_type", OPT_UINT32, &hpcl_comp_noc_type,
    			   "1: default, MESH",
    			   "1");

    option_parser_register(opp, "-hpcl_comp_ideal_comp_ratio", OPT_UINT32, &hpcl_comp_ideal_comp_ratio,
    			   "0: default, no compression",
    			   "0");
    ///

    //added by kh(042416)
    option_parser_register(opp, "-hpcl_inter_rep_comp_en", OPT_UINT32, &hpcl_inter_rep_comp_en,
			     "0: disable inter-rep compression, 1: enable inter-rep compression",
			     "0");
    ///

    //added by kh(042516)
    option_parser_register(opp, "-hpcl_req_coal_en", OPT_UINT32, &hpcl_req_coal_en,
			 "0: disable req coalescing, 1: enable req coalescing",
			 "0");
    option_parser_register(opp, "-hpcl_req_coal_buffer_size", OPT_UINT32, &hpcl_req_coal_buffer_size,
  			   "1: default, ",
  			   "1");
    option_parser_register(opp, "-hpcl_req_coal_lazy_inj_en", OPT_UINT32, &hpcl_req_coal_lazy_inj_en,
    			 "0: disable lazy req injection, 1: enable lazy req injection",
    			 "0");
    ///
  }


  enum comp_algo_type {
    GLOBAL_PRIVATE = 0,
    LOCAL_WORD_MATCHING,

    //added by abpd (042816)
    BDI_WORD_MATCHING,
    ABPD_LOCAL_WORD_MATCHING,
    CPACK_WORD_MATCHING,
    FPC_WORD_MATCHING
  };


  unsigned hpcl_comp_en;
  enum comp_algo_type hpcl_comp_algo;

  //added by kh(031916)
  unsigned hpcl_comp_buffer_size;
  unsigned hpcl_comp_word_size;


  //added by kh(041216)
  enum noc_type {
    MESH = 0,
    GLOBAL_CROSSBAR,
  };
  enum noc_type hpcl_comp_noc_type;

  unsigned hpcl_comp_ideal_comp_ratio;		//ideal compression ratio: 0, 20, 40, 60
  ///

  //added by kh(042416)
  unsigned hpcl_inter_rep_comp_en;
  ///

  //added by kh(042416)
  unsigned hpcl_req_coal_en;
  unsigned hpcl_req_coal_buffer_size;
  unsigned hpcl_req_coal_lazy_inj_en;
  //

};




#endif /* HPCL_DIRECT_LINK_CONFIG_H_ */

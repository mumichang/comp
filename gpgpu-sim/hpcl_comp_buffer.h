/*
 * hpcl_comp_buffer.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_COMP_BUFFER_H_
#define HPCL_COMP_BUFFER_H_

//#include "hpcl_comp_lwm_pl_proc.h"
//#include "hpcl_comp_pl_data.h"
//#include "hpcl_dict_ctrl_msg.h"
#include <vector>
#include <cassert>
#include <algorithm>    // std::find

#include "hpcl_dict.h"
#include "hpcl_comp_anal.h"

extern hpcl_comp_anal* g_hpcl_comp_anal;


class hpcl_comp_buffer {
private:
  unsigned m_id;

  std::vector<mem_fetch*> m_comp_buffer;

public:
  hpcl_comp_buffer()		{	m_id = 0;	m_comp_buffer_size = 0;	};
  hpcl_comp_buffer(int id)	{	m_id = id;	m_comp_buffer_size = 0;	};
  ~hpcl_comp_buffer()		{	};


  void create(unsigned buffer_size, unsigned subbuffer_no);
  void push_mem_fetch(mem_fetch* mf);
  void pop_mem_fetch();
  mem_fetch* top_mem_fetch();

private:
  unsigned m_comp_buffer_size;
public:
  bool has_comp_buffer_space();

//added by kh(042116)
  std::vector<std::vector<mem_fetch*> > m_comp_subbuffer;
  void print(unsigned id, unsigned comp_res=0);
  bool has_pending_mem_fetch();
  void get_reply_rate_dist(double& single_rep_rate, double& multi_rep_rate, double& no_rep_rate, double& multi_rep_no);
///

//added by kh(042316)
private:
  hpcl_dict<unsigned char>* m_dict_for_inter_read_replies;
public:
  void anal_redund_inter_read_replies();
///


//added by kH(042316)
public:
  std::vector<mem_fetch*>& get_all_waiting_mfs (mem_fetch* mf);
  void delete_mfs (std::vector<mem_fetch*>& mfs);
};

void hpcl_comp_buffer::create(unsigned comp_buffer_size, unsigned subbuffer_no) {
  m_comp_buffer_size = comp_buffer_size;

  //added by kh(042116)
  //std::map<unsigned, mem_fetch*> tmp;
  std::vector<mem_fetch*> tmp;
  for(int i = 0; i < subbuffer_no; i++) {
      m_comp_subbuffer.push_back(tmp);
  }
  ///

  //added by kh(042316)
  m_dict_for_inter_read_replies = new hpcl_dict<unsigned char>(1024, HPCL_LFU);
  m_dict_for_inter_read_replies->clear();
  ///
}


void hpcl_comp_buffer::push_mem_fetch(mem_fetch* mf) {

//  if(mf)	std::cout << "hpcl_comp_buffer::push_mem_fetch mf " << mf->get_request_uid() << std::endl;
//  else		std::cout << "hpcl_comp_buffer::push_mem_fetch mf " << "NULL" << std::endl;

  m_comp_buffer.push_back(mf);

  //added by kh(042116)
  //m_comp_subbuffer[mf->get_tpc()].insert(std::pair<unsigned, mem_fetch*>(mf->get_request_uid(), mf));
  m_comp_subbuffer[mf->get_tpc()].push_back(mf);
  ///
}

void hpcl_comp_buffer::pop_mem_fetch() {
  mem_fetch* mf = m_comp_buffer[0];
  m_comp_buffer.erase(m_comp_buffer.begin());

  //added by kh(042116)
  //std::map<unsigned, mem_fetch*>::iterator it;
  //it = m_comp_subbuffer[mf->get_tpc()].find(mf->get_request_uid());
  ///m_comp_subbuffer[mf->get_tpc()].erase (it);
  std::vector<mem_fetch*>::iterator it = m_comp_subbuffer[mf->get_tpc()].begin();
  unsigned erase_index = 0;
  for(unsigned i = 0; i < m_comp_subbuffer[mf->get_tpc()].size(); i++) {
    if(m_comp_subbuffer[mf->get_tpc()][i] == mf) {
      erase_index = i;	break;
    }
  }
  m_comp_subbuffer[mf->get_tpc()].erase(it+erase_index);
  ///
}

mem_fetch* hpcl_comp_buffer::top_mem_fetch() {
  if(m_comp_buffer.size() == 0)	return NULL;
  else				return m_comp_buffer[0];
}

bool hpcl_comp_buffer::has_comp_buffer_space() {
  if(m_comp_buffer.size() >= m_comp_buffer_size)	return false;
  else							return true;
}

void hpcl_comp_buffer::print(unsigned id, unsigned comp_res) {

  std::vector<unsigned short> word_list_2B;
  std::vector<unsigned int> word_list_4B;
  std::vector<unsigned long long> word_list_8B;

  printf("hpcl_comp_buffer[%u] = ", id);
  for(unsigned i = 0; i < m_comp_subbuffer.size(); i++)
  {
      printf("%u ", (unsigned)m_comp_subbuffer[i].size());
  }
  printf("\n");

  for(unsigned i = 0; i < m_comp_subbuffer.size(); i++)
  {
    if(m_comp_subbuffer[i].size() > 1) {
      for(unsigned j = 0; j < m_comp_subbuffer[i].size(); j++) {

	unsigned resolution = comp_res;
	if(resolution == 0) {
	    resolution = m_comp_subbuffer[i][j]->get_comp_res();
	}

	printf("mf %u\n", m_comp_subbuffer[i][j]->get_request_uid());
	std::vector<unsigned char>& cache_data = m_comp_subbuffer[i][j]->get_real_data_ptr();
	//printf("org_data_sm_%u_rep_%u = 0x");
	//for(unsigned i = 0; i < cache_data.size(); i++) {
	//  printf("%02x", cache_data[i]);
	//}
	//printf("\n");
	if(cache_data.size() > 0) {
	  /*
	  if(resolution == 2) {
	    hpcl_comp_lwm_pl_proc<unsigned short>::decompose_data(cache_data, word_list_2B);
	    hpcl_comp_lwm_pl_proc<unsigned short>::print_word_list(word_list_2B);
	    word_list_2B.clear();
	  } else if(resolution == 4) {
	    hpcl_comp_lwm_pl_proc<unsigned int>::decompose_data(cache_data, word_list_4B);
	    hpcl_comp_lwm_pl_proc<unsigned int>::print_word_list(word_list_4B);
	    word_list_4B.clear();
	  } else if(resolution == 8) {
	    hpcl_comp_lwm_pl_proc<unsigned long long>::decompose_data(cache_data, word_list_8B);
	    hpcl_comp_lwm_pl_proc<unsigned long long>::print_word_list(word_list_8B);
	    word_list_8B.clear();
	  }
	  */
	  //m_comp_subbuffer[i][j]->print_compword();
	    m_comp_subbuffer[i][j]->print_uncompword();
	} else {
	  printf("\tno data\n");
	}
      }
    }
  }

  printf("\n");
}

bool hpcl_comp_buffer::has_pending_mem_fetch() {
  return (m_comp_buffer.size() > 0)? true : false;
}

void hpcl_comp_buffer::get_reply_rate_dist(double& single_rep_rate, double& multi_rep_rate, double& no_rep_rate, double& avg_multi_rep_no)
{
  single_rep_rate = 0;
  multi_rep_rate = 0;
  no_rep_rate = 0;

  avg_multi_rep_no = 0;
  double sm_no_for_multi_rep = 0;
  for(unsigned i = 0; i < m_comp_subbuffer.size(); i++)
  {
    //deleted by kh(042316)
    /*
    if(m_comp_subbuffer[i].size() == 0)	no_rep_rate++;
    else if(m_comp_subbuffer[i].size() == 1)	single_rep_rate++;
    else					multi_rep_rate++;
    */

    //added by kh(042316)
    //To check only the redundancy in read replies
    if(m_comp_subbuffer[i].size() == 0)	no_rep_rate++;
    else if(m_comp_subbuffer[i].size() == 1) {
      if(m_comp_subbuffer[i][0]->get_is_write())	no_rep_rate++;
      else						single_rep_rate++;
    } else {
      unsigned multi_read_rep_no = 0;
      for(unsigned j = 0; j < m_comp_subbuffer[i].size(); j++) {
	 if(!m_comp_subbuffer[i][j]->get_is_write())	multi_read_rep_no++;
      }

      if(multi_read_rep_no == 0) 	no_rep_rate++;
      else if(multi_read_rep_no == 1)	single_rep_rate++;
      else if(multi_read_rep_no > 1) {
	  multi_rep_rate++;
	  avg_multi_rep_no += multi_read_rep_no;
	  sm_no_for_multi_rep++;
      }
    }
    ///
  }

  no_rep_rate = no_rep_rate/m_comp_subbuffer.size();
  single_rep_rate = single_rep_rate/m_comp_subbuffer.size();
  multi_rep_rate = multi_rep_rate/m_comp_subbuffer.size();

  if(sm_no_for_multi_rep > 0)	avg_multi_rep_no = avg_multi_rep_no/sm_no_for_multi_rep;
}


void hpcl_comp_buffer::anal_redund_inter_read_replies() {

  std::vector<unsigned short> word_list_2B;
  std::vector<unsigned int> word_list_4B;
  std::vector<unsigned long long> word_list_8B;
/*
  printf("hpcl_comp_buffer[%u] = ", id);
  for(unsigned i = 0; i < m_comp_subbuffer.size(); i++)
  {
      printf("%u ", (unsigned)m_comp_subbuffer[i].size());
  }
  printf("\n");
*/

  for(unsigned i = 0; i < m_comp_subbuffer.size(); i++)
  {
    //if multi-reply data for the same SM are found.
    std::vector<mem_fetch*> multi_read_replies;
    if(m_comp_subbuffer[i].size() > 1) {
      for(unsigned j = 0; j < m_comp_subbuffer[i].size(); j++) {
	/*
	unsigned resolution = comp_res;
	if(resolution == 0) {
	  resolution = m_comp_subbuffer[i][j]->get_comp_res();
	}
	*/
	//printf("mf %u\n", m_comp_subbuffer[i][j]->get_request_uid());
	std::vector<unsigned char>& cache_data = m_comp_subbuffer[i][j]->get_real_data_ptr();

	if(cache_data.size() > 0) {	//read reply(not inst) only are filtered.
	  //m_comp_subbuffer[i][j]->print_compword();
	  //m_comp_subbuffer[i][j]->print_uncompword();
	  multi_read_replies.push_back(m_comp_subbuffer[i][j]);
	} else {
	  //printf("\tno data\n");
	}
      }
    }

    unsigned all_word1B_no = 0;
    unsigned redund_word1B_no = 0;
    for(unsigned j = 0; j < multi_read_replies.size() && multi_read_replies.size() > 1; j++) {
	//printf("mf %u\n", multi_read_replies[j]->get_request_uid());
	//multi_read_replies[j]->print_uncompword();

	//redundancy is measured based on the uncompressed word in the first reply
	if(j == 0) {
	  unsigned row = multi_read_replies[j]->get_uncompword_row_no();
	  for(unsigned m = 0; m < row; m++) {
	    unsigned col = multi_read_replies[j]->get_uncompword_col_no(m);
	    for(unsigned n = 0; n < col; n++) {
	      class mem_fetch::comp_word tmp = multi_read_replies[j]->get_uncompword(m, n);
	      if(tmp.comp_status == mem_fetch::UNCOMPRESSED) {
		  m_dict_for_inter_read_replies->update_dict(tmp.word1B, 0);
	      }
	    }
	  }
	} else {
	  unsigned row = multi_read_replies[j]->get_uncompword_row_no();
	  for(unsigned m = 0; m < row; m++) {
	    unsigned col = multi_read_replies[j]->get_uncompword_col_no(m);
	    for(unsigned n = 0; n < col; n++) {
	      class mem_fetch::comp_word tmp = multi_read_replies[j]->get_uncompword(m, n);
	      if(tmp.comp_status == mem_fetch::UNCOMPRESSED) {
		  int index = m_dict_for_inter_read_replies->search_word(tmp.word1B);
		  if(index >= 0) redund_word1B_no++;

		  all_word1B_no++;
	      }
	    }
	  }
	}
    }

    if(all_word1B_no > 0) {
	//printf("all_word1B_no %u, redund_word1B_no %u, inter_read_replies_redund_rate %f ",
	//       all_word1B_no, redund_word1B_no, (double)redund_word1B_no/all_word1B_no);
	g_hpcl_comp_anal->add_sample(hpcl_comp_anal::WORD1B_REDUND_RATE_INTER_READ_REPLIES, (double)redund_word1B_no/all_word1B_no);
    }

    m_dict_for_inter_read_replies->clear();
    multi_read_replies.clear();
  }

}

std::vector<mem_fetch*>& hpcl_comp_buffer::get_all_waiting_mfs (mem_fetch* mf)
{
  unsigned sm_id = mf->get_tpc();
  return m_comp_subbuffer[sm_id];
}


void hpcl_comp_buffer::delete_mfs (std::vector<mem_fetch*>& mfs)
{
  std::vector<mem_fetch*>::iterator it;
  for(unsigned i = 0; i < mfs.size(); i++) {
//    printf("hpcl_comp_buffer::delete_mfs - mf %u", mfs[i]->get_request_uid());

    std::vector<mem_fetch*>::iterator it1 = std::find (m_comp_buffer.begin(), m_comp_buffer.end(), mfs[i]);
//    if(it1 == m_comp_buffer.end()) {
//      for(unsigned j = 0; j < m_comp_buffer.size(); j++) {
//	printf("\tcomp_buffer %u mf %u\n", j, m_comp_buffer[j]->get_request_uid());
//      }
//    }
    assert (it1 != m_comp_buffer.end());
    m_comp_buffer.erase(it1);
    unsigned sm_id = mfs[i]->get_tpc();
    std::vector<mem_fetch*>::iterator it2 = std::find (m_comp_subbuffer[sm_id].begin(), m_comp_subbuffer[sm_id].end(), mfs[i]);
    assert (it2 != m_comp_subbuffer[sm_id].end());
    m_comp_subbuffer[sm_id].erase(it2);

    //printf("hpcl_comp_buffer::delete_mfs - mf %u, mf %u", (*it1)->get_request_uid(), (*it2)->get_request_uid());
  }
}



#endif /* HPCL_COMP_PL_H_ */

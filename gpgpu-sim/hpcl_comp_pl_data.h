/*
 * hpcl_comp_pl_data.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_COMP_PL_DATA_H_
#define HPCL_COMP_PL_DATA_H_

#include <vector>
#include <limits>
//#include "hpcl_virt_flit.h"
#include <cmath>

#include "../intersim2/flit.hpp"

enum hpcl_dict_rep_policy {
  HPCL_INVALID = 0,
  HPCL_LRU,
  HPCL_LFU
};

//template <class K>
class hpcl_comp_pl_data {
public:
  hpcl_comp_pl_data() 		{
    //deleted by kh(031816)
    //m_flit_ptr.clear();

    //added by kh(031816)
    clean();
  }
  virtual ~hpcl_comp_pl_data()	{ m_flit_ptr.clear(); }

private:
  //std::vector<hpcl_virt_flit*> m_flit_ptr;	//original flit in gpgpu-sim
  std::vector<Flit*> m_flit_ptr;	//original flit in gpgpu-sim
  double m_comp_rsl;			//cumulative average compression ratio
  bool m_is_tail_done;

public:

  void add_comp_pl_data(Flit* flit_ptr) {
    m_flit_ptr.push_back(flit_ptr);
  }

  Flit* get_flit_ptr(int index=0) {
    if(m_flit_ptr.size() == 0) 	return NULL;
    else {
	assert(m_flit_ptr.size() > index);
	return m_flit_ptr[index];
    }
  }
  unsigned get_flit_no() {
    return m_flit_ptr.size();
  }
  bool is_compressible() {
    if(fabs(m_comp_rsl - 100) < std::numeric_limits<double>::epsilon())
      return true;
    else
      return false;
  }
  void set_comp_result(double comp_rsl) {
    m_comp_rsl = comp_rsl;
  }

  double get_comp_result() {
    return m_comp_rsl;
  }

  void copy(hpcl_comp_pl_data* hcpd) {
    m_flit_ptr = hcpd->m_flit_ptr;
    m_comp_rsl = hcpd->m_comp_rsl;
  }

  void clean() {
    m_flit_ptr.clear();
    m_comp_rsl = -1;
    m_is_tail_done = false;
    m_is_comp_done = false;

    //added by kh(031817)
    m_mf = NULL;
  }

  void set_tail_done_flag() {
    m_is_tail_done = true;
  }

  bool get_tail_done_flag() {
    return m_is_tail_done;
  }


private:
  bool m_is_comp_done;

public:
  void set_comp_done_flag()	{ 	m_is_comp_done = true;	}
  void clear_comp_done_flag()	{ 	m_is_comp_done = false;	}
  bool get_comp_done_flag()	{	return m_is_comp_done;	}


//added by kh(031817)
private:
  mem_fetch* m_mf;

public:
  mem_fetch* get_mem_fetch()		{	return m_mf;		}
  void set_mem_fetch(mem_fetch* mf)	{	m_mf = mf;		}
///
};

/*
void hpcl_comp_pl_data::add_comp_pl_data(hpcl_virt_flit* flit_ptr) {
  m_flit_ptr.push_back(flit_ptr);
}
hpcl_virt_flit* hpcl_comp_pl_data::get_flit_ptr(int index) {
  if(m_flit_ptr.size() == 0) 	return NULL;
  else				return m_flit_ptr[index];
}
unsigned hpcl_comp_pl_data::get_flit_no() {
  return m_flit_ptr.size();
}
bool hpcl_comp_pl_data::is_compressible() {
  if(fabs(m_comp_rsl - 100) < std::numeric_limits<double>::epsilon())
    return true;
  else
    return false;
}

void hpcl_comp_pl_data::set_comp_result(double comp_rsl) {
  m_comp_rsl = comp_rsl;
}
double hpcl_comp_pl_data::get_comp_result() {
  return m_comp_rsl;
}

void hpcl_comp_pl_data::copy(hpcl_comp_pl_data* hcpd) {
  m_flit_ptr = hcpd->m_flit_ptr;
  m_comp_rsl = hcpd->m_comp_rsl;
}

void hpcl_comp_pl_data::clean() {
  m_flit_ptr.clear();
  m_comp_rsl = -1;
  m_is_tail_done = false;
}

void hpcl_comp_pl_data::set_tail_done_flag() {
  m_is_tail_done = true;
}

bool hpcl_comp_pl_data::get_tail_done_flag() {
  return m_is_tail_done;
}
*/

#endif /* HPCL_COMP_PL_DATA_H_ */

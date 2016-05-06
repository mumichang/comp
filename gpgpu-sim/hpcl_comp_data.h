/*
 * hpcl_virt_flit.h
 *
 *  Created on: Feb 22, 2016
 *      Author: mumichang
 */

#ifndef HPCL_VIRT_FLIT_H_
#define HPCL_VIRT_FLIT_H_

#include <vector>

class hpcl_comp_data {
public:
  hpcl_comp_data () {}
  virtual ~hpcl_comp_data () {}

public:
  std::vector<unsigned> data_index;
  std::vector<unsigned short> raw_data;	//for debugging
  void copy(hpcl_comp_data* hcd) {
    data_index = hcd->data_index;
  }
};

/*
class hpcl_virt_flit {
public:
  hpcl_virt_flit () {}
  virtual ~hpcl_virt_flit () {}

public:
  enum virt_flit_type {
    HEAD = 0,
    BODY,
    TAIL
  };

  int index;
  enum virt_flit_type type;
  int m_enc_status;				//encoding status
  std::vector<unsigned char> m_raw_data;	//original data (32B)
  std::vector<hpcl_comp_data*> m_comp_data;	//compressed data format
  std::vector<unsigned char> m_decomp_data;	//decompressed data
  ///

  bool check_correctness() {
    if(m_raw_data.size() == m_decomp_data.size()) {
      for(unsigned i = 0; i < m_raw_data.size(); i++) {
        if(m_raw_data[i] != m_decomp_data[i])	return false;
      }
    } else {
        return false;
    }
    return true;
  }

};
*/

#endif /* HPCL_VIRT_FLIT_H_ */
